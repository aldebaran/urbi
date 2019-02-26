/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <cstdarg>

#include <libport/bind.hh>
#include <libport/lexical-cast.hh>
#include <libport/foreach.hh>
#include <libport/hash.hh>
#include <libport/lexical-cast.hh>
#include <libport/synchronizer.hh>

#include <kernel/config.h>

#include <urbi/kernel/uconnection.hh>
#include <urbi/kernel/userver.hh>
#include <kernel/uvalue-cast.hh>
#include <kernel/uobject.hh>

#include <urbi/object/cxx-primitive.hh>
#include <urbi/object/dictionary.hh>
#include <object/finalizable.hh>
#include <urbi/uevent.hh>
#include <urbi/object/event.hh>
#include <urbi/object/float.hh>
#include <urbi/object/global.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/slot.hh>
#include <urbi/object/string.hh>
#include <urbi/object/tag.hh>
#include <urbi/object/symbols.hh>
#include <object/system.hh>
#include <object/uconnection.hh>
#include <urbi/object/urbi-exception.hh>
#include <object/uvalue.hh>

#include <urbi/runner/raise.hh>
#include <runner/job.hh>

#include <eval/call.hh>

#include <urbi/uobject.hh>
#include <urbi/uexternal.hh>
#include <urbi/uvalue-serialize.hh>

#include <urbi/ucontext-factory.hh>

#if defined WIN32
# define APPLE_LINUX_WINDOWS(Apple, Linux, Windows) Windows
#elif defined __APPLE__
# define APPLE_LINUX_WINDOWS(Apple, Linux, Windows) Apple
#else
# define APPLE_LINUX_WINDOWS(Apple, Linux, Windows) Linux
#endif

GD_CATEGORY(Urbi.UObject);

// Make it more readable.
using object::List;
using object::rPath;
//Next using disabled, fails on gcc3, conflict with boost::filesystem::Path.
//using object::Path;
using object::rEvent;
using object::rObject;
using object::rSlot;
using object::rLobby;
using object::objects_type;
using object::void_class;
using object::nil_class;
using libport::Symbol;
using kernel::server;

// Declare our UObject implementation
namespace urbi
{
  namespace impl
  {
    /* Link between UObject and associated object::Object.
     * When instanciated from urbiscript, we do not hold a ref count,
     * so that destruction is possible from urbiscript.
     * But when instanciated from C++, we do
     */
    /// Link to an rObject, refcounted or not
    class Link
    {
    public:
      Link()
        : norefPtr(0)
        , isRef (false)
        , uobject(0)
      {
      }

      Link(object::Object* target, bool refCount, urbi::UObject* uob)
        : norefPtr(0)
        , isRef (refCount)
        , uobject(uob)
      {
        set(target, refCount);
      }

      void set(object::Object* target, bool refCount)
      {
        isRef = refCount;
        if (refCount)
          refPtr = target;
        else
          norefPtr = target;
      }

      void setUObject(UObject* o)
      {
        uobject = o;
      }

      void operator = (const Link& b)
      {
        set(b.isRef?b.refPtr.get():b.norefPtr, b.isRef);
        uobject = b.uobject;
      }

      bool operator == (const Link& b) const
      {
        return uobject->__name == b.uobject->__name;
      }

      // Downgrade from refcounted to not-refcounted.
      void deref()
      {
        if (isRef)
        {
          norefPtr = refPtr.get();
          refPtr = 0;
          isRef = false;
        }
      }

      void ref()
      {
        if (!isRef)
        {
          refPtr = norefPtr;
          norefPtr = 0;
          isRef = true;
        }
      }

      void resetPtr()
      {
        norefPtr = 0;
        refPtr = 0;
      }

      void resetUObject()
      {
        uobject = 0;
      }

      UObject* getUObject()
      {
        return uobject;
      }

      object::Object* getRef()
      {
        return isRef ? refPtr.get() : norefPtr;
      }

    private:
      object::Object* norefPtr;
      object::rObject refPtr;
      bool isRef;
      urbi::UObject* uobject;
    };


    typedef boost::unordered_map<std::string, Link> ObjectLinks;

    class KernelUContextImpl: public UContextImpl
    {
    public:
      /// FIXME: only one kernel for now
      KernelUContextImpl();
      virtual UObjectHub* getUObjectHub(const std::string& n);
      virtual UObject* getUObject(const std::string& n);
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
                         UAutoValue v6 = UAutoValue()
                         );
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
      virtual void yield_for(libport::utime_t delay) const;
      virtual void side_effect_free_set(bool s);
      virtual bool side_effect_free_get() const;
      virtual UVarImpl* getVarImpl();
      virtual UObjectImpl* getObjectImpl();
      virtual UGenericCallbackImpl* getGenericCallbackImpl();
      virtual TimerHandle setTimer(UTimerCallback* cb);
      virtual void registerHub(UObjectHub*);
      virtual void removeHub(UObjectHub*) ;
      virtual void setHubUpdate(UObjectHub*, ufloat);
      virtual void instanciated(UObject* uob);
      virtual void lock();
      virtual void unlock();
      virtual Barrier* barrier();
      virtual boost::asio::io_service& getIoService();
      static inline KernelUContextImpl* instance() {return instance_;}

    private:
      static KernelUContextImpl* instance_;
    };

    KernelUContextImpl* KernelUContextImpl::instance_ = 0;
    class KernelUGenericCallbackImpl;
    class KernelUObjectImpl: public UObjectImpl
    {
    public:
      virtual void initialize(UObject* owner);
      virtual void clean();
      virtual void setUpdate(ufloat period);
      virtual bool removeTimer(TimerHandle h);
      virtual ~KernelUObjectImpl();
      std::string bareName() const;
    private:
      UObject* owner_;
      friend class KernelUGenericCallbackImpl;
      std::vector<UGenericCallback*> callbacks_;
    };

    class KernelUVarImpl: public UVarImpl
    {
    public:
      KernelUVarImpl();
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
      object::rSlot slot() { return slot_;}
      void initialize(UVar* owner, object::rSlot slot);
    private:
      libport::Lockable asyncLock_; // lock for pending_
      /* Schedule an operation to be executed by the main thread, preventing
       * the destruction of this object and the owner UVar until the operation
       * is completed. If sync, blocks until completion.
       */
      void schedule(libport::Symbol s, boost::function0<void> op,
                    bool sync=false) const;
      void async(boost::function0<void> op);
      void async_get(UValue**) const;
      void async_get_prop(UValue&, UProperty);
      unsigned pending_; // number of pending asynchronous operations
      urbi::UVar* owner_;
      bool bypassMode_;
      mutable UValue cache_;
      typedef std::vector<KernelUGenericCallbackImpl*> callbacks_type;
      callbacks_type callbacks_;
      std::pair<std::string, std::string> splitName_;
      ATTRIBUTE_RW(object::rSlot, slot);
      friend class KernelUGenericCallbackImpl;
    };

    class KernelUGenericCallbackImpl: public UGenericCallbackImpl
    {
    public:
      virtual void initialize(UGenericCallback* owner, bool owned);
      virtual void initialize(UGenericCallback* owner);
      virtual void registerCallback();
      virtual void clear();
    private:
      UGenericCallback* owner_;
      bool owned_;
      bool registered_;
      /// The Primitive we put in 'change'.
      rObject callback_;
      /// The connection we implicitly created or 0.
      object::rUConnection connection_;
      /// Input port the notify was redirected to, or empty
      ATTRIBUTE_RW(object::rSlot, inPort);
      friend class KernelUObjectImpl;
      friend class KernelUVarImpl;
    };
  }
}

using urbi::impl::ObjectLinks;

static ObjectLinks& get_object_links()
{
  static ObjectLinks* v = new ObjectLinks();
  return *v;
}
// Where to store uobjects
static rObject where;
// List of already initialized instances.
static std::set<void*> initialized;
// Tracke mode
static bool trace_uvars = 0;
static libport::file_library uobjects_path;

// Link between rObject and UObject.
static ObjectLinks& object_links = get_object_links();
static libport::Lockable object_links_lock;


using urbi::uobjects::get_base;
using urbi::uobjects::uname_split;
using urbi::uobjects::StringPair;

static void setTrace(rObject, bool v)
{
  trace_uvars = v;
}

namespace
{
  inline
  rObject
  xget_base(const std::string& objname,
            const std::string& error = "no such UObject: %s")
  {
    if (rObject res = get_base(objname))
      return res;
    FRAISE(error, objname);
  }
}


