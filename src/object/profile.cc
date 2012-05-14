/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/foreach.hh>

#include <runner/job.hh>

#include <object/profile.hh>

namespace urbi
{
  namespace object
  {


    /*----------.
    | Profile.  |
    `----------*/

    Profile::Profile()
      : yields_(0)
      , wall_clock_time_(0)
      , total_time_(0)
      , function_calls_(0)
      , function_call_depth_max_(0)
    {
      proto_add(proto);
    }

    Profile::Profile(rProfile model)
      : yields_(0)
      , wall_clock_time_(0)
      , total_time_(0)
      , function_calls_(0)
      , function_call_depth_max_(0)
    {
      proto_add(model);
    }

    Profile::~Profile()
    {}

    Profile::Info::Info()
      : checkpoint(0)
      , function_current(0)
      , function_call_depth(0)
    {
    }

    static bool function_profiles_sort(Object* lhs, Object* rhs)
    {
      return static_cast<FunctionProfile*>(lhs)->self_time_get()
        > static_cast<FunctionProfile*>(rhs)->self_time_get();
    }

    static rList function_profiles(Profile* profile)
    {
      objects_type res;

      foreach (const FunctionProfiles::value_type& fp,
               profile->functions_profile_get())
        res << fp.second;
      std::sort(res.begin(), res.end(), &function_profiles_sort);

      return new List(res);
    }

    URBI_CXX_OBJECT_INIT(Profile)
      : yields_(0)
      , wall_clock_time_(0)
      , total_time_(0)
      , function_calls_(0)
      , function_call_depth_max_(0)
    {
      proto_add(Object::proto);

      bind(SYMBOL(calls), function_profiles);
      BINDG(maxFunctionCallDepth, function_call_depth_max_get);
      BINDG(totalCalls, function_calls_get);
      BINDG(totalTime);
      BINDG(wallClockTime);
      BINDG(yields, yields_get);
    }


    /*------------------.
    | FunctionProfile.  |
    `------------------*/

    FunctionProfile::FunctionProfile()
      : calls_(0)
      , self_time_(0)
      , time_(0)
    {
      proto_add(proto);
    }

    FunctionProfile::FunctionProfile(rFunctionProfile model)
      : calls_(0)
      , self_time_(0)
      , time_(0)
    {
      proto_add(model);
    }

    FunctionProfile::~FunctionProfile()
    {}

    URBI_CXX_OBJECT_INIT(FunctionProfile)
      : calls_(0)
      , self_time_(0)
      , time_(0)
    {
      proto_add(Object::proto);

      BINDG(calls, calls_get);
      BINDG(name, name_get);
      BINDG(selfTime);
      BINDG(selfTimePer);
      BINDG(time);
    }

    FunctionProfile&
    FunctionProfile::operator+=(const FunctionProfile& rhs)
    {
      calls_     += rhs.calls_;
      self_time_ += rhs.self_time_;
      time_      += rhs.time_;
      return *this;
    }

    ufloat
    FunctionProfile::selfTime() const
    {
      return ufloat(self_time_get()) / 1000000;
    }

    ufloat
    FunctionProfile::selfTimePer() const
    {
      return ufloat(self_time_get()) / calls_get() / 1000000;
    }

    ufloat
    FunctionProfile::time() const
    {
      return ufloat(time_get()) / 1000000;
    }

    ufloat
    Profile::totalTime() const
    {
      return ufloat(total_time_get()) / 1000000;
    }

    ufloat
    Profile::wallClockTime() const
    {
      return ufloat(wall_clock_time_get()) / 1000000;
    }

    Profile&
    Profile::operator+=(const Profile& rhs)
    {
      yields_          += rhs.yields_;
      wall_clock_time_ += rhs.wall_clock_time_;
      total_time_      += rhs.total_time_;
      function_calls_  += rhs.function_calls_;

      function_call_depth_max_ = std::max(function_call_depth_max_,
                                          rhs.function_call_depth_max_);

      foreach (const FunctionProfiles::value_type& value,
               rhs.functions_profile_)
      {
        FunctionProfiles::iterator it = functions_profile_.find(value.first);
        if (it != functions_profile_.end())
          *it->second += *value.second;
        else
          functions_profile_.insert(value);
      }
      return *this;
    }

    void
    Profile::step(libport::utime_t& checkpoint, void* function_current)
    {
      libport::utime_t now = libport::utime();
      libport::utime_t res = now - checkpoint;
      total_time_ += res;
      wall_clock_time_ += res;
      functions_profile_[function_current]->self_time_ += res;
      checkpoint = now;
    }

    void
    Profile::start(libport::Symbol name, Object* current,
                   bool count, Info& i)
    {
      i.checkpoint = libport::utime();
      i.function_current = current;
      if (!functions_profile_[current])
      {
        functions_profile_[current] = new FunctionProfile;
        functions_profile_[current]->name_ = name;
      }
      if (count)
        ++functions_profile_[current]->calls_;
    }

    void
    Profile::stop(Info& i)
    {
      step(i.checkpoint, i.function_current);
    }

    void
    Profile::preempted(Info& i)
    {
      step(i.checkpoint, i.function_current);
      ++yields_;
    }

    void
    Profile::resumed(Info& i)
    {
      libport::utime_t now = libport::utime();
      wall_clock_time_ += now - i.checkpoint;
      i.checkpoint = now;
    }

    Profile*
    Profile::fork()
    {
      Profile* child = new Profile;
      child->function_call_depth_max_ = function_call_depth_max_;
      return child;
    }

    void
    Profile::join(Profile* other)
    {
      *this += *other;
      delete other;
    }

    Profile::idx
    Profile::enter(Object* function, libport::Symbol msg, Info& i)
    {
      idx prev = 0;
      step(i.checkpoint, i.function_current);
      prev = i.function_current;
      i.function_current = function;
      ++function_calls_;
      ++i.function_call_depth;
      if (i.function_call_depth > function_call_depth_max_)
        function_call_depth_max_ = i.function_call_depth;
      if (!functions_profile_[function])
        functions_profile_[function] = new FunctionProfile;
      ++functions_profile_[function]->calls_;
      if (functions_profile_[function]->name_.empty())
        functions_profile_[function]->name_ = msg;
      return prev;
    }

    void
    Profile::leave(idx prev, Info& i)
    {
      --i.function_call_depth;
      step(i.checkpoint, i.function_current);
      i.function_current = prev;
    }
  }
}
