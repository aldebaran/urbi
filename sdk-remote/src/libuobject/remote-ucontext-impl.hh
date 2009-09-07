/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#ifndef LIBUOBJECT_REMOTE_UCONTEXT_IMPL_HH
# define LIBUOBJECT_REMOTE_UCONTEXT_IMPL_HH
#include <urbi/uobject.hh>
#include <urbi/usyncclient.hh>

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
      virtual void call(const std::string& object,
                        const std::string& method,
                        UAutoValue v1 = UAutoValue(),
                        UAutoValue v2 = UAutoValue(),
                        UAutoValue v3 = UAutoValue(),
                        UAutoValue v4 = UAutoValue(),
                        UAutoValue v5 = UAutoValue(),
                        UAutoValue v6 = UAutoValue(),
                        UAutoValue v7 = UAutoValue(),
                        UAutoValue v8 = UAutoValue());
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
      virtual void setTimer(UTimerCallback* cb);
      virtual void registerHub(UObjectHub*);
      virtual void removeHub(UObjectHub*) ;
      virtual void setHubUpdate(UObjectHub*, ufloat);
      virtual void instanciated(UObject* obj);

    public:
      /// Dispatch a message on our connection
      UCallbackAction dispatcher(const UMessage& msg);
      USyncClient* getClient();
      USyncClient* client_;
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
      UTable& tableByName(const std::string& n);
      UCallbackAction clientError(const UMessage&);
      // Create it on demand.
      UObject* getDummyUObject();
      UObject* dummyUObject;
    };

    class URBI_SDK_API RemoteUObjectImpl: public UObjectImpl
    {
    public:
      virtual ~RemoteUObjectImpl();
      virtual void initialize(UObject* owner);
      virtual void clean();
      virtual void setUpdate(ufloat period);

    private:
      UObject* owner_;
      int period;
    };

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
      void update(const UValue& v);

    private:
      USyncClient* client_;
      UValue value_;
      UVar* owner_;
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
    };
  }
}
#endif // !LIBUOBJECT_REMOTE_UCONTEXT_IMPL_HH
