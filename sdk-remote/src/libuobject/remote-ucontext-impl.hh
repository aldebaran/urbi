/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef LIBUOBJECT_REMOTE_UCONTEXT_IMPL_HH
# define LIBUOBJECT_REMOTE_UCONTEXT_IMPL_HH

# include <libport/package-info.hh>

# include <serialize/binary-o-serializer.hh>
# include <urbi/uobject.hh>
# include <urbi/usyncclient.hh>

namespace urbi
{
  namespace impl
  {
    class URBI_SDK_API RemoteUContextImpl
      : public impl::UContextImpl
    {
    public:
      /// Setup to work on given client
      RemoteUContextImpl(USyncClient* client);
      virtual ~RemoteUContextImpl();
      virtual void newUObjectClass(baseURBIStarter* s);
      virtual void newUObjectHubClass(baseURBIStarterHub* s);
      virtual void uobject_unarmorAndSend(const char* str);
      virtual void send(const char* str);
      virtual void send(const void* buf, size_t size);
      void send(const std::string& s)
      {
        send(s.c_str(), s.size());
      }
      virtual void call(const std::string& object,
                        const std::string& method,
                        UAutoValue v1 = UAutoValue(),
                        UAutoValue v2 = UAutoValue(),
                        UAutoValue v3 = UAutoValue(),
                        UAutoValue v4 = UAutoValue(),
                        UAutoValue v5 = UAutoValue(),
                        UAutoValue v6 = UAutoValue());
      virtual void declare_event(const UEvent* owner);
      virtual void emit(const std::string& object,
                        UAutoValue& v1,
                        UAutoValue& v2,
                        UAutoValue& v3,
                        UAutoValue& v4,
                        UAutoValue& v5,
                        UAutoValue& v6,
                        UAutoValue& v7
                        );
      virtual UObjectMode getRunningMode() const;
      virtual std::pair<int, int> kernelVersion() const;
      virtual void yield() const;
      virtual void yield_until(libport::utime_t deadline) const;
      virtual void yield_until_things_changed() const;
      virtual void side_effect_free_set(bool s);
      virtual bool side_effect_free_get() const;
      virtual UVarImpl* getVarImpl();
      virtual UObjectImpl* getObjectImpl();
      virtual UGenericCallbackImpl* getGenericCallbackImpl();
      virtual TimerHandle setTimer(UTimerCallback* cb);
      virtual void registerHub(UObjectHub*);
      virtual void removeHub(UObjectHub*) ;
      virtual void setHubUpdate(UObjectHub*, ufloat);
      virtual void instanciated(UObject* obj);
      virtual void lock();
      virtual void unlock();
      virtual boost::asio::io_service& getIoService();

    public:
      /// Dispatch a message on our connection
      UCallbackAction dispatcher(const UMessage& msg);
      USyncClient* getClient();
      /** Make a new RTP link with the engine, using hash key \b key.
       * The link is established asynchronously and is ready when
       * RTPLinks[key] != 0.
       * Check if initialization is not already in progress by calling
       * RTPLinks.has(key) first.
       */
      void makeRTPLink(const std::string& key);

      /// Handle an assignment request.
      void assignMessage(const std::string& name, const UValue& v, time_t ts,
                         bool bypass=false, UValue* target=0,
                         time_t* tsTarget = 0);

      /// Handle a function call.
      /// \param name  function name (should be array[1])
      /// \param var   where to store the result (should be array[2])
      /// \param args  an array which *will* shitfed by 3 to find the
      ///              function call arguments.
      void evalFunctionMessage(const std::string& name,
                               const std::string& var,
                               UList& args);

      /// Handle a RTP enable/disable message.
      /// \param varname the variable name on which it applies
      /// \param state enable rtp if nonzero
      void setRTPMessage(const std::string& varname,
                         int state);

      /// Enable/disable binary serialization mode.
      void setSerializationMode(bool);

      /// Return the result of the evaluation of the given expression
      UMessage* syncGet(const std::string& exp, libport::utime_t timeout=0);

      /** Notify that data was sent.
       * Just write dataSent_ if called from dispatch, or add a '; and flush.
       */
      void markDataSent();
      USyncClient* backend_;

      /** Return the name of the hook-point UObject for UVar notifies with
       *  unknown source.
       */
      std::string hookPointName();
      /// True if we received a clientError message.
      bool closed_;
#define TABLE(Type, Name)                       \
    private:                                    \
      Type Name ## _;                           \
    public:                                     \
      Type& Name()                              \
      {                                         \
        return Name ## _;                       \
      }
      TABLE(UTable, accessmap);
      TABLE(UTable, eventmap);
      TABLE(UTable, eventendmap);
      TABLE(UTable, functionmap);
      TABLE(UTable, monitormap);
      TABLE(UVarTable, varmap);
      TABLE(UTimerTable, timermap);
#undef TABLE
      libport::Lockable tableLock;
      UValue localCall(const std::string& object,
                const std::string& method,
                UAutoValue v1 = UAutoValue(),
                UAutoValue v2 = UAutoValue(),
                UAutoValue v3 = UAutoValue(),
                UAutoValue v4 = UAutoValue(),
                UAutoValue v5 = UAutoValue(),
                UAutoValue v6 = UAutoValue(),
                UAutoValue v7 = UAutoValue(),
                UAutoValue v8 = UAutoValue());
      UTable& tableByName(const std::string& n);
      UCallbackAction clientError(const UMessage&);
      UCallbackAction onRTPListenMessage(const UMessage&);
      // Create it on demand.
      UObject* getDummyUObject();
      void onTimer(UTimerCallback* cb);
      UObject* dummyUObject;
      typedef boost::unordered_map<std::string,
        std::pair<libport::AsyncCallHandler, UTimerCallback*> > TimerMap;
      TimerMap timerMap;
      libport::Lockable mapLock;
      // Pool of RTP connections (key= UVar name)
      typedef boost::unordered_map<std::string, UObject*> RTPLinks;
      RTPLinks rtpLinks;
      // Use RTP connections in this context if available
      bool enableRTP;
      unsigned int dispatchDepth;
      // Stream to use for urbiscript output
      LockableOstream* outputStream;
      // True when something was sent. Reset by dispatcher().
      bool dataSent;
      // Send serialized binary messages if set.
      bool serializationMode;
      libport::serialize::BinaryOSerializer* oarchive;
      libport::PackageInfo::Version version;
      // Name of the hook-point UObject for UVars.
      std::string hookPointName_;
      // Shared RTP link cached instance.
      UObject* sharedRTP_;
      #define URBI_REMOTE_RTP_INIT_CHANNEL "__remote_rtp_init"
    };