// Object,method names of last call, using for operation tracing
static std::vector<std::pair<std::string, std::string> > bound_context;

#define MAKE_VOIDCALL(ptr, cls, meth)                   \
  object::primitive(                                    \
    boost::function1<void, rObject>(                    \
      boost::bind(&cls::meth, ptr)))                    \

#ifdef SCHED_CORO_OSTHREAD
# define CHECK_MAINTHREAD()
#else
# define CHECK_MAINTHREAD()				\
  if (::kernel::urbiserver->isAnotherThread())          \
    throw std::runtime_error("Current thread is not main thread")
#endif

/* Statistics gathering about UObject function call time.
 */
namespace Stats
{
  struct Value
  {
    unsigned sum;
    unsigned min;
    unsigned max;
    unsigned count;
  };
  typedef boost::unordered_map<std::string, Value> Values;
  static Values hash;
  static bool enabled = false;
  static void add(void*, const std::string& key, libport::utime_t d)
  {
    if (!enabled)
      return;
    Values::iterator i = hash.find(key);
    if (i == hash.end())
    {
      Value& v = hash[key];
      v.sum = v.min = v.max = d;
      v.count = 1;
    }
    else
    {
      i->second.sum += d;
      i->second.count++;
      i->second.min = std::min(i->second.min, (unsigned)d);
      i->second.max = std::max(i->second.max, (unsigned)d);
    }
  }

  static void clear(rObject)
  {
    hash.clear();
  }

  static object::rDictionary get(rObject)
  {
    using object::Float;
    object::rDictionary res = new object::Dictionary();
    foreach (Values::value_type &v, hash)
    {
      object::rList l = new object::List();
      *l << new Float(v.second.sum / v.second.count)
         << new Float(v.second.min)
         << new Float(v.second.max)
         << new Float(v.second.count);
      res->set(new object::String(v.first), l);
      /*
      size_t sep = v.first.find_first_of(' ');
      std::string sPtr = v.first.substr(0, sep);
      std::string name = v.first.substr(sep+1, v.first.npos);
      */
    }
    return res;
  }
  static void enable(rObject, bool state)
  {
    enabled = state;
  }
};

static inline void traceOperation(urbi::UVar*v, libport::Symbol op)
{
  if (trace_uvars)
  {
    StringPair p = uname_split(v->get_name());
    rObject o = xget_base(p.first);
    object::global_class
      ->slot_get_value(SYMBOL(UVar))
      ->slot_get_value(op)
      ->call(SYMBOL(syncEmit), o, o->slot_get_value(Symbol(p.second)),
             object::to_urbi(bound_context));
  }
}

static void periodic_call(rObject, libport::ufloat interval, rObject method,
                          libport::Symbol msg, object::objects_type args)
{
  runner::Job& r = ::kernel::runner();
  libport::utime_t delay = libport::seconds_to_utime(interval);
  while (true)
  {
    urbi::setCurrentContext(urbi::impl::KernelUContextImpl::instance());
    eval::call_apply(r, method, msg, args);
    libport::utime_t target = libport::utime() + delay;
    r.yield_until(target);
  }
}

// Call function and unlock semaphore.
static void call_and_unlock(boost::function0<void> f, libport::Semaphore& s)
{
  f();
  s++;
}

void uobjects_reload()
{
  urbi::impl::KernelUContextImpl::instance()->init();
  foreach (urbi::baseURBIStarterHub* i, urbi::baseURBIStarterHub::list())
    if (!libport::has(initialized, i))
      initialized.insert(i);

  foreach (urbi::baseURBIStarter* i, urbi::baseURBIStarter::list())
    if (!libport::has(initialized, i))
      initialized.insert(i);
}

const libport::file_library
uobject_uobjects_path()
{
  return uobjects_path;
}

/// The UObject load path as a List of Path.
static
List::value_type
uobject_uobjectsPath(const rObject&)
{
  List::value_type list;

  foreach (const libport::path& p,
           uobjects_path.search_path_get())
    list << new object::Path(p);
  return list;
}

/// Change the UObject load path from a List of Path.
static
void
uobject_uobjectsPathSet(const rObject&, List::value_type list)
{
  uobjects_path.search_path().clear();
  foreach (rObject p, list)
  {
    rPath path = p->as<object::Path>();
    uobjects_path.search_path().push_back(path->value_get());
  }
}

static std::vector<std::string> all_uobjects(rObject)
{
  libport::BlockLock bl(object_links_lock);
  std::vector<std::string> res;
  for (ObjectLinks::iterator i = object_links.begin(); i!= object_links.end();
       ++i)
    res << i->first;
  return res;
}

/* Setter/getter put in oset/oget, bouncing to UObject notifyChange or
 * notifyAccess function.
 */
static rObject
wrap_ucallback_notify(const object::objects_type& ol,
                      urbi::UGenericCallback* ugc,
                      const std::string& traceName,
                      bool isChange)
{
  if (isChange && ol.size() != 3)
    runner::raise_arity_error(ol.size()-1, 2);
  if (!isChange && ol.size() != 2)
    runner::raise_arity_error(ol.size()-1, 1);
  URBI_SCOPE_DISABLE_DEPENDENCY_TRACKER;
  { // cannot put two finally in the same scope
  /* if isChange:  bound to oset. Args: (obj, newVal, slot)
  *  else: bound to oget. Args: (obj, slot)
  */
  GD_FPUSH_TRACE("Calling bound notify %s sync=%s", traceName,
                 ugc->isSynchronous());
  urbi::setCurrentContext(urbi::impl::KernelUContextImpl::instance());

  // UObject backtrace
  bound_context
    << std::make_pair(&ugc->owner ? ugc->owner.__name : "unknown",
                      ugc->name);
  bool dummy = false;
  FINALLY(((bool, dummy)), bound_context.pop_back(); GD_INFO_TRACE("Done"));

  rSlot slot = static_cast<urbi::impl::KernelUVarImpl*>(ugc->target->impl_)
    ->slot_get();
  /* We need to write the value to the backend, as Slot will not do it, and
   * UObject will read it instead of using the value in argument.
   */
  if (isChange)
    slot->value_set(ol[1]);

  urbi::impl::KernelUGenericCallbackImpl& impl =
    static_cast<urbi::impl::KernelUGenericCallbackImpl&>  (*ugc->impl_);
  urbi::UList l;
  l.array << new urbi::UValue();

  l[0].storage = ugc->target;
  libport::utime_t t = libport::utime();
  if (!ugc->isSynchronous())
  {
    /* This is an asynchronous notifychange. For synchronous ones we pass the
     * UVar to the callback function, but it is not appropriate for asynchronous
     * calls: we want each call to receive the value at the time of the call.
     * So we pass the value. It just means our user cannot take a UVar& as
     * callback argument, and must take the value directly.
     */
     l[0] = uvalue_cast(slot->value(object::nil_class, true));
     ugc->eval(l);
  }
  else
    ugc->syncEval(l);


  if (isChange && impl.inPort_get()) // Input port do not store the value
  { // FIXME: actualy we never wrote to the damm port, so this should be useless
    GD_FINFO_TRACE("Reseting value on input port %s", impl.inPort_get());
    impl.inPort_get()->value_set(object::void_class);
  }
  if (isChange && slot->local_slot_get(SYMBOL(inputPort)))
  {
    slot->value_set(object::void_class);
  }
  rObject ret = object::void_class;
  // If getter, it expects the value back, but the UObject just wrote it.
  if (!isChange)
    ret = slot->output_value_get();

  Stats::add(ol.front().get(), traceName, libport::utime() - t);
  return ret;
}}

static void write_and_unfreeze_mainthread(object::rTag* tag,
                                          urbi::UValue& resSlot,
                                          std::string& exceptionSlot,
                                          bool* async_abort,
                                          urbi::UValue* result,
                                          std::string exceptionResult)
{
  // Magic, we are in the main thread, so no possible race with wrap_ucallback!
  if (*async_abort)
  { // The call was aborted on the other end. Do absolutely nothing.
    GD_INFO_LOG("Aborted threaded call, ignoring return value");
    delete async_abort;
  }
  else
  {
    exceptionSlot = exceptionResult;
    resSlot = *result;
    delete result;
    (*tag)->unfreeze();
    delete tag;
  }
}

