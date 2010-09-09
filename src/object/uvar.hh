/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_STRING_UVAR_HH
# define OBJECT_STRING_UVAR_HH

# include <urbi/object/cxx-object.hh>
# include <runner/runner.hh>

# include <urbi/uvalue.hh>

namespace urbi
{
  namespace object
  {

    /** class providing access and change notification of a slot.
     *
     *  See uobject.u for the urbi part.
     */
    class UVar: public Primitive
    {
    public:
      UVar();
      UVar(libport::intrusive_ptr<UVar> model);
      rObject update_(rObject arg);
      rObject update_timed_(rObject arg, libport::utime_t timestamp);
      rObject update_timed(rObject arg, libport::utime_t timestamp);
      rObject accessor();
      /// Like accessor, but if fromCxx is true and the value is an UValue,
      /// return it instead of its content.
      rObject getter(bool fromCXX);
      /// Check if we have both in/out callbaks, periodicaly trigger if so.
      void loopCheck();
      rObject writeOwned(rObject newval);
    private:
      /// Check and unlock getters stuck waiting.
      void checkBypassCopy();
      void changeAccessLoop();
      bool looping_;
      /// Set of runners currently in a notifyChange.
      std::set<void*> inChange_;
      bool inAccess_;
      URBI_CXX_OBJECT_(UVar);
      int waiterCount_;
    };
  }
}
#endif
