/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_EVENT_HH
# define URBI_OBJECT_EVENT_HH

# include <boost/unordered_map.hpp>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Event: public CxxObject
    {
    public:
      Event(rEvent model);
      Event(rEvent model, rList payload);
      URBI_CXX_OBJECT(Event);

    public:
      void emit(const objects_type& args);
      void onEvent(rExecutable guard, rExecutable enter, rExecutable leave);
      void stop();
      void syncEmit(const objects_type& args);
      rEvent syncTrigger(const objects_type& args);
      rEvent trigger(const objects_type& args);
      void localTrigger(const objects_type& args);
      void waituntil(rObject pattern);
      bool hasSubscribers() const;

    private:

      void waituntil_release(rObject payload);
      rEvent source();
      void trigger_job(rExecutable guard, rExecutable enter, rExecutable leave);

      /// Callbacks listening on this event.
      struct Actions
      {
        Actions(rExecutable g, rExecutable e, rExecutable l)
          : guard(g), enter(e), leave(l)
        {}
        rExecutable guard, enter, leave;
      };
      std::vector<Actions> listeners_;

      /// Job waiting for this event.
      typedef std::pair<runner::rRunner, rObject> waiter_type;
      std::vector<waiter_type> waiters_;

      /// Leave callbacks to trigger on stop.
      typedef std::pair<rExecutable, objects_type> stop_job_type;
      std::vector<stop_job_type> stop_jobs_;
      void register_stop_job(const stop_job_type& stop_job);

      /// Active instances of this event (handler => payload).
      typedef boost::unordered_map<rEvent, rList> actives_type;
      actives_type _active;
    };
  }
}

#endif
