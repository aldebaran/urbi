/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_SUBSCRIPTION_HH
#define URBI_OBJECT_SUBSCRIPTION_HH

# include <boost/function.hpp>
# include <boost/signal.hpp>

# include <urbi/object/cxx-object.hh>
# include <runner/state.hh>

namespace urbi
{
  namespace object
  {

  /** Instances of this class represent one subscription to an event.
  * This subscription can be an at, a custom C++ subscription with a
  * C++ callback, or a custom urbiscript subscription.
  *
  * Only at-kind hold a reference to the subscribed event, since they need
  * to perform some actions synchronously.
  * Other kinds are unregistered asynchronously when the event triggers.
  */
  class Subscription: public CxxObject
  {
    URBI_CXX_OBJECT(Subscription, CxxObject);
  public:
    typedef boost::function1<void, const objects_type&> callback_type;
    void stop();
    Subscription();
    Subscription(libport::intrusive_ptr<Subscription> model);
    Subscription(rEvent source, rExecutable g, rExecutable e, rExecutable l, bool s);
    Subscription(callback_type* cb);
    // Factor initialization code
    void init_();
    ~Subscription();
    /** Run the subscription in this job.
     * @param src source event
     * @param payload event payload
     * @param h event handler in case this is a trigger. 0 if it is an emit.
     * @param detach detach state asked by emitter
     * @param sync true if run_sync was called from the emitter job.
     */
    void run_sync(rEvent src, const objects_type& payload, EventHandler* h,
                  bool detach, bool sync);
  private:
    friend class Event;

    // Configuration
    /// Do not call if not enabled
    ATTRIBUTE_RW(bool, enabled);
    /// Switch between synchronous or asynchronous call
    ATTRIBUTE_RW(bool, asynchronous);
    /** Maximum number of parallel processing allowed. Will drop if reached.
     * 0 means 'unlimited'.
     */
    ATTRIBUTE_RW(int, maxParallelEvents);
    /// Min time interval between two calls
    ATTRIBUTE_RW(ufloat, minInterval);


    // Statistics
    /// Number of notifies the subscription is currently processing
    ATTRIBUTE_RW(int, processing);
    /// Timestamp of last call
    ATTRIBUTE_RW(ufloat, lastCall);
    /// Number of times we were called
    ATTRIBUTE_RW(long, callCount);
    /// Total time spent in callback
    ATTRIBUTE_RW(double, totalCallTime);
    ATTRIBUTE_RW(double, minCallTime);
    ATTRIBUTE_RW(double, maxCallTime);


    /** True if the subscription wants out.
    * We do not hold a pointer to owner event to avoil
    * circular dependencies. So removal is asynchronous.
    */
    ATTRIBUTE_RW(bool, disconnected);

    // Mode one, slot callback
    ATTRIBUTE_RW(rObject, onEvent);

    // Mode two, at-specific callback triplet
    void unregister();
    void freeze();
    void unfreeze();
    // Event is only set for mode-two. We need it to call unsubscribed_
    rEvent event_;
    rExecutable guard, enter, leave;
    rProfile profile;
    std::vector<boost::signals::connection> connections;
    /// Number of frozen tags this Subscription is marked with.
    unsigned int frozen;
    runner::State::tag_stack_type tag_stack;
    /// Create job with this lobby when executing actions if set.
    rLobby lobby;
    /// Call stack from the event handler.
    call_stack_type call_stack;

    // Mode three, c++ callback
    callback_type* cb_;
  };

  }
}
#endif
