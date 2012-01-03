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

#include <object/profile.hh>

namespace urbi
{
  namespace object
  {
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
        res.push_back(fp.second);
      std::sort(res.begin(), res.end(), &function_profiles_sort);

      return new List(res);
    }

    URBI_CXX_OBJECT_INIT(Profile)
    {
      proto_add(Object::proto);

      bind(SYMBOL(calls), function_profiles);
      bind(SYMBOL(maxFunctionCallDepth), &Profile::function_call_depth_max_get);
      bind(SYMBOL(totalCalls), &Profile::function_calls_get);
      bind(SYMBOL(totalTime), &Profile::totalTime);
      bind(SYMBOL(wallClockTime), &Profile::wallClockTime);
      bind(SYMBOL(yields), &Profile::yields_get);
    }

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
    {
      proto_add(Object::proto);

      bind(SYMBOL(name),     &FunctionProfile::name_get);
      bind(SYMBOL(calls),    &FunctionProfile::calls_get);
      bind(SYMBOL(selfTime), &FunctionProfile::selfTime);
      bind(SYMBOL(selfTimePer), &FunctionProfile::selfTimePer);
      bind(SYMBOL(time),     &FunctionProfile::time);
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
  }
}
