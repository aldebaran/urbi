/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
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
# include <urbi/object/subscription.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Event: public CxxObject
    {
      URBI_CXX_OBJECT(Event, CxxObject);
      friend class EventHandler;

    public:
      Event();
      Event(rEvent model);
      Event(rEvent model, rList payload);
      virtual ~Event();

    public:
      boost::function0<void> destructed, subscribed, unsubscribed;

    public:
      typedef Subscription::callback_type callback_type;

    public:
      /// C++ callback registration, synchronous by default.
      rSubscription onEvent(const callback_type& cb);
      /// Urbi callback registration.
      void onEvent(rExecutable guard,
                   rExecutable enter, rExecutable leave = 0, bool sync = false);
      typedef
        void (Event::*on_event_type)
        (rExecutable guard, rExecutable enter, rExecutable leave, bool sync);

      void subscribe(rSubscription s);
      /// Synchronous emission.
      void syncEmit(const objects_type& args = objects_type());

      /// Asynchronous emission.
      void emit(const objects_type& args = objects_type());

      /// Asynchronous trigger.
      rEventHandler trigger(const objects_type& args);

      /// Synchronous trigger.
      rEventHandler syncTrigger(const objects_type& args);

      void waituntil(rObject pattern);
      bool hasSubscribers() const;

      static
      runner::rJob
      action_job(rLobby lobby, const call_stack_type& stack,
                 rExecutable e,
                 rProfile profile, const objects_type& args);

    private:
      /** Handle synchronous/asynchronous invocation of subscribers for
       * emit and trigger
       * @param pl the emit payload
       * @param detach true if the emitter requested asynchronous notification
       * @param h set to the associated event handler in case of trigger,
       *        0 in case of emit
       */
      void emit_backend(const objects_type& pl, bool detach,
                        EventHandler*h = 0);

      rEventHandler trigger_backend(const objects_type& pl, bool detach);

      void waituntil_release(rObject payload);
      void waituntil_remove(rTag what);
      rEvent source();

      struct Waiter
      {
        Waiter(rTag ct, runner::Job* r, rObject& p);
        rTag controlTag;
        runner::Job* runner;
        rObject pattern;
      };
      /// Job waiting for this event.
      std::vector<Waiter> waiters_;

    public:
      /// Active instances of this event (handler => payload).
      typedef boost::unordered_set<rEventHandler> actives_type;
      ATTRIBUTE_R(actives_type, active);
      ATTRIBUTE_RW(rEvent, onSubscribe);
    private:
      friend class Subscription;
      /// C++ callbacks
      typedef std::vector<rSubscription> callbacks_type;
      // Using an unordered set is slightly slower with few elements,
      // which is the most common case (isn't it?). So although it
      // entails linear-time search, std::vector is preferable.
      //
      // typedef boost::unordered_set<callback_type*> callbacks_type;
      // /!\ Never ever yield while holding an iterator on callbacks_.
      callbacks_type callbacks_;
    };
  }
}

#endif
