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

# include <boost/signal.hpp>
# include <boost/unordered_map.hpp>

# include <urbi/object/cxx-object.hh>

# include <runner/interpreter.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Event: public CxxObject
    {
    public:
      Event();
      Event(rEvent model);
      Event(rEvent model, rList payload);
      URBI_CXX_OBJECT(Event);

      typedef boost::function1<void, const objects_type&> callback_type;

      class Subscription
      {
      public:
        void stop();

      private:
        friend class Event;
        Subscription(rEvent event, const callback_type* cb);
        rEvent event_;
        const callback_type* cb_;
      };

    public:
      void emit(const objects_type& args);
      void emit();
      void onEvent(rExecutable guard, rExecutable enter, rExecutable leave);
      Subscription onEvent(const callback_type& cb);
      void stop();
      void syncEmit(const objects_type& args);
      rEvent syncTrigger(const objects_type& args);
      rEvent trigger(const objects_type& args);
      void localTrigger(const objects_type& args, bool detach);
      void waituntil(rObject pattern);
      bool hasSubscribers() const;

    private:
      void emit_backend(const objects_type& args, bool detach);
      rEvent trigger_backend(const objects_type& args, bool detach);
      void stop_backend(bool detach);

      /** Callbacks listening on this event.
       *
       *  /!\ Keep me private, if an Actions survives its owner Event, it will
       *  SEGV.
       */
      struct Actions: public libport::RefCounted
      {
        Actions(rExecutable g, rExecutable e, rExecutable l)
          : guard(g), enter(e), leave(l), frozen(0)
        {}
	~Actions();

        bool
        operator==(const Actions& other)
        {
          return (guard == other.guard
                  && enter == other.enter
                  && leave == other.leave);
        }

        rExecutable guard, enter, leave;
	/// Number of frozen tag this Actions is marked with.
        unsigned int frozen;
        std::vector<boost::signals::connection> connections;
        runner::tag_stack_type tag_stack;
      };
      typedef libport::intrusive_ptr<Actions> rActions;

      void waituntil_release(rObject payload);
      void waituntil_remove(rTag what);
      rEvent source();
      void trigger_job(const rActions& actions, bool detach);

      /** The following three functions are callbacks installed on tags.
       *  The Actions argument is stored in the boost::bind.
       *  Since the callbacks are removed in ~Actions(), it is safe not to
       *  take shared pointers here.
       */
      void unregister(Actions*);
      void freeze(Actions*);
      void unfreeze(Actions*);
      typedef std::vector<rActions> Listeners;
      Listeners listeners_;

      struct Waiter
      {
        Waiter(rTag ct, runner::Runner* r, rObject& p)
        : controlTag(ct), runner(r), pattern(p) {}
        rTag controlTag;
        runner::Runner* runner;
        rObject pattern;
      };
      /// Job waiting for this event.
      std::vector<Waiter> waiters_;

      /// Leave callbacks to trigger on stop.
      typedef std::pair<rExecutable, objects_type> stop_job_type;
      std::vector<stop_job_type> stop_jobs_;
      void register_stop_job(const stop_job_type& stop_job);

      /// Active instances of this event (handler => payload).
      typedef boost::unordered_map<rEvent, rList> actives_type;
      actives_type _active;

      /// C++ callbacks
      typedef std::vector<callback_type*> callbacks_type;
      callbacks_type callbacks_;
      /// Running C++ callbacks of an event instance
      typedef std::vector<callback_type> callbacks_instance_type;
      callbacks_instance_type callbacks_instance_;

    };
  }
}

#endif