static void write_and_unfreeze(urbi::UValue& r, std::string& exception,
                               bool* async_abort,
                               object::rTag* tag,
                               urbi::UValue& v, const std::exception* e)
{
  std::string exceptionMessage;

  if (e)
    exceptionMessage = e->what();
  if (server().isAnotherThread())
    server().schedule_fast(
                      boost::bind(&write_and_unfreeze_mainthread, tag,
                                  boost::ref(r), boost::ref(exception),
                                  async_abort, new urbi::UValue(v),
                                  exceptionMessage));
  else
  {
    exception = exceptionMessage;
    r = v;
    (*tag)->unfreeze();
    delete tag;
  }

}

// UObject bound function.
static rObject wrap_ucallback(const object::objects_type& ol,
                              urbi::UGenericCallback* ugc,
                              const std::string& message, bool withThis)
{
  GD_FPUSH_TRACE("Calling bound function %s", message);
  urbi::UList l;
  l.array.reserve(ol.size() - (withThis?1:0));
  urbi::setCurrentContext(urbi::impl::KernelUContextImpl::instance());
  object::check_arg_count(ol.size() - (withThis?1:0), ugc->nbparam);
  bool first = true;
  foreach (const rObject& co, ol)
  {
    if (withThis && first)
    {
      first = false;
      continue;
    }
    urbi::UValue v = uvalue_cast(co);
    l.array << new urbi::UValue(v);
  }
  libport::utime_t start = libport::utime();
  urbi::UValue res;

  bound_context
    << std::make_pair(&ugc->owner ? ugc->owner.__name : "unknown",
                      ugc->name);
  FINALLY(((bool, first)), bound_context.pop_back();GD_INFO_DUMP("Done"));
  // write_and_unfreeze will delete us if true, we handle it if false
  bool* async_abort = new bool;
  *async_abort = false;
  try
  {
    // This if is there to optimize the synchronous case.
    if (ugc->isSynchronous())
      res = ugc->__evalcall(l);
    else
    {
      /* We are going to make a threaded call that will return a result.
       * But while it runs we ourselve can be interrupted by a tag.stop.
       * Furthermore our shared ptr are not thread safe. So:
       * - We give to the thread references to res and exception on our stack.
       * - If we are stopped, those will be invalid because the stack will
       * unwind, so we also give it a bool* async_abort not on stack that we set
       * to true if we are interrupted (if it is false we delete it otherwise
       * the thread does).
       * - To ensure no race condition in the handling of the three above
       * pointers, the thread is going to server().schedule_fast the operation
       * handling passing back the return value, so that it is performed
       * also on this thread.
       */
      libport::Finally f;
      /* write_and_unfreeze will delete tag. It is only passed along by the
       * thread, not copied, not dereferenced, so its safe.
       */
      object::rTag* tag = new object::rTag(new object::Tag);
      ::kernel::runner().state.apply_tag(*tag, &f);
      // Tricky: tag->freeze() yields, but we must freeze tag before calling
      // eval or there will be a race if asyncEval goes too fast and unfreeze
      // before we freeze. So go through backend.
      (*tag)->value_get()->freeze(::kernel::scheduler());
      std::string exception;
      ugc->eval(l, boost::bind(write_and_unfreeze, boost::ref(res),
                               boost::ref(exception),
                               async_abort, tag, _1, _2));
      ::kernel::runner().yield();
      if (!exception.empty())
        throw std::runtime_error("Exception in threaded call: " + exception);
    }
  }
  catch (const sched::exception&)
  {
    *async_abort = true;
    throw;
  }
  catch (const std::exception& e)
  {
    // Message matching the one in SDK Remote so that plug/remote give
    // equivalent messages.
    FRAISE("exception caught while calling %s: %s",
           ugc->getName(), e.what());
  }
  catch (...)
  {
    // Likewise.
    FRAISE("exception caught while calling %s: %s",
           ugc->getName(), "invalid exception");
  }
  delete async_abort;
  start = libport::utime() - start;
  Stats::add(ol.front().get(), message, start);
  return object_cast(res);
}


static
rObject
wrap_event(const object::objects_type& ol,
           urbi::UGenericCallback* ugc,
           const std::string& traceName)
{
  // We were called with arg1 = event, arg2 = payload, arg3 = pattern.
  object::objects_type args = ol[2]->as<object::List>()->value_get();
  if (args.size() == (unsigned int)ugc->nbparam)
    return wrap_ucallback(args, ugc, traceName, false);
  else
    GD_FINFO_DEBUG("C++ at %s not called: wrong arity", traceName);
  return object::void_class;
}


static rObject
uobject_clone(const object::objects_type& l)
{
  rObject proto = l.front();
  return urbi::uobjects::uobject_new(proto);
}

static rObject
uobject_finalize(const object::objects_type& args)
{
  // The object::Object associated with an UObject is being destroyed.
  rObject o = args.front();
  std::string objName =
  o->slot_get_value(SYMBOL(__uobjectName))->as<object::String>()->value_get();
  // Call the finalizer part written in urbiscript.
  o->call(SYMBOL(defaultFinalize));
  libport::BlockLock bl(object_links_lock);
  ObjectLinks::iterator i = object_links.find(objName);
  if (i == object_links.end())
  {
    GD_FERROR("Dying object %s is not referenced", objName);
    return object::void_class;
  }
  // Since we are dead, make us inaccessible.
  i->second.resetPtr();
  if (!i->second.getUObject()) // UObject already removed.
  {
    GD_FINFO_TRACE("uobjects_link removing %s", objName);
    object_links.erase(i);
  }
  else
  {
    delete i->second.getUObject();
  }
  return object::void_class;
}

// Write to an UVar, pretending we are comming from lobby ctx.
static void writeFromContext(const std::string& ctx,
                             const std::string& varName,
                             const urbi::UValue& val)
{
  CHECK_MAINTHREAD();
  if (ctx.substr(0, 2) != "0x")
    throw std::runtime_error("invalid context: " + ctx);
  rLobby rl;
  foreach (object::Lobby* lobby, object::Lobby::instances_get())
    if (lobby->uid() == ctx)
    {
      rl = lobby;
      break;
    }
  if (!rl)
  {
    GD_FWARN("writeFromContext: non existing lobby: %x", ctx);
    return;
  }
  runner::Job& r = kernel::urbiserver->getCurrentRunner();
  rLobby cl = r.state.lobby_get();
  FINALLY(((rLobby, cl))((runner::Job&, r)), r.state.lobby_set(cl));
  r.state.lobby_set(rl);
  object::rUValue ov(new object::UValue());
  ov->put(val, false);
  StringPair p = uname_split(varName);
  rObject o = get_base(p.first);
  if (!o)
    runner::raise_lookup_error(libport::Symbol(varName), object::global_class);
  o->slot_get(Symbol(p.second))->as<object::Slot>()
    ->uobject_set(ov, o, libport::utime());
  ov->invalidate();
}

static object::rObject get_robject(rObject, const std::string& s)
{
  CAPTURE_LANG(lang);
  libport::BlockLock bl(object_links_lock);
  ObjectLinks::iterator i = object_links.find(s);
  if (i != object_links.end())
    return i->second.getRef();
  rObject res = lang->slot_get_value(libport::Symbol(s), false);
  if (res)
    return res;
  // If at some point we remove the 'import uobjects', check here too
  CAPTURE_GLOBAL(uobjects);
  res = uobjects->slot_get_value(libport::Symbol(s), false);
  if (res)
    return res;
  return object::void_class;
}

namespace urbi
{

  /*----------.
  | UObject.  |
  `----------*/

  static
  void
  bounce_update(urbi::UObject* ob, void* me, const std::string& key)
  {
    urbi::setCurrentContext(urbi::impl::KernelUContextImpl::instance());
    libport::utime_t t = libport::utime();
    ob->update();
    Stats::add(me, key, libport::utime()-t);
  }

  UObjectMode running_mode()
  {
    return MODE_PLUGIN;
  }

  namespace impl
  {

    UContextImpl* getPluginContext()
    {
      return KernelUContextImpl::instance();
    }

    KernelUContextImpl::KernelUContextImpl()
    {
      rtpSend = 0;
      rtpSendGrouped = 0;
      instance_ = this;
    }

    void
    KernelUContextImpl::newUObjectClass(baseURBIStarter* s)
    {
      if (server().isAnotherThread())
      {
        server().schedule_fast(
          boost::bind(&KernelUContextImpl::newUObjectClass,
                      this, s));
        return;
      }
      GD_FINFO_TRACE("MakeProto for %s", s->name);
      object::rObject proto = ::urbi::uobjects::uobject_make_proto(s->name);
      GD_FINFO_TRACE("Writing class for %s", s->name);
      where->slot_set_value(libport::Symbol(s->name + "_class"), proto);
      // Make our first instance.
      GD_FINFO_TRACE("Creating instance for %s", s->name);
      rObject o = ::urbi::uobjects::uobject_new(proto, true);
    }

