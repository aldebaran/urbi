/*
 * Copyright (C) 2010, 2011, 2012, Gostai S.A.S.
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
# include <boost/unordered_set.hpp>

# include <libport/attributes.hh>

# include <urbi/object/cxx-object.hh>
# include <urbi/object/lobby.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Event: public CxxObject
    {
      friend class EventHandler;

    public:
      Event();
      Event(rEvent model);
      Event(rEvent model, rList payload);
      virtual ~Event();
      URBI_CXX_OBJECT(Event, CxxObject);

    public:
      typedef boost::signal0<void> signal_type;
      signal_type& destructed_get();
      signal_type& subscribed_get();
      signal_type& unsubscribed_get();
      ATTRIBUTE_R(signal_type, destructed);
      ATTRIBUTE_R(signal_type, subscribed);
      ATTRIBUTE_R(signal_type, unsubscribed);

    public:
      typedef boost::function1<void, const objects_type&> callback_type;

      class Subscription
      {
      public:
        void stop();

      private:
        friend class Event;
        Subscription(rEvent event, callback_type* cb);
        rEvent event_;
        callback_type* cb_;
      };

    public:
      /// C++ callback registration.
      Subscription onEvent(const callback_type& cb);
      /// Urbi callback registration.
      void onEvent(rExecutable guard, rExecutable enter, rExecutable leave = 0, bool sync = false);

      /// Synchronous emission.
      void syncEmit(const objects_type& args);

      /// Asynchronous emission.
      void emit(const objects_type& args);
      void emit();

      /// Asynchronous trigger.
      rEventHandler trigger(const objects_type& args);

      /// Synchronous trigger.
      rEventHandler syncTrigger(const objects_type& args);

      void waituntil(rObject pattern);
      bool hasSubscribers() const;

    private:
      void emit_backend(const objects_type& pl, bool detach);
      sched::rJob spawn_actions_job(rLobby lobby, rExecutable e,
                                    rProfile profile, const objects_type& args);
      rEventHandler trigger_backend(const objects_type& pl, bool detach);

      /** Callbacks listening on this event.
       *
       *  /!\ Keep me private, if an Actions survives its owner Event, it will
       *  SEGV.
       */
      struct Actions: public libport::RefCounted
      {
        Actions(rExecutable g, rExecutable e, rExecutable l, bool s);
	~Actions();

        bool operator==(const Actions& other) const;

        rExecutable guard, enter, leave;
        rProfile profile;
	/// Number of frozen tag this Actions is marked with.
        unsigned int frozen;
        /// Whether this onEvent is synchronous
        bool sync;
        std::vector<boost::signals::connection> connections;
        tag_stack_type tag_stack;
        /// Create job with this lobby when executing actions if set.
        rLobby lobby;
      };
      typedef libport::intrusive_ptr<Actions> rActions;

      void waituntil_release(rObject payload);
      void waituntil_remove(rTag what);
      rEvent source();

      /** The following three functions are callbacks installed on tags.
       *  The Actions argument is stored in the boost::bind.
       *  Since the callbacks are removed in ~Actions(), it is safe not to
       *  take shared pointers here.
       */
      void unregister(Actions*);
      void freeze(Actions*);
      void unfreeze(Actions*);
      typedef std::vector<rActions> listeners_type;
      listeners_type listeners_;

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

    public:
      /// Active instances of this event (handler => payload).
      typedef boost::unordered_set<rEventHandler> actives_type;
      ATTRIBUTE_R(actives_type, active);

    private:
      /// C++ callbacks
      typedef std::vector<callback_type*> callbacks_type;
      // Using an unordered set is slightly slower with few elements,
      // which is the most common case (isn't it?). So although it
      // entails linear-time search, std::vector is preferable.
      //
      // typedef boost::unordered_set<callback_type*> callbacks_type;
      callbacks_type callbacks_;
    };
  }
}

#endif
