/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_PROFILE_HH
# define URBI_OBJECT_PROFILE_HH

# include <boost/unordered_map.hpp>

# include <libport/attributes.hh>
# include <libport/symbol.hh>
# include <libport/utime.hh>

# include <runner/fwd.hh>
# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class FunctionProfile: public CxxObject
    {
    public:
      FunctionProfile();
      FunctionProfile(libport::intrusive_ptr<FunctionProfile> model);
      virtual ~FunctionProfile();
      ATTRIBUTE_R(libport::Symbol, name);
      ATTRIBUTE_R(unsigned, calls);
      ATTRIBUTE_R(unsigned, self_time);
      ATTRIBUTE_R(unsigned, time);
      ufloat selfTime() const;
      ufloat selfTimePer() const;
      ufloat time() const;
      int totalCalls() const;
      FunctionProfile& operator +=(const FunctionProfile& other);
    private:
      friend class ::runner::Interpreter;
      friend class Profile;

      URBI_CXX_OBJECT(FunctionProfile, CxxObject);
    };

    typedef boost::unordered_map<void*, rFunctionProfile> FunctionProfiles;
    class Profile: public CxxObject
    {
    public:
      Profile();
      Profile(libport::intrusive_ptr<Profile> model);
      virtual ~Profile();
      Profile& operator+=(const Profile& other);
      ufloat totalTime() const;
      ufloat wallClockTime() const;
      ATTRIBUTE_R(unsigned, yields);
      ATTRIBUTE_R(unsigned, wall_clock_time);
      ATTRIBUTE_R(unsigned, total_time);
      ATTRIBUTE_R(unsigned, function_calls);
      ATTRIBUTE_R(unsigned, function_call_depth_max);
      ATTRIBUTE_R(FunctionProfiles, functions_profile);

    private:
      void step(libport::utime_t& checkpoint, void* function_current);
      friend class ::runner::Interpreter;

      URBI_CXX_OBJECT(Profile, CxxObject);
    };
  }
}

#endif