    void
    KernelUContextImpl::newUObjectHubClass(baseURBIStarterHub* s)
    {
      if (server().isAnotherThread())
      {
        server().schedule_fast(
          boost::bind(&KernelUContextImpl::newUObjectHubClass, this, s));
        return;
      }
      s->instanciate(this);
    }

    void
    KernelUContextImpl::send(const char* str)
    {
      if (server().isAnotherThread())
      { // Copy the data
        server().schedule_fast(boost::bind(
                                 (void (UContextImpl::*)(const std::string&))&UContextImpl::send,
                                 this, std::string(str)));
        return;
      }
      kernel::urbiserver->ghost_connection_get().received(str);
    }

    void
    KernelUContextImpl::send(const void* buf, size_t size)
    {
      if (server().isAnotherThread())
      { // Copy the data
        server().schedule_fast(boost::bind(
                                 (void (UContextImpl::*)(const std::string&))&UContextImpl::send,
                                 this, std::string((const char*)buf, size)));
        return;
      }
      // Feed this to the ghostconnection.
      kernel::urbiserver->ghost_connection_get()
        .received(static_cast<const char*>(buf), size);
    }

#define INTERRUPTIBLE                                                   \
    runner::Job& r = ::kernel::runner();                                \
    bool b = r.non_interruptible_get();                                 \
    r.non_interruptible_set(false);                                     \
    FINALLY(((runner::Job&, r))((bool, b)), r.non_interruptible_set(b))


    void KernelUContextImpl::yield() const
    {
      CHECK_MAINTHREAD();
      INTERRUPTIBLE;
      ::kernel::runner().yield();
    }

    void KernelUContextImpl::yield_until(libport::utime_t deadline) const
    {
      CHECK_MAINTHREAD();
      INTERRUPTIBLE;
      ::kernel::runner().yield_until(deadline);
    }

    void KernelUContextImpl::yield_for(libport::utime_t delay) const
    {
      CHECK_MAINTHREAD();
      INTERRUPTIBLE;
      ::kernel::runner().yield_for(delay);
    }

#undef INTERRUPTIBLE

    void KernelUContextImpl::side_effect_free_set(bool)
    {
      CHECK_MAINTHREAD();
    }

    bool KernelUContextImpl::side_effect_free_get() const
    {
      CHECK_MAINTHREAD();
      return false;
    }

    UObjectHub*
    KernelUContextImpl::getUObjectHub(const std::string& n)
    {
      return UContextImpl::getUObjectHub(n);
    }

    UObject*
    KernelUContextImpl::getUObject(const std::string& n)
    {
      /// That check can be done in any thread.
      UObject* res = UContextImpl::getUObject(n);
      if (res)
        return res;
      size_t p = n.find_last_of('.');
      if (p != n.npos)
        res = getUObject(n.substr(p+1, n.npos));
      if (res)
        return res;
      libport::BlockLock bl(object_links_lock);

      ObjectLinks::iterator i = object_links.find(n);
      if (i!= object_links.end())
        return i->second.getUObject();
      GD_FINFO_TRACE("getUObject failed for %s", n);
      return 0;
    }