    class URBI_SDK_API RemoteUObjectImpl: public UObjectImpl
    {
    public:
      virtual ~RemoteUObjectImpl();
      virtual void initialize(UObject* owner);
      virtual void clean();
      virtual void setUpdate(ufloat period);
      virtual bool removeTimer(TimerHandle h);
      void onUpdate();

    private:
      UObject* owner_;
      ufloat period;
      libport::AsyncCallHandler updateHandler;
    };
    class RemoteUGenericCallbackImpl;
    class URBI_SDK_API RemoteUVarImpl: public UVarImpl
    {
    public:
      virtual void initialize(UVar* owner);
      virtual void clean();
      virtual void setOwned();
      virtual void sync();
      virtual void request();
      virtual void keepSynchronized();
      virtual void set(const UValue& v);
      virtual const UValue& get() const;
      virtual ufloat& in();
      virtual ufloat& out();
      virtual UDataType type() const;
      virtual UValue getProp(UProperty prop);
      virtual void setProp(UProperty prop, const UValue& v);
      virtual bool setBypass(bool enable);
      virtual time_t timestamp() const;
      virtual void unnotify();
      virtual void useRTP(bool enable);
      virtual void setInputPort(bool enable);
    private:
      // transmit the value to the remote kernel
      void transmit(const UValue& v, libport::utime_t timestamp);
      // Transmit in serialized mode.
      void transmitSerialized(const UValue& v, libport::utime_t time);
      USyncClient* client_;
      // Last value, shared among all UVarImpls with the same name.
      UValue* value_;
      UVar* owner_;
      time_t* timestamp_; // shared among all UVarImpls with same name.
      friend class RemoteUGenericCallbackImpl;
      friend class RemoteUContextImpl;
      std::vector<RemoteUGenericCallbackImpl*> callbacks_;
      bool bypass_;
    };

    class URBI_SDK_API RemoteUGenericCallbackImpl: public UGenericCallbackImpl
    {
    public:
      virtual void initialize(UGenericCallback* owner, bool owned);
      virtual void initialize(UGenericCallback* owner);
      virtual void registerCallback();
      virtual void clear();

    private:
      UGenericCallback* owner_;
      friend class RemoteUVarImpl;
    };
  }
}
#endif // !LIBUOBJECT_REMOTE_UCONTEXT_IMPL_HH
