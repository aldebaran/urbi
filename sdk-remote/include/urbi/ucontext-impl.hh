/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef UCONTEXT_IMPL_HH
# define UCONTEXT_IMPL_HH

#include <set>

#include <boost/function.hpp>
#include <libport/system-warning-push.hh>
#include <boost/thread.hpp>
#include <libport/system-warning-pop.hh>

#include <libport/asio.hh>

#include <urbi/fwd.hh>
#include <urbi/uprop.hh>

namespace urbi
{
  /** The interfaces UContextImpl, UVarImpl, UObjectImpl and
   * UGenericCallbackImpl are what must be implemented to make an UObject
   * backend.
   */
  namespace impl
  {
    class URBI_SDK_API UContextImpl
    {
    public:
      virtual ~UContextImpl() = 0;
      /// Inject all loaded uobjects in the system by calling newUObjectClass.
      void init();
      /// Inject a specific uobject
      bool bind(const std::string& name, std::string rename = std::string());
      /// Called to instanciate the first Object of class.
      virtual void newUObjectClass(urbi::baseURBIStarter* s) = 0;
      virtual void newUObjectHubClass(urbi::baseURBIStarterHub* s) = 0;
      /// Retrieve a UObjectHub based on its name or return 0 if not found.
      virtual UObjectHub* getUObjectHub(const std::string& n);
      /// Retrieve a UObject based on its name or return 0 if not found.
      virtual UObject* getUObject(const std::string& n);
      /// Send Urbi code (ghost connection in plugin mode, default
      /// connection in remote mode).
      virtual void uobject_unarmorAndSend(const char* str) = 0;

      /// Send the string to the connection hosting the UObject.
      virtual void send(const char* str) = 0;

      /// Send buf to the connection hosting the UObject.
      virtual void send(const void* buf, size_t size) = 0;

      /// Bouncing overload.
      void send(const std::string& s);

      virtual void call(const std::string& object,
                        const std::string& method,
                        UAutoValue v1 = UAutoValue(),
                        UAutoValue v2 = UAutoValue(),
                        UAutoValue v3 = UAutoValue(),
                        UAutoValue v4 = UAutoValue(),
                        UAutoValue v5 = UAutoValue(),
                        UAutoValue v6 = UAutoValue()) = 0;

      virtual void declare_event(const UEvent* owner) = 0;

      virtual void emit(const std::string& object,
                        UAutoValue& v1,
                        UAutoValue& v2,
                        UAutoValue& v3,
                        UAutoValue& v4,
                        UAutoValue& v5,
                        UAutoValue& v6,
                        UAutoValue& v7) = 0;

      /// The mode in which the code is running.
      virtual UObjectMode getRunningMode() const = 0;
      /// The kernel major,minor version
      virtual std::pair<int, int> kernelVersion() const = 0;

      /// Yield execution until next cycle.
      /// Process pending messages in remote mode.
      virtual void yield() const = 0;

      /// Yield execution until \b deadline is met (see libport::utime()).
      virtual void yield_until(libport::utime_t deadline) const = 0;

      /// Yield execution for \b delay.
      /// Bounces to yield_until.
      virtual void yield_for(libport::utime_t delay) const;

      /** Yield execution until something else is scheduled,
       *  or until a message is received in remote mode.
       */
      virtual void yield_until_things_changed() const = 0;

      /** If \b s is true, mark the current task as having no side effect.
       * This call has no effect in remote mode.
       */
      virtual void side_effect_free_set(bool s) = 0;
      /// Get the current side_effect_free state.
      virtual bool side_effect_free_get() const = 0;
      virtual UVarImpl* getVarImpl() = 0;
      virtual UObjectImpl* getObjectImpl() = 0;
      virtual UGenericCallbackImpl* getGenericCallbackImpl() = 0;
      /// Default implementations appends to 'objects'.
      virtual void registerObject(UObject*o);
      virtual TimerHandle setTimer(UTimerCallback* cb) = 0;
      virtual void registerHub(UObjectHub*);
      virtual void removeHub(UObjectHub*) = 0;
      virtual void setHubUpdate(UObjectHub*, ufloat) = 0;
      /// Called by the urbiStarter after each UObject instanciation.
      virtual void instanciated(UObject*) = 0;
      typedef boost::unordered_map<std::string, UObject*>  objects_type;
      objects_type objects;
      typedef boost::unordered_map<std::string, UObjectHub*> hubs_type;
      hubs_type hubs;
      std::set<void*> initialized;

      /** Cleanup Stack RAII. Useful to delete temporaries created by
       * UValue casters.
       * The UObject API instanciates one for each call to a bound function or
       * notify.
       */
      class CleanupStack
      {
      public:
        CleanupStack(UContextImpl& owner);
        ~CleanupStack();
      private:
        UContextImpl& owner_;
      };

      /// Add \b ptr to the list of objects to delete whean cleanup() is called.
      template<typename T>
      void addCleanup(T* ptr);

      /// Add an arbitrary operation to perform at cleanup() time.
      void addCleanup(boost::function0<void> op);

      /// Push a new cleanup stack.
      void pushCleanupStack();
     /// Delete all pointers passed to addCleanup in current stack and pop.
      void popCleanupStack();

      /** Request a context lock from another thread to perform multiple
       * operations.
       */
      virtual void lock() = 0;
      /// Release lock acquired with lock()
      virtual void unlock() = 0;
      /// The io_service used by this context.
      virtual boost::asio::io_service& getIoService() = 0;

      /// RTP hooks for performance
      typedef void(*RTPSend)(UObject* rtp, const UValue& v);
      typedef void(*RTPSendGrouped)(UObject* rtp, const std::string&,
                                    const UValue&,
                                   libport::utime_t);
      RTPSend rtpSend;
      RTPSendGrouped rtpSendGrouped;
    private:
      typedef std::vector<std::vector<boost::function0<void> > > CleanupList;
      boost::thread_specific_ptr<CleanupList> cleanup_list_;
    };

    class URBI_SDK_API UObjectImpl
    {
    public:
      virtual ~UObjectImpl() = 0;
      // Called by uobject constructor
      virtual void initialize(UObject* owner) = 0;
      virtual void clean() = 0;
      virtual void setUpdate(ufloat period) = 0;
      virtual bool removeTimer(TimerHandle h) = 0;
    };

    class URBI_SDK_API UVarImpl
    {
    public:
      virtual ~UVarImpl() = 0;
      virtual void initialize(UVar* owner) = 0;
      virtual void clean() = 0;
      virtual void setOwned() = 0;
      virtual void sync() = 0;
      virtual void request() = 0;
      virtual void keepSynchronized() = 0;
      // Set the UVar value from v. Must deep-copy unless bypass mode is enabled
      virtual void set(const UValue& v) = 0;
      virtual const UValue& get() const = 0;
      virtual UDataType type() const = 0;
      virtual UValue getProp(UProperty prop) = 0;
      virtual void setProp(UProperty prop, const UValue& v) = 0;
      virtual bool setBypass(bool enable) = 0;
      virtual time_t timestamp() const = 0;
      virtual void unnotify() = 0;
      virtual void useRTP(bool enable) = 0;
      virtual void setInputPort(bool enable) = 0;
    };
  }
}

# include <urbi/ucontext-impl.hxx>

#endif