    void
    KernelUContextImpl::call(const std::string& object,
                             const std::string& method,
                             UAutoValue v1,
                             UAutoValue v2,
                             UAutoValue v3,
                             UAutoValue v4,
                             UAutoValue v5,
                             UAutoValue v6)
    {
      // Bypass writeFromContext to avoid an extra UValue copy.
      if (method == "$uobject_writeFromContext")
      {
        std::string ctx = v1;
        std::string varname = v2;
        urbi::UValue val = v3;
        if (server().isAnotherThread())
          server().schedule_fast(boost::bind(&writeFromContext,
                                             ctx, varname, val));
        else
          writeFromContext(v1, v2, v3);
        return;
      }
      if (server().isAnotherThread())
      {
        server().schedule(SYMBOL("uobjectSchedule"),
          boost::bind(&KernelUContextImpl::call,
            this, object, method,
            v1, v2, v3, v4, v5, v6));
        return;
      }
      GD_FINFO_DUMP("Call %s.%s", object, method);
      rObject b = xget_base(object);
      object::objects_type args;
      args << b;

#define ARG(Nth)                                \
      do {                                      \
        if (v##Nth.type != DATA_VOID)           \
          args << object_cast(v##Nth);          \
      } while (false)

      ARG(1);
      ARG(2);
      ARG(3);
      ARG(4);
      ARG(5);
      ARG(6);
      b->call_with_this(libport::Symbol(method), args);
    }

    static void declare_event_name(std::string name)
    {
      rEvent e = new object::Event(object::Event::proto);
      StringPair p = uname_split(name);
      rObject o = xget_base(p.first,
                            "UEvent creation on non existing object: %s");
      if (!o->local_slot_get(Symbol(p.second)))
        o->slot_set_value(Symbol(p.second), e);
    }

    void
    KernelUContextImpl::declare_event(const UEvent* owner)
    {
      if (server().isAnotherThread())

        server().schedule_fast(boost::bind(&declare_event_name,
                                           owner->get_name()));
      else
        declare_event_name(owner->get_name());
    }

    static
    void
    doEmit(const std::string& object, const object::objects_type& args)
    {
      StringPair p = uname_split(object);
      rObject o = xget_base(p.first)->slot_get_value(libport::Symbol(p.second));
      o->call(SYMBOL(emit), args);
    }

    void
    KernelUContextImpl::emit(const std::string& object,
                             UAutoValue& v1,
                             UAutoValue& v2,
                             UAutoValue& v3,
                             UAutoValue& v4,
                             UAutoValue& v5,
                             UAutoValue& v6,
                             UAutoValue& v7
      )
    {
      if (server().isAnotherThread())
      {
        server().schedule_fast(boost::bind(&KernelUContextImpl::emit,
                                           this, object, v1, v2, v3, v4, v5, v6, v7));
        return;
      }
      object::objects_type args;
      ARG(1);
      ARG(2);
      ARG(3);
      ARG(4);
      ARG(5);
      ARG(6);
      ARG(7);
      doEmit(object, args);
    }

    static void unarmor_and_send(std::string s)
    {
      server().ghost_connection_get().received(s.c_str(), s.length());
    }

    void
    KernelUContextImpl::uobject_unarmorAndSend(const char* str)
    {
      size_t len = strlen(str);
      if (server().isAnotherThread())
      {
        std::string arg;
        if (2 <= len && str[0] == '(')
          arg = std::string(str+1, len-1);
        else
          arg = str;
        server().schedule_fast(boost::bind(&unarmor_and_send, arg));
        return;
      }
      // Feed this to the ghostconnection.
      kernel::UConnection& ghost = kernel::urbiserver->ghost_connection_get();
      if (2 <= len && str[0] == '(')
        ghost.received(str + 1, len - 2);
      else
        ghost.received(str);
    }

    UObjectMode
    KernelUContextImpl::getRunningMode() const
    {
      return MODE_PLUGIN;
    }

    UVarImpl*
    KernelUContextImpl::getVarImpl()
    {
      return new KernelUVarImpl();
    }

    UObjectImpl*
    KernelUContextImpl::getObjectImpl()
    {
      return new KernelUObjectImpl();
    }

    UGenericCallbackImpl*
    KernelUContextImpl::getGenericCallbackImpl()
    {
      return new KernelUGenericCallbackImpl();
    }

    static void set_timer(UTimerCallback* cb, TimerHandle* handle,
                          libport::Semaphore& sem)
    {
      *handle = KernelUContextImpl::instance()->setTimer(cb);
      sem++;
    }

    TimerHandle
    KernelUContextImpl::setTimer(UTimerCallback* cb)
    {
      if (server().isAnotherThread())
      {
        TimerHandle th;
        libport::Semaphore sem;
        GD_INFO_DUMP("SetTimer: async request.");
        server().schedule_fast(boost::bind(&set_timer, cb, &th,
                                           boost::ref(sem)));
        sem--;
        GD_INFO_DUMP("SetTimer: returning.");
        return th;
      }
      rObject me = xget_base(cb->objname);
      rObject f = me->slot_get_value(SYMBOL(setTimer));
      rObject p = new object::Float(cb->period / 1000.0);
      std::string stag = object::String::proto->as<object::String>()->fresh();
      rObject tag = new object::String(stag);
      rObject call = MAKE_VOIDCALL(cb, urbi::UTimerCallback, call);
      object::objects_type args;
      args << me << p << call << tag;
      eval::call_apply(::kernel::runner(), f, SYMBOL(setTimer), args);
      GD_FINFO_DUMP("SetTimer on %s: %s", cb, stag);
      return TimerHandle(new std::string(stag));
    }

    boost::asio::io_service&
    KernelUContextImpl::getIoService()
    {
      return ::kernel::urbiserver->get_io_service();
    }

    bool
    KernelUObjectImpl::removeTimer(TimerHandle h)
    {
      GD_FINFO_DUMP("RemoveTimer on %s %s", h, owner_->__name);
      if (!h)
        return false;
      if (server().isAnotherThread())
      { // Return value is not that important, so lie for the sake of making
        // an asynchronous call.
        server().schedule_fast(
          boost::bind(&KernelUObjectImpl::removeTimer, this, h));
        return true;
      }
      rObject me = xget_base(owner_->__name);
      me->call(SYMBOL(removeTimer), new object::String(*h));
      h.reset();
      return true;
    }

    void KernelUContextImpl::registerHub(UObjectHub*)
    {
    }

    void KernelUContextImpl::removeHub(UObjectHub*)
    {
    }

    void KernelUContextImpl::setHubUpdate(UObjectHub* hub, libport::ufloat period)
    {
      // Call Urbi-side setHubUpdate, passing an rPrimitive wrapping
      // the 'update' call.
      if (server().isAnotherThread())
      {
        server().schedule_fast(
          boost::bind(&KernelUContextImpl::setHubUpdate, this, hub, period));
        return;
      }
      CAPTURE_GLOBAL(UObject);
      rObject f = UObject->slot_get_value(SYMBOL(setHubUpdate));
      object::objects_type args;
      args << UObject
           << rObject(new object::String(hub->get_name()))
           << new object::Float(period / 1000.0)
           << MAKE_VOIDCALL(hub, urbi::UObjectHub, update);
      eval::call_apply(::kernel::runner(), f, SYMBOL(setHubUpdate), args);
    }

    std::pair<int, int>
    KernelUContextImpl::kernelVersion() const
    {
      //FIXME: fetch from some other place
      return std::make_pair(2, 0);
    }

    void
    KernelUContextImpl::instanciated(UObject*)
    {
    }

    void
    KernelUContextImpl::lock()
    {
    }

    void
    KernelUContextImpl::unlock()
    {
    }

    class KernelBarrier: public Barrier
    {
    private:
      class Storage
      {
      public:
        Storage() : tag(0) {}
        ~Storage()
        {
          if (server().isAnotherThread())
            server().schedule_fast(boost::bind(&KernelBarrier::Storage::rtagDelete, tag));
          else
            rtagDelete(tag);
        }
        static void rtagDelete(object::rTag* t)
        {
          delete t;
        }
        boost::promise<int> prom;
        object::rTag* tag;
      };
      typedef boost::shared_ptr<Storage> StoragePtr;
    public:
      KernelBarrier()
      : storage(new Storage)
      {
        if (server().isAnotherThread())
        {
          boost::promise<int> prom;
          server().schedule_fast(boost::bind(&KernelBarrier::doInitialize, this, &prom));
          prom.get_future().wait();
        }
        else
          storage->tag = new object::rTag(new object::Tag);
      }
      ~KernelBarrier()
      {
      }
      void doInitialize(boost::promise<int>* prom)
      {
        storage->tag = new object::rTag(new object::Tag);
        prom->set_value(0);
      }
      void wait()
      {
        StoragePtr s = storage;
        if (server().isAnotherThread())
        {
          s->prom.get_future().wait();
        }
        else
        {
          libport::Finally f;
          ::kernel::runner().state.apply_tag(*s->tag, &f);
          (*s->tag)->freeze();
        }
      }
      static void doRelease(StoragePtr s)
      {
        s->prom.set_value(0);
        (*s->tag)->unfreeze();
      }
      void release()
      {
        GD_FINFO_TRACE("Barrier::release %s", this);
        if (server().isAnotherThread())
          server().schedule_fast(boost::bind(&KernelBarrier::doRelease, storage));
        else
        {
          doRelease(storage);
        }
      }
      StoragePtr storage;
    };

    Barrier*
    KernelUContextImpl::barrier()
    {
      return new KernelBarrier();
    }

    KernelUObjectImpl::~KernelUObjectImpl()
    {
      foreach(UGenericCallback* c, callbacks_)
        delete c;
    }

    void
    KernelUObjectImpl::initialize(UObject* owner)
    {
      GD_FINFO_TRACE("Initializing %s: %s", owner, owner->__name);
      if (server().isAnotherThread())
      {
        static int uid = 1;
        /* We must name the uobject now, in case the caller synchronously
        *  tries to call stuff, which will push an async job with the object name.
        *  Async jobs will be ordered properly so initialize will be first.
        */
        if (owner->__name.empty())
          owner->__name = "__cxx_" + string_cast(++uid);
        server().schedule_fast(
          boost::bind(&KernelUObjectImpl::initialize, this,
                      owner));
        return;
      }
      owner_ = owner;
      bool fromcxx = owner_->__name.empty()
       || (owner_->__name.size() >= 3 &&owner_->__name.substr(0, 5) == "__cxx");

      rObject r; // Must live until we call ref() on object_links's entry.
      if (fromcxx)
      {
        CAPTURE_GLOBAL(UObject);
        r = ::urbi::uobjects::uobject_new(UObject);
        if (owner_->__name.empty())
        {
          r = ::urbi::uobjects::uobject_new(UObject);
          owner_->__name =
            r->call(SYMBOL(__uobjectName))->as<object::String>()->value_get();
        }
        else
        {
          r = ::urbi::uobjects::uobject_new(UObject, false, true, owner_->__name);
        }
      }
      owner_->ctx_->registerObject(owner);
      rObject o = get_base(owner->__name);
      if (!o)
      {
        // Instantiation occurred through ucontext::bind.
        o = ::urbi::uobjects::uobject_make_proto(owner->__name);
        where->slot_set_value(Symbol(owner->__name), o);
      }
      libport::BlockLock bl(object_links_lock);
      ObjectLinks::iterator i = object_links.find(bareName());
      if (i == object_links.end())
      { // uobject_new should have created the link
        GD_ERROR("Object link not found");
        abort();
      }
      i->second.setUObject(owner);
      if (fromcxx)
      {
        i->second.ref();
        assert(get_base(bareName()));
      }
      o->slot_set_value(SYMBOL(lobby), kernel::runner().state.lobby_get());
    }

    void
    KernelUObjectImpl::clean()
    {
      // The C++ UObject is getting destroyed.
      if (server().isAnotherThread())
      {
        server().schedule_fast(
          boost::bind(&KernelUObjectImpl::clean, this));
        return;
      }
      libport::BlockLock bl(object_links_lock);
      ObjectLinks::iterator i = object_links.find(bareName());
      if (i == object_links.end())
      {
        GD_FERROR("Dying UObject %s is not referenced", owner_->__name);
        return;
      }
      i->second.resetUObject();
      i->second.deref();
      if (!i->second.getRef()) // rObject already removed.
      {
        GD_FINFO_TRACE("uobjects_link removing %s", bareName());
        object_links.erase(i);
      }
      owner_->ctx_->objects.erase(owner_->__name);
    }

    std::string
    KernelUObjectImpl::bareName() const
    {
      std::string res = owner_->__name;
      size_t pos = res.find_last_of('.');
      if (pos != res.npos)
        res = res.substr(pos+1, res.npos);
      return res;
    }

    static
    std::string
    me_id(rObject me)
    {
      object::objects_type args;
      args << me;
      return
        eval::call_apply(
          ::kernel::runner(),
          me->slot_get_value(SYMBOL(DOLLAR_id)), SYMBOL(DOLLAR_id), args)
        ->as<object::String>()
        ->value_get();
    }

    static
    std::string
    trace_name(rObject me)
    {
      if (me->slot_has(SYMBOL(compactName)))
        return
          me
          ->slot_get_value(SYMBOL(compactName))
          ->as<object::String>()
          ->value_get();
      else if (me->slot_has(SYMBOL(__uobjectName)))
        return
          me
          ->slot_get_value(SYMBOL(__uobjectName))
          ->as<object::String>()
          ->value_get();
      else
        return me_id(me);
    }

    void
    KernelUObjectImpl::setUpdate(libport::ufloat period)
    {
      if (server().isAnotherThread())
      {
        server().schedule_fast(
          boost::bind(&KernelUObjectImpl::setUpdate, this,
                      period));
        return;
      }
      rObject me = xget_base(owner_->__name);
      rObject f = me->slot_get_value(SYMBOL(setUpdate));
      me->slot_update
        (SYMBOL(update),
         object::primitive
         (boost::function1<void, rObject>(
           boost::bind(&bounce_update,
                       owner_, me.get(), me_id(me) + " update"))));
      object::objects_type args;
      args << me
           << new object::Float(period / 1000.0);
      eval::call_apply(::kernel::runner(), f, SYMBOL(setUpdate), args);
    }

    void
    KernelUVarImpl::async(boost::function0<void> f)
    {
      f();
      libport::BlockLock bl(asyncLock_);
      --pending_;
    }

    void
    KernelUVarImpl::schedule(libport::Symbol, boost::function0<void> f,
                             bool sync) const
    {
      GD_INFO_DUMP("Fast async schedule from UVar");
      KernelUVarImpl* self = const_cast<KernelUVarImpl*>(this);
      {
        libport::BlockLock bl(self->asyncLock_);
        ++self->pending_;
      }
      if (sync)
      {
        libport::Semaphore sem;
        server().schedule_fast(
          boost::bind(&call_and_unlock,
                      boost::function0<void>(boost::bind(&KernelUVarImpl::async, self,
                                                         f)),
                      boost::ref(sem)));
        sem--;
      }
      else
        server().schedule_fast(boost::bind(&KernelUVarImpl::async, self, f));
    }

    void
    KernelUVarImpl::unnotify()
    {
      if (server().isAnotherThread())
      {
        schedule(SYMBOL(UObject),boost::bind(&KernelUVarImpl::unnotify, this));
        return;
      }
      GD_FINFO_TRACE("Unnotify on %s: %s callbacks ", owner_->get_name(),
                     callbacks_.size());
      foreach (KernelUGenericCallbackImpl* v, callbacks_)
      {
        GD_FINFO_TRACE("Unnotify processing callback %s", v);
        /* We are stored either:
         * -as a oget/oset of slot_ if there is no inPort_
         * -as a oget/oset of inPort_, and as a UConnection otherwise
         */
        object::rSlot r = v->inPort_?v->inPort_:slot_;
        if (r)
        {
          if (v->owner_->type == "var") // We are the osetter
            r->oset_set(0);
          else if (v->owner_->type == "varaccess") // ogetter
            r->oget_set(0);
        }
        if (v->connection_)
        {
          v->connection_->CxxObject::call(SYMBOL(disconnect));
          GD_FINFO_TRACE("Killing connection associated with UGC %s", v);
        }
      }
    }

    KernelUVarImpl::KernelUVarImpl()
      : pending_(0)
      , bypassMode_(false)
    {
    }

    void
    KernelUVarImpl::initialize(UVar* owner)
    {
      initialize(owner, 0);
    }

    void
    KernelUVarImpl::initialize(UVar* owner, rSlot slot)
    {
      owner_ = owner; // set at least that even from an other thread.
      splitName_ = uname_split(owner->get_name());
      if (slot)
      {
        slot_ = slot;
        bypassMode_ = false;
        return;
      }
      GD_FINFO_TRACE("KernelUVarImpl::init %s", owner->get_name());
      if (server().isAnotherThread())
      {
        schedule(SYMBOL(UObject),boost::bind(&KernelUVarImpl::initialize, this,
                                             owner));
        return;
      }
      // Protect against multiple parallel creation of the same UVar.
      runner::Job& runner = ::kernel::runner();
      bool prevState = runner.non_interruptible_get();
      FINALLY(
        ((bool, prevState))
        ((runner::Job&, runner))
        ((UVar*, owner)),
        runner.non_interruptible_set(prevState););
      runner.non_interruptible_set(true);
      owner_ = owner;
      owner_->owned = false;
      bypassMode_ = false;
      if (splitName_.first == "@")
      { // UVar address was passed directly.
        std::stringstream ss;
        ss.str(splitName_.second);
        void* addr;
        ss >> addr;
        object::Slot* v = (object::Slot*)addr;
        slot_ = v;
        GD_FINFO_DUMP("UVar created from address %s", addr);
        return;
      }
      rObject o = get_base(splitName_.first);
      if (!o)
        FRAISE("UVar creation on non existing object: %s", owner->get_name());
      Symbol varName(splitName_.second);
      rObject v = o->local_slot_get(varName);
      if (v)
      {
        slot_ = v->as<object::Slot>();
        if (!slot_)  //trigger transparent slot creations
          slot_ = o->getSlot(varName)->as<object::Slot>();
      }
      if (!slot_)
      {
        CAPTURE_GLOBAL(Slot);
        slot_ = Slot->call(SYMBOL(new),
                                 o, new object::String(splitName_.second))
        ->as<object::Slot>();
      }
      traceOperation(owner, SYMBOL(traceBind));
      GD_FINFO_DUMP("Uvar %s creating new object %s.",
                    owner_->get_name(),
                    slot_.get());
    }

    void KernelUVarImpl::clean()
    {
      GD_FINFO_TRACE("KernelUVarImpl::clean %s", owner_->get_name());
      // Called by the owner UVar's destructor.
      // Block destruction until all pending operations are finished
      if (server().isAnotherThread())
        while(pending_)
          usleep(100);
      else
        while(pending_)
          ::kernel::runner().yield();
      //noop
    }

    void KernelUVarImpl::setOwned()
    {
      if (server().isAnotherThread())
      {
        schedule(SYMBOL(UObject),boost::bind(&KernelUVarImpl::setOwned, this));
        return;
      }

      owner_->owned = true;
      // Write 1 to the Urbi-side uvar owned slot.
      slot_->split_set(true);
    }

    void KernelUVarImpl::sync()
    {
      //noop
    }

    void KernelUVarImpl::request()
    {
      //noop
    }

    void KernelUVarImpl::keepSynchronized()
    {
      //noop
    }

    void KernelUVarImpl::set(const UValue& v)
    {
      if (server().isAnotherThread())
      {
        GD_FINFO_DUMP("async set %s to %s", owner_->get_name(), v);
        UValue vv(v);
        if (vv.type == DATA_BINARY)
          vv.binary->temporary_ = true;
        schedule(SYMBOL(UObject), boost::bind(&KernelUVarImpl::set, this, vv));
        return;
      }
      GD_FINFO_DUMP("set %s to %s", owner_->get_name(), v);
      traceOperation(owner_, SYMBOL(traceSet));
      object::rUValue ov(new object::UValue());
      ov->put(v, bypassMode_);
      if (owner_->owned)
        slot_->set_output_value(ov);
      else
      {
        object::Object* sender = 0;
        // We need to find out the owner UObject to pass the correct sender.
        sender = get_base(splitName_.first);
        slot_->set(ov, sender, libport::utime());
      }
      ov->invalidate();
    }

    void KernelUVarImpl::async_get(UValue** v) const
    {
      *v = &const_cast<UValue&>(get());
    }

    const UValue& KernelUVarImpl::get() const
    {
      if (server().isAnotherThread())
      {
        urbi::UValue* v;
        schedule(SYMBOL(UObject),
                 boost::bind(&KernelUVarImpl::async_get, this, &v),
                 true);
        return *v;
      }
      aver(slot_);
      traceOperation(owner_, SYMBOL(traceGet));
      try
      {
        object::Object* sender;
        sender = get_base(splitName_.first);
        rObject o = (owner_->owned
                     ? slot_->value_get()
                     : slot_->value(sender, true));
        aver(o);
        if (object::rUValue bv = o->as<object::UValue>())
          return bv->value_get();
        else
        {
          cache_ = ::uvalue_cast(o);
          return cache_;
        }
      }
      catch (object::UrbiException& e)
      {
        FRAISE("invalid read of void UVar '%s': %s", owner_->get_name(),
               e.what());
      }
    }

    libport::ufloat& KernelUVarImpl::in()
    {
      throw std::runtime_error("in() is not implemented");
    }

    libport::ufloat& KernelUVarImpl::out()
    {
      throw std::runtime_error("out() is not implemented");
    }

    UDataType
    KernelUVarImpl::type() const
    {
      // Do not bounce to get().type which might need to copy the value.
      if (server().isAnotherThread())
      {
        urbi::UValue* v;
        schedule(SYMBOL(UObject),
                 boost::bind(&KernelUVarImpl::async_get, this,
                             &v), true);
        return v->type;
      }
      object::Object* sender;
      sender = get_base(splitName_.first);
      rObject o = (owner_->owned
                     ? slot_->value_get()
                     : slot_->value(sender, true));
      return ::uvalue_type(o);
    }

    void
    KernelUVarImpl::async_get_prop(urbi::UValue&v,
                                   UProperty prop)
    {
      v = getProp(prop);
    }

    UValue KernelUVarImpl::getProp(UProperty prop)
    {
      if (server().isAnotherThread())
      {
        urbi::UValue v;
        schedule(SYMBOL(UObject),
                 boost::bind(&KernelUVarImpl::async_get_prop, this,
                             boost::ref(v), prop), true);
        return v;
      }
      aver(slot_);
      if (prop == urbi::PROP_RANGEMIN)
        return UValue(slot_->rangemin_get());
      else if (prop == urbi::PROP_RANGEMAX)
        return UValue(slot_->rangemax_get());
      rObject o = xget_base(splitName_.first);
      return ::uvalue_cast(o->call(SYMBOL(getProperty),
                                   new object::String(splitName_.second),
                                   new object::String(UPropertyNames[prop])));
    }

    void KernelUVarImpl::setProp(UProperty prop, const UValue& v)
    {
      if (server().isAnotherThread())
      { // Copy the data
        schedule(SYMBOL(UObject),boost::bind(&KernelUVarImpl::setProp,
                                             this, prop, v));
        return;
      }
      rObject o = xget_base(splitName_.first);
      o->call(SYMBOL(setProperty),
              new object::String(splitName_.second),
              new object::String(UPropertyNames[prop]),
              object_cast(v));
    }

    bool KernelUVarImpl::setBypass(bool enable)
    {
      bypassMode_ = enable;
      return true;
    }

    void KernelUVarImpl::useRTP(bool enable)
    {
      if (server().isAnotherThread())
      { // Copy the data
        schedule(SYMBOL(UObject),boost::bind(&KernelUVarImpl::useRTP,
                                             this, enable));
        return;
      }
      slot_->slot_set_value(SYMBOL(rtp),  object::to_urbi(enable));
    }

    void KernelUVarImpl::setInputPort(bool enable)
    {
      if (server().isAnotherThread())
      { // Copy the data
        schedule(SYMBOL(UObject),boost::bind(&KernelUVarImpl::setInputPort,
                                             this, enable));
        return;
      }
      runner::Job& r = ::kernel::urbiserver->getCurrentRunner();
      bool last_rdm = r.state.redefinition_mode_get();
      FINALLY( ((bool, last_rdm))((runner::Job&, r)),
               r.state.redefinition_mode_set(last_rdm));
      r.state.redefinition_mode_set(true);
      if (enable)
        slot_->slot_set_value(SYMBOL(inputPort), object::to_urbi(true));
      else
        slot_->slot_remove(SYMBOL(inputPort));
    }

    time_t
    KernelUVarImpl::timestamp() const
    {
      if (!slot_)
        throw std::runtime_error("UVar without cache");
      return time_t(slot_->timestamp_get());
    }

    void
    KernelUGenericCallbackImpl::initialize(UGenericCallback* owner, bool owned)
    {
      owner_ = owner;
      owned_ = owned;
      registered_ = false;
    }

    void
    KernelUGenericCallbackImpl::initialize(UGenericCallback* owner)
    {
      initialize(owner, false);
    }

    void
    KernelUGenericCallbackImpl::registerCallback()
    {
      if (server().isAnotherThread())
      {
        server()
          .schedule_fast
          (boost::bind(&KernelUGenericCallbackImpl::registerCallback,
                       this));
        return;
      }
      if (registered_)
      {
        GD_FINFO_DUMP("UGenericcallback on %s already registered",
                      owner_->name);
        return;
      }
      registered_ = true;
      StringPair p = uname_split(owner_->name);
      std::string method = p.second;
      GD_FPUSH_DUMP("UGC %s, %s, %s, %s", owner_->type,p.first, method, owned_);
      // UObject owning the variable/event to monitor
      rObject me = get_base(p.first); //objname?
      std::string traceName;
      if (me)
        traceName = trace_name(me);
      if (owner_->type == "function")
      {
        if (!me)
        {
          GD_FERROR("No such UObject %s" , p.first);
          throw std::runtime_error("No such UObject " + p.first);
        }
        traceName += "." + p.second;
        me->slot_set_value(libport::Symbol(method), new object::Primitive(
                       boost::function1<rObject, const objects_type&>
                       (boost::bind(&wrap_ucallback, _1, owner_, traceName,
                                    true))));
        me->slot_get(libport::Symbol(method))->slot_set(SYMBOL(watchIncompatible),
          urbi::object::to_urbi(true));
      }
      if (owner_->type == "event")
      {
        if (!me)
          throw std::runtime_error("No such UObject " + p.first);
        UEvent e = UEvent(p.first, p.second); // force creation of the event
        rEvent event =
          me->slot_get_value(libport::Symbol(method))->as<object::Event>();
        event->onEvent(0, new object::Primitive(
                         boost::function1<rObject, const objects_type&>
                         (boost::bind(&wrap_event, _1, owner_, traceName))));
      }
      if (owner_->type == "var" || owner_->type == "varaccess")
      {
        // NotifyChange or NotifyAccess callback
        GD_FINFO_DUMP("UGC %s using backend UConnection.", this);
        //owner_->owner is the UObject that sets the callback: target
        // Compute tracing name
        // traceName is objName__obj__var
        traceName += "." + p.second + " --> ";
        if (&owner_->owner)
          if (rObject you = get_base(owner_->owner.__name))
            traceName += trace_name(you);

        // Source UVar
        object::rSlot var;
        if (owner_->target)
          var = ((KernelUVarImpl*)owner_->target->impl_)->slot_;
        else if (me)
          var = me->slot_get(Symbol(method))->as<object::Slot>();
        else if (p.first == "@")
        {
          void* addr;
          std::stringstream ss;
          ss.str(p.second);
          ss >> addr;
          var = (object::Slot*)addr;
        }
        aver(var);
        rObject source = xget_base(owner_->objname);
        GD_FINFO_TRACE("UGC same source: %s (%s==%s)", source == me,
                       owner_->objname, p.first);
        if (source != me)
        {
          /* We do not own the slot to hook, so create a new Slot in our
          * uobject, hook that instead, and setup a UConnection between them.
          */
          if (owner_->type == "var" && !owned_)
          {
            GD_FINFO_TRACE("UGC %s using UConnection backend.", this);
            // Use the new mechanism: create an input port and use it
            std::string ipname =
              boost::replace_all_copy(owner_->name, ".", "_");
            GD_FINFO_TRACE("UGC creating input port %s.%s",
                           owner_->objname, ipname);
            InputPort ip(owner_->objname, ipname);
            inPort_ = xget_base(owner_->objname)->slot_get(Symbol(ipname))
              ->as<object::Slot>();
            std::string inportName_ = owner_->objname + "." + ipname;
            // And link it with the source using a uconnection.
            object::rUConnection c
              = object::UConnection::proto
              ->Object::call(SYMBOL(new), var, inPort_)
              ->as<object::UConnection>();
            // Retarget callback on our own input port we just created
            var = inPort_;
            connection_ = c;
            GD_FINFO_TRACE("UGC %s using UConnection backend %s.", this,
                           connection_.get());
          }
          else
            GD_FWARN("invalid usage of foreign UVar: notifyaccess or USensor"
                     "notifychange on %s from %s",
                     owner_->name, owner_->objname);
        }
        callback_ = new object::Primitive(
          boost::function1<rObject, const objects_type&>
          (boost::bind(&wrap_ucallback_notify, _1, owner_,
                       traceName, owner_->type == "var")));
        callback_->slot_set_value
          (SYMBOL(target),
           new object::String(&owner_->owner ? owner_->owner.__name:"unknown"));

        if (owner_->type == "varaccess")
          var->oget_set(callback_);
        else if (owned_)
          var->oset_set(callback_);
        else
          var->oset_set(callback_);
        if (!var->hasLocalSlot(SYMBOL(watchIncompatible)))
          var->slot_set(SYMBOL(watchIncompatible),
            urbi::object::to_urbi(true));
        GD_FINFO_DUMP("Registered on %s(%s)", owner_->name,
                      var.get());
        if (owner_->target)
        {
          KernelUVarImpl* vimpl
            = static_cast<KernelUVarImpl*>(owner_->target->impl_);
          GD_FINFO_DUMP("Registering callback to KernelUVarImpl %s", vimpl);
          vimpl->callbacks_ << this;
        }
      }
      if (&owner_->owner)
        static_cast<KernelUObjectImpl*>(owner_->owner.impl_get())->callbacks_
          .push_back(owner_);
    }

    void
    KernelUGenericCallbackImpl::clear()
    {
    }
  }

  int kernelMajor()
  {
    const std::string& kv = kernelVersion();
    return boost::lexical_cast<int>(kv.substr(0, kv.find_first_of('.')));
  }

  int kernelMinor()
  {
    const std::string& kv = kernelVersion();
    return boost::lexical_cast<int>(kv.substr(kv.find_first_of('.')+1,
                                              kv.npos));
  }

  const std::string& kernelVersion()
  {
    return object::system_class->call(SYMBOL(version))->as<object::String>()
      ->value_get();
  }

  namespace uobjects
  {

    /** Find an UObject from its name passed to us by the user using one of
     * the UObject API calls.
     * We look for it in our mapping tables, in case the user used
     * 'name_get()' on an UObject, and also for Global objects
     * (which includes GSRAPI short names).
     */
    rObject
    get_base(const std::string& objname)
    {
      libport::BlockLock bl(object_links_lock);
      ObjectLinks::iterator i = object_links.find(objname);
      if (i != object_links.end())
      {
        rObject res(i->second.getRef());
        if (!res)
          GD_FWARN("get_base: entry present but empty for %s", objname);
        return res;
      }
      size_t p = objname.find_last_of('.');
      // Try with last component.
      if (p != objname.npos)
      {
        rObject res = get_base(objname.substr(p+1, objname.npos));
        if (res)
          return res;
      }
      GD_FINFO_TRACE("get_base falling back to getSlot for %s", objname);
      CAPTURE_LANG(lang);
      rObject res =
        lang->slot_get_value(libport::Symbol(objname),
                                             false);
      if (!res)
      {
        res = where->slot_get_value(libport::Symbol(objname), false);
      }
      GD_FINFO_TRACE("get_base: %s", res);
      return res;
    }

    /*! Initialize plugin UObjects.
      \param args object in which the instances will be stored.
    */
    rObject uobject_initialize(const objects_type& args)
    {
      CAPTURE_GLOBAL(Object);
      urbi::setCurrentContext(new urbi::impl::KernelUContextImpl());
      where = args.front();
      where->slot_set_value(SYMBOL(setTrace), object::primitive(&setTrace));
      uobjects_reload();
      where->slot_set_value(SYMBOL(getStats),
                            object::primitive(&Stats::get));
      where->slot_set_value(SYMBOL(clearStats),
                            object::primitive(&Stats::clear));
      where->slot_set_value(SYMBOL(enableStats),
                            object::primitive(&Stats::enable));
      where->slot_set_value(SYMBOL(allUObjects),
                            object::primitive(&all_uobjects));
      where->slot_set_value(SYMBOL(findUObject),
                            object::primitive(&get_robject));
      Object->slot_set_value(SYMBOL(uvalueDeserialize), primitive(&uvalue_deserialize));

      where->bind(SYMBOL(searchPath),    &uobject_uobjectsPath,
                  &uobject_uobjectsPathSet);
      uobjects_path
        .push_back(libport::xgetenv("URBI_UOBJECT_PATH", APPLE_LINUX_WINDOWS(".:", ".:", ".;")),
                   kernel::urbiserver->urbi_root_get().uobjects_path(),
                   APPLE_LINUX_WINDOWS(":", ":", ";"));
      return object::void_class;
    }

    /*! Create the prototype for an UObject class.
     */
    rObject
    uobject_make_proto(const std::string& name)
    {
      CAPTURE_GLOBAL(UObject);
      rObject res =
        UObject
        ->call(SYMBOL(clone));
      res->call(SYMBOL(uobjectInit));
      res->call(SYMBOL(init));
      res->slot_set_value(SYMBOL(finalize), new object::Primitive(&uobject_finalize));
      res->slot_set_value(SYMBOL(__uobject_cname), new object::String(name));
      res->slot_set_value(SYMBOL(__uobject_base), res);
      res->slot_set_value(SYMBOL(clone), new object::Primitive(&uobject_clone));
      res->slot_set_value(SYMBOL(periodicCall), object::primitive(&periodic_call));
      return res;
    }

    /*! Instanciate a new prototype inheriting from a UObject.
      A new instance of UObject is created
      \param proto proto object, created by uobject_make_proto() or uobject_new()
      \param forceName force the reported C++ name to be the class name
      \param instanciate true if the UObject should be instanciated, false if it
      \param name if not empty, force this name
      already is.
    */
    rObject
    uobject_new(rObject proto, bool forceName, bool instanciate, const std::string& fname)
    {
      rObject res = new object::Finalizable(proto->as<object::Finalizable>());

      // Get UObject name.
      rObject rcName = proto->slot_get_value(SYMBOL(__uobject_cname));
      const std::string& cname = rcName.cast<object::String>()->value_get();

      // Get the name we will pass to uobject.
      std::string name;
      if (forceName)
      {
        res->slot_set_value(SYMBOL(type), rcName);
        name = cname;
        where->slot_set_value(libport::Symbol(name), res);
      }
      else if (!fname.empty())
        name = fname;
      else
      {
        // boost::lexical_cast does not work on the way back, so dont
        // use it here.
        std::stringstream ss;
        ss << "uob_" << res.get();
        name = ss.str();
      }
      res->slot_set_value(SYMBOL(__uobjectName), object::to_urbi(name));
      {
        libport::BlockLock bl(object_links_lock);
        ObjectLinks::iterator i = object_links.find(name);
        if (i == object_links.end())
        {
          GD_FINFO_TRACE("pushing %s %s", name, res);
          object_links[name] = urbi::impl::Link(res.get(), false, 0);
          assert(get_base(name));
        }
        else
          i->second.set(res, false);
      }
      res->call(SYMBOL(uobjectInit));
      // Instanciate UObject.
      if (instanciate)
      {
        foreach (urbi::baseURBIStarter* i, urbi::baseURBIStarter::list())
        {
          if (i->name == cname)
          {
            bound_context.push_back(std::make_pair(name, name + ".new"));
            FINALLY(((std::string, name)), bound_context.pop_back());
            i->instanciate(urbi::impl::KernelUContextImpl::instance(),
                           forceName?name:("getUObject." + name));
            return res;
          }
        }
      }
      return res;
    }

    std::string
    processSerializedMessage(int msgType,
                             libport::serialize::BinaryISerializer& ia)
    {
      switch(msgType)
      {
      case urbi::UEM_ASSIGNVALUE:
      {
        std::string name;
        urbi::UValue val;
        libport::utime_t time;
        unsigned int tlow, thi;
        ia >> name >> val >> tlow >> thi;
        time = tlow + ((libport::utime_t)thi << 32);
        StringPair p = uname_split(name);
        GD_FINFO_TRACE("UEM_ASSIGNVALUE %s %s", name, val);
        object::rUValue ov(new object::UValue(val));
        rObject o = xget_base(p.first);
        if (!o)
          GD_FWARN("Object '%s' requested by remote not found", p.first);
        else
        {
          rSlot s =  o->getSlot(Symbol(p.second))->as<object::Slot>();
          if (!s)
            GD_FWARN("Slot '%s' from '%s' is not a slot", p.second, p.first);
          else
            s->uobject_set(ov, o, time);
        }
      }
      break;
      case urbi::UEM_EMITEVENT:
      {
        std::string name;
        int count;
        ia >> name >> count;
        object::objects_type args;
        for (int i=0; i<count; ++i)
        {
          UValue v;
          ia >> v;
          args.push_back(object_cast(v));
        }
        impl::doEmit(name, args);
      }
      break;
      case urbi::UEM_REPLY:
      {
        std::string id;
        UValue val;
        ia >> id >> val;
        CAPTURE_GLOBAL(UObject);
        UObject->call(SYMBOL(funCall), object::to_urbi(id),
                 object_cast(val));
      }
      break;
      case urbi::UEM_EVAL:
      {
        std::string code;
        ia >> code;
        return code;
      }
      break;
      default:
        GD_FWARN("Invalid serialized message number %s", (int)msgType);
        break;
      }
      return "";
    }
  }
}
