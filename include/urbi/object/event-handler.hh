/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_EVENT_INSTANCE_HH
# define URBI_OBJECT_EVENT_INSTANCE_HH

# include <urbi/object/object.hh>
# include <urbi/object/event.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API EventHandler: public CxxObject
    {
      URBI_CXX_OBJECT(EventHandler, CxxObject);
      friend class Event;

    public:
      EventHandler(rEventHandler model);
      EventHandler(rEvent parent, rList payload);

      /// Effective trigger action.
      void trigger(bool detach);
      /// Event stopping function.
      /// Same synchronicity as trigger function.
      void stop();
      rEvent source();
      rList payload();

    private:
      typedef Event::callback_type callback_type;
      typedef Event::callbacks_type callbacks_type;

      /// Leave callbacks to trigger on stop.
      struct stop_job_type
      {
        stop_job_type(rSubscription s, objects_type& a, bool d)
          : subscription(s), args(a), detach(d)
        {}

        rSubscription subscription;
        objects_type args;
        // FIXME: not used.
        bool detach;
      };
      typedef std::vector<stop_job_type> stop_jobs_type;

      /// Listener jobs execution function.
      void
      trigger_job(const rSubscription& actions, const objects_type& args,
                  bool detach);
      /// Register the stop job.
      EventHandler& operator<<(const stop_job_type& stop_job);
      /// The parent Event of this handler.
      rEvent source_;
      /// The payload given to handler constructor.
      rList payload_;
      /// Copy of Boolean given to trigger, used for stop synchronicity.
      bool detach_;
      stop_jobs_type stop_jobs_;
      friend class Subscription;
    };
  }
}

# include <urbi/object/event-handler.hxx>

#endif
