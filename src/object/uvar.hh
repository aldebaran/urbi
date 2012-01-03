/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
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
      typedef std::pair<unsigned int, rObject> Callback;
      typedef std::vector<Callback> Callbacks;
      UVar();
      UVar(libport::intrusive_ptr<UVar> model);
      rObject update_(rObject arg);
      rObject update_timed_(rObject arg, libport::utime_t timestamp);
      rObject update_timed(rObject arg, libport::utime_t timestamp);
      rObject accessor(const objects_type&);
      /// Like accessor, but if fromCxx is true and the value is an UValue,
      /// return it instead of its content.
      rObject getter(bool fromCXX);
      /** Check if we have both in/out callbaks, periodicaly trigger
       * and return true if so.
       */
      bool loopCheck();
      rObject writeOwned(rObject newval);
      unsigned int notifyChange_(rObject cb);
      unsigned int notifyAccess_(rObject cb);
      unsigned int notifyChangeOwned_(rObject cb);
      bool removeNotifyChange(unsigned int);
      bool removeNotifyAccess(unsigned int);
      bool removeNotifyChangeOwned(unsigned int);
      bool removeCallback(Callbacks& cb, unsigned int id);
      // Return the UVar from its full name.
      static rObject fromName(const std::string& n);
      rList changeConnections; // bound in urbiscript
      ufloat timestamp; // idem
      ufloat rangemin, rangemax;
      rObject val;
      rObject valsensor;
    private:
      Callbacks change_;
      Callbacks access_;
      Callbacks changeOwned_;
      Callbacks accessInLoop_;
      /// Check and unlock getters stuck waiting.
      void checkBypassCopy();
      void changeAccessLoop();
      bool looping_;
      /// Set of runners currently in a notifyChange.
      std::vector<void*> inChange_;
      bool inAccess_;
      URBI_CXX_OBJECT(UVar, Primitive);
      int waiterCount_;
      libport::Symbol initialName;
      bool owned;
      static unsigned int uid_; // Unique id for callbacks
      friend class UConnection;
      URBI_ATTRIBUTE_ON_DEMAND_DECLARE(Event, changed);
    };
    /// Call some notifies on an UVar.
    void callNotify(runner::Runner& r, rUVar self,
                    UVar::Callbacks& notifyList, rObject sourceUVar);
    /// Call uconnections.
    void callConnections(runner::Runner&, rObject self, rList conns);
  }
}
#endif
