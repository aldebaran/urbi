/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <cstdarg>

#include <boost/assign.hpp>
#include <libport/bind.hh>
#include <libport/lexical-cast.hh>

#ifdef ENABLE_DEBUG_TRACES_UOBJECT
# define ENABLE_DEBUG_TRACES
#endif

#include <libport/echo.hh>
#include <libport/foreach.hh>
#include <libport/hash.hh>
#include <libport/lexical-cast.hh>
#include <libport/synchronizer.hh>

#include <kernel/config.h>

#include <kernel/uconnection.hh>
#include <kernel/userver.hh>
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
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <urbi/object/tag.hh>
#include <object/symbols.hh>
#include <object/system.hh>
#include <object/urbi-exception.hh>
#include <object/uvalue.hh>
#include <object/uvar.hh>
#include <urbi/runner/raise.hh>
#include <runner/runner.hh>

#include <urbi/uobject.hh>

#include <urbi/ucontext-factory.hh>


// Make it more readable.
using namespace boost::assign;
using object::rEvent;
using object::rObject;
using object::rLobby;
using object::objects_type;
using object::void_class;
using object::nil_class;
using libport::Symbol;

// Using RAII, cannot be a function
#define LOCK_KERNEL \
  libport::Synchronizer::SynchroPoint \
  urbi_synchro_point_(kernel::urbiserver->synchronizer_get(), true)

// Declare our UObject implementation
namespace urbi
{
  namespace impl
  {
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
                         UAutoValue v6 = UAutoValue(),
                         UAutoValue v7 = UAutoValue(),
                         UAutoValue v8 = UAutoValue());
      virtual void declare_event(const UEvent* owner);
      virtual void emit(const std::string& object,
                        UAutoValue& v1,
                        UAutoValue& v2,
                        UAutoValue& v3,
                        UAutoValue& v4,
                        UAutoValue& v5,
                        UAutoValue& v6,
                        UAutoValue& v7,
                        UAutoValue& v8
                        );
      virtual UObjectMode getRunningMode() const;
      virtual std::pair<int, int> kernelVersion() const;
      virtual void yield() const;
      virtual void yield_until(libport::utime_t deadline) const;
      virtual void yield_for(libport::utime_t delay) const;
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
      virtual void instanciated(UObject* uob);
      virtual void lock();
      virtual void unlock();
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
    private:
      UObject* owner_;
      friend class KernelUGenericCallbackImpl;
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
    private:
      UVar* owner_;
      bool bypassMode_;
      mutable UValue cache_;
      typedef std::vector<KernelUGenericCallbackImpl*> callbacks_type;
      callbacks_type callbacks_;
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
      rObject callback_;
      friend class KernelUObjectImpl;
      friend class KernelUVarImpl;
    };
  }
}

typedef std::pair<std::string, std::string> StringPair;
// Where to store uobjects
static rObject where;
typedef boost::unordered_map<std::string, urbi::UObject*> uobject_to_robject_type;
static uobject_to_robject_type uobject_to_robject;
static std::set<void*> initialized;
static bool trace_uvars = 0;

static rObject get_base(const std::string& objname);
static void writeFromContext(const std::string& ctx, const std::string& varName,
                             const urbi::UValue& val);
/// Split a string of the form "a.b" in two
static StringPair split_name(const std::string& name)
{
  size_t p = name.find_last_of(".");
  std::string oname = name.substr(0, p);
  std::string slot = name.substr(p + 1, name.npos);
  return StringPair(oname, slot);
}

static void setTrace(rObject, bool v)
{
  trace_uvars = v;
}
// Object,method names of last call
static std::vector<std::pair<std::string, std::string> > bound_context;

#define MAKE_VOIDCALL(ptr, cls, meth)                   \
  object::make_primitive(                               \
    boost::function1<void, rObject>(                    \
      boost::bind(&cls::meth, ptr)))

#ifdef SCHED_CORO_OSTHREAD
# define CHECK_MAINTHREAD()
#else
# define CHECK_MAINTHREAD()				\
  passert(!::kernel::urbiserver->isAnotherThread(),     \
          "The UObject API isn't thread safe. "         \
          "Do the last call within the main thread.")
#endif

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
    foreach(Values::value_type &v, hash)
    {
      object::rList l = new object::List();
      l->insertBack(new Float(v.second.sum / v.second.count));
      l->insertBack(new Float(v.second.min));
      l->insertBack(new Float(v.second.max));
      l->insertBack(new Float(v.second.count));
      res->set(libport::Symbol(v.first), l);
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
    StringPair p = split_name(v->get_name());
    rObject o = get_base(p.first);
    object::global_class
      ->slot_get(SYMBOL(UVar))
      ->slot_get(op)
      ->call(SYMBOL(syncEmit), o, o->slot_get(Symbol(p.second)),
             object::to_urbi(bound_context));
  }
}

static void periodic_call(rObject, ufloat interval, rObject method,
                          libport::Symbol msg, object::objects_type args)
{
  runner::Runner& r = ::kernel::runner();
  libport::utime_t delay = libport::seconds_to_utime(interval);
  while (true)
  {
    urbi::setCurrentContext(urbi::impl::KernelUContextImpl::instance());
    r.apply(method, msg, args);
    libport::utime_t target = libport::utime() + delay;
    r.yield_until(target);
  }
}

/// UObject read to an urbi variable.
static rObject urbi_get(rObject r, const std::string& slot)
{
  object::objects_type args;
  args << r;
  Symbol symSlot(slot);
  LIBPORT_DEBUG("applying get for " << slot << "...");
  rObject var = r->slot_get(symSlot);
  // Bypass the apply if we can.
  if (object::rUVar uvar = var->as<object::UVar>())
    return uvar->getter(true);
  else
    return ::kernel::runner().apply(var, symSlot, args);
}

/// UObject write to an urbi variable.
static rObject urbi_set(rObject r, const std::string& slot, rObject v)
{
  rObject name = new object::String(slot);
  object::objects_type args = list_of (r)(name)(v);
  LIBPORT_DEBUG("applying set...");
  rObject ret = ::kernel::runner().apply(r->slot_get(SYMBOL(updateSlot)),
                               SYMBOL(updateSlot), args);
  LIBPORT_DEBUG("done");
  return ret;
}


static rObject uvar_get(const std::string& name);
static rObject uvar_set(const std::string& name, rObject val);

static rObject uvar_uowned_get(const std::string& name);
static rObject uvar_uowned_set(const std::string& name, rObject val);


void uobjects_reload()
{
  urbi::impl::KernelUContextImpl::instance()->init();
  foreach (urbi::baseURBIStarterHub* i, urbi::baseURBIStarterHub::list())
    if (!libport::mhas(initialized, i))
    {
      initialized.insert(i);
    }
  foreach (urbi::baseURBIStarter* i, urbi::baseURBIStarter::list())
    if (!libport::mhas(initialized, i))
    {
      initialized.insert(i);
    }
}

/*! Initialize plugin UObjects.
 \param args object in which the instances will be stored.
*/
rObject uobject_initialize(const objects_type& args)
{
  urbi::setCurrentContext(new urbi::impl::KernelUContextImpl());
  where = args.front();
  where->slot_set(SYMBOL(setTrace), object::make_primitive(&setTrace));
  uobjects_reload();
  where->slot_set(SYMBOL(getStats),    object::make_primitive(&Stats::get));
  where->slot_set(SYMBOL(clearStats),  object::make_primitive(&Stats::clear));
  where->slot_set(SYMBOL(enableStats), object::make_primitive(&Stats::enable));
  return object::void_class;
}

// No rObject here as we do not want to prevent object destruction.
static boost::unordered_map<std::string, object::Object*> uobject_map;

static rObject wrap_ucallback_notify(const object::objects_type& ol ,
                                     urbi::UGenericCallback* ugc,
                                     std::string traceName)
{
  LIBPORT_DEBUG("uvwrapnotify");
  urbi::setCurrentContext(urbi::impl::KernelUContextImpl::instance());
  bound_context.push_back(std::make_pair(
      (&ugc->owner)? ugc->owner.__name:"unknown",
      ugc->name
      ));
  bool dummy = false;
  FINALLY(((bool, dummy)), bound_context.pop_back());
  urbi::UList l;
  l.array << new urbi::UValue();
  l[0].storage = ugc->target;
  libport::utime_t t = libport::utime();
  ugc->eval(l);
  Stats::add(ol.front().get(), traceName, libport::utime() - t);
  return object::void_class;
}

static void write_and_unfreeze(urbi::UValue& r, std::string& exception,
                               object::rTag* tag,
                               urbi::UValue& v, const std::exception* e)
{
  LOCK_KERNEL;
  if (e)
    exception = e->what();
  else
    r = v;
  (*tag)->unfreeze();
  // Wake up the poll task.
  ::kernel::urbiserver->wake_up();
}

static rObject wrap_ucallback(const object::objects_type& ol,
                              urbi::UGenericCallback* ugc,
                              const std::string& message)
{
  urbi::UList l;
  urbi::setCurrentContext(urbi::impl::KernelUContextImpl::instance());
  object::check_arg_count(ol.size() - 1, ugc->nbparam);
  bool tail = false;
  foreach (const rObject& co, ol)
  {
    if (!tail++)
      continue;
    urbi::UValue v = uvalue_cast(co);
    l.array << new urbi::UValue(v);
  }
  libport::utime_t start = libport::utime();
  urbi::UValue r;

  bound_context.push_back(std::make_pair(
      (&ugc->owner)? ugc->owner.__name:"unknown",
      ugc->name
      ));
  FINALLY(((bool, tail)), bound_context.pop_back());
  try
  {
    // This if is there to optimize the synchronous case.
    if (ugc->isSynchronous())
      r = ugc->__evalcall(l);
    else
    {
      libport::Finally f;
      object::rTag tag(new object::Tag);
      ::kernel::runner().apply_tag(tag, &f);
      // Tricky: tag->freeze() yields, but we must freeze tag before calling
      // eval or there will be a race if asyncEval goes to fast and unfreeze
      // before we freeze. So go throug backend.
      tag->value_get()->freeze();
      std::string exception;
      ugc->eval(l, boost::bind(write_and_unfreeze, boost::ref(r),
                               boost::ref(exception), &tag, _1, _2));
      ::kernel::runner().yield();
      if (!exception.empty())
        throw std::runtime_error("Exception in threaded call: " + exception);
    }
  }
  catch (const sched::exception&)
  {
    throw;
  }
  catch(const std::exception& e)
  {
    FRAISE("Exception caught while calling %s: %s",
           ugc->getName(), e.what());
  }
  catch(...)
  {
    FRAISE("Unknown exception caught while calling %s",
           ugc->getName());
  }
  start = libport::utime() - start;
  Stats::add(ol.front().get(), message, start);
  return object_cast(r);
}



static rObject
uobject_clone(const object::objects_type& l)
{
  rObject proto = l.front();
  return uobject_new(proto);
}

static rObject
uobject_finalize(const object::objects_type& args)
{
  rObject o = args.front();
  std::string objName = o->slot_get(SYMBOL(__uobjectName)).get<std::string>();
  // FIXME: uobject_to_robject[objName] should be enough
  urbi::UObject* uob = urbi::impl::KernelUContextImpl::instance()
    ->getUObject(objName);
  if (!uob)
    FRAISE("uobject_finalize: No uobject by the name of %s found", objName);
  delete uob;
  urbi::impl::KernelUContextImpl::instance()->objects.erase(objName);
  return object::void_class;
}

/*! Create the prototype for an UObject class.
*/
rObject
uobject_make_proto(const std::string& name)
{
  rObject oc =
    object::Object::proto
    ->slot_get(SYMBOL(UObject))
    ->call(SYMBOL(clone));
  oc->call(SYMBOL(uobjectInit));
  oc->call(SYMBOL(init));
  oc->slot_set(SYMBOL(finalize), new object::Primitive(&uobject_finalize));
  oc->slot_set(SYMBOL(__uobject_cname), new object::String(name));
  oc->slot_set(SYMBOL(__uobject_base), oc);
  oc->slot_set(SYMBOL(clone), new object::Primitive(&uobject_clone));
  oc->slot_set(SYMBOL(periodicCall), object::make_primitive(&periodic_call));
  return oc;
}

/*! Instanciate a new prototype inheriting from a UObject.
 A new instance of UObject is created
 \param proto proto object, created by uobject_make_proto() or uobject_new()
 \param forceName force the reported C++ name to be the class name
*/
rObject
uobject_new(rObject proto, bool forceName)
{
  rObject r = new object::Finalizable(proto->as<object::Finalizable>());

  // Get UObject name.
  rObject rcName = proto->slot_get(SYMBOL(__uobject_cname));
  const std::string& cname = rcName.cast<object::String>()->value_get();

  // Get the name we will pass to uobject.
  std::string name;
  if (forceName)
  {
    r->slot_set(SYMBOL(type), rcName);
    name = cname;
  }
  else
  {
    // boost::lexical_cast does not work on the way back, so dont use it here
    std::stringstream ss;
    ss << "uob_" << r.get();
    name = ss.str();
    /* We need to make this name accessible in urbi in case the UObject code
    emits urbi code using this name.*/
    where->slot_set(libport::Symbol(name), r);
  }
  uobject_map[name] = r.get();
  r->slot_set(SYMBOL(__uobjectName), object::to_urbi(name));
  r->call(SYMBOL(uobjectInit));
  // Instanciate UObject.
  foreach (urbi::baseURBIStarter* i, urbi::baseURBIStarter::list())
  {
    if (i->name == cname)
    {
      LIBPORT_DEBUG("Instanciating a new " << cname << " named "<< name);
      bound_context.push_back(std::make_pair(name, name + ".new"));
      FINALLY(((std::string, name)), bound_context.pop_back());
      i->instanciate(urbi::impl::KernelUContextImpl::instance(), name);
      return r;
    }
  }
  return r;
}

/** Find an UObject from its name.

The UObject class expects to know the variable name, i.e. a = new b;
should pass a to b's corresponding UObject ctor. Since we don't have this
information, we create a unique string, pass it to the ctor, and store it
in a.

But the user can create UVars based on the variable name it knows about,
i.e. a.val. So get_base must look in its uid map, and if it finds nothing,
look for an Urbi variable with given name. We expect all UObjects to be created
in Global.
*/

static rObject
get_base(const std::string& objname)
{
  rObject res = libport::find0(uobject_map, objname);
  object::Object::location_type s;
  // The user may be using the Urbi variable name.
  if (!res)
  {
    s = object::global_class->slot_locate(libport::Symbol(objname));
    // Not simplifyable! If the rSlot contains 0, casting to rObject will segv.
    if (s.second)
      res = *s.second;
  }
  if (!res)
  {
    s = where->slot_locate(libport::Symbol(objname));
    if (s.second)
      res = *s.second;
  }
  return res;
}


/// Get the uvar owner from is name
static rObject
uvar_owner_get(const std::string& name)
{
  return get_base(split_name(name).first);
}

/// Get the uvar name
static std::string
uvar_name_get(const std::string& name)
{
  return split_name(name).second;
}

/// Get an rObject from its uvar name
static rObject
uvar_get(const std::string& name)
{
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  if (!o)
    runner::raise_lookup_error(libport::Symbol(name), object::global_class);
  return urbi_get(o, p.second);
}

/// Write an rObject to a slot from its uvar name
static rObject
uvar_set(const std::string& name, rObject val)
{
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  if (!o)
    runner::raise_lookup_error(libport::Symbol(name), object::global_class);
  return urbi_set(o, p.second, val);
}

/// UVar get in owned mode: directly access the value.
static rObject
uvar_uowned_get(const std::string& name)
{
  LIBPORT_DEBUG("uowned get for "<<name);
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  if (!o)
    runner::raise_lookup_error(libport::Symbol(name), object::global_class);
  return o->slot_get(Symbol(p.second))
    ->slot_get(SYMBOL(val));
}
/// UVar set in owned mode: call Urbi-side writeOwned.
static rObject
uvar_uowned_set(const std::string& name, rObject val)
{
  LIBPORT_DEBUG("uowned set for "<<name);
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  if (!o)
    runner::raise_lookup_error(libport::Symbol(name), object::global_class);
  rObject v = o->slot_get(Symbol(p.second));
  object::objects_type args = list_of (v) (val);
  return ::kernel::runner().apply(v->slot_get(SYMBOL(writeOwned)),
                                  SYMBOL(writeOwned),
                                  args);
}

// Write to an UVar, pretending we are comming from lobby ctx.
static void writeFromContext(const std::string& ctx,
                             const std::string& varName,
                             const urbi::UValue& val)
{
  LOCK_KERNEL;
  if (ctx.substr(0, 2) != "0x")
    throw std::runtime_error("Invalid context: " + ctx);
  unsigned long l = strtol(ctx.c_str()+2, 0, 16);
  rLobby rl((object::Lobby*)l);
  runner::Runner& r = kernel::urbiserver->getCurrentRunner();
  rLobby cl = r.lobby_get();
  FINALLY(((rLobby, cl))((runner::Runner&, r)), r.lobby_set(cl));
  r.lobby_set(rl);
  object::rUValue ov(new object::UValue());
  ov->put(val, false);
  StringPair p = split_name(varName);
  rObject o = get_base(p.first);
  if (!o)
    runner::raise_lookup_error(libport::Symbol(varName), object::global_class);
  o->slot_get(Symbol(p.second))->call(SYMBOL(update_timed), ov,
                                      object::to_urbi(libport::utime()));
  ov->invalidate();
}

namespace urbi
{

  /*----------.
  | UObject.  |
  `----------*/

  static void bounce_update(urbi::UObject* ob, void* me, const std::string& key)
  {
    urbi::setCurrentContext(urbi::impl::KernelUContextImpl::instance());
    libport::utime_t t = libport::utime();
    ob->update();
    Stats::add(me, key, libport::utime()-t);
  }

  typedef std::pair<std::string, std::string> StringPair;
  /// Split a string of the form "a.b" in two
  static StringPair
  split_name(const std::string& name)
  {
    size_t p = name.find_last_of(".");
    std::string oname = name.substr(0, p);
    std::string slot = name.substr(p + 1, name.npos);
    return StringPair(oname, slot);
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
      instance_ = this;
    }
    void
    KernelUContextImpl::newUObjectClass(baseURBIStarter* s)
    {
      LOCK_KERNEL;
      object::rObject proto = uobject_make_proto(s->name);
      where->slot_set(libport::Symbol(s->name + "_class"), proto);
      // Make our first instance.
      where->slot_set(libport::Symbol(s->name), uobject_new(proto, true));
    }
    void
    KernelUContextImpl::newUObjectHubClass(baseURBIStarterHub* s)
    {
      LOCK_KERNEL;
      s->instanciate(this);
    }
    void
    KernelUContextImpl::send(const char* str)
    {
      LOCK_KERNEL;
      kernel::urbiserver->ghost_connection_get().received(str);
    }

    void
    KernelUContextImpl::send(const void* buf, size_t size)
    {
      LOCK_KERNEL;
      // Feed this to the ghostconnection.
      kernel::urbiserver->ghost_connection_get()
        .received(static_cast<const char*>(buf), size);
    }
    void KernelUContextImpl::yield() const
    {
      CHECK_MAINTHREAD();
      ::kernel::runner().yield();
    }
    void KernelUContextImpl::yield_until(libport::utime_t deadline) const
    {
      CHECK_MAINTHREAD();
      ::kernel::runner().yield_until(deadline);
    }
    void KernelUContextImpl::yield_for(libport::utime_t delay) const
    {
      CHECK_MAINTHREAD();
      ::kernel::runner().yield_for(delay);
    }
    void KernelUContextImpl::yield_until_things_changed() const
    {
      CHECK_MAINTHREAD();
      ::kernel::runner().yield_until_things_changed();
    }
    void KernelUContextImpl::side_effect_free_set(bool s)
    {
      CHECK_MAINTHREAD();
      ::kernel::runner().side_effect_free_set(s);
    }
    bool KernelUContextImpl::side_effect_free_get() const
    {
      CHECK_MAINTHREAD();
      return ::kernel::runner().side_effect_free_get();
    }
    UObjectHub*
    KernelUContextImpl::getUObjectHub(const std::string& n)
    {
      return UContextImpl::getUObjectHub(n);
    }

    UObject*
    KernelUContextImpl::getUObject(const std::string& n)
    {
      LOCK_KERNEL;
      UObject* res = UContextImpl::getUObject(n);
      if (res)
        return res;
      uobject_to_robject_type::iterator it = uobject_to_robject.find(n);
      if (it != uobject_to_robject.end())
        return it->second;
      rObject r = get_base(n);
      if (!r)
        return 0;
      std::string name =
        object::from_urbi<std::string>(r->slot_get(SYMBOL(__uobjectName)));
      if (name == n)
        return 0;
      return getUObject(name);
    }

    void
    KernelUContextImpl::call(const std::string& object,
                             const std::string& method,
                             UAutoValue v1,
                             UAutoValue v2,
                             UAutoValue v3,
                             UAutoValue v4,
                             UAutoValue v5,
                             UAutoValue v6,
                             UAutoValue v7,
                             UAutoValue v8)
    {
      // Bypass writeFromContext to avoid an extra UValue copy.
      if (method == "$uobject_writeFromContext")
      {
        std::string ctx = v1;
        std::string varname = v2;
        urbi::UValue val = v3;
        writeFromContext(v1, v2, v3);
        return;
      }
      LOCK_KERNEL;
      rObject b = get_base(object);
      object::objects_type args;
      args << b;

#define ARG(Nth)                               \
      do {                                     \
        if (v##Nth.type != DATA_VOID)          \
          args << object_cast(v##Nth);         \
      } while (false)

      ARG(1);
      ARG(2);
      ARG(3);
      ARG(4);
      ARG(5);
      ARG(6);
      ARG(7);
      ARG(8);
      b->call_with_this(libport::Symbol(method), args);
    }

    void
    KernelUContextImpl::declare_event(const UEvent* owner)
    {
      LOCK_KERNEL;
      rEvent e = new object::Event(object::Event::proto);
      StringPair p = split_name(owner->get_name());
      rObject o = get_base(p.first);
      if (!o)
        FRAISE("UEvent creation on non existing object: %s", p.first);
      else if (!o->local_slot_get(Symbol(p.second)))
        o->slot_set(Symbol(p.second), e);
    }

    void
    KernelUContextImpl::emit(const std::string& object,
                             UAutoValue& v1,
                             UAutoValue& v2,
                             UAutoValue& v3,
                             UAutoValue& v4,
                             UAutoValue& v5,
                             UAutoValue& v6,
                             UAutoValue& v7,
                             UAutoValue& v8
                             )
    {
      LOCK_KERNEL;
      StringPair p = split_name(object);
      rObject o = get_base(p.first)->slot_get(libport::Symbol(p.second));

      object::objects_type args;
      ARG(1);
      ARG(2);
      ARG(3);
      ARG(4);
      ARG(5);
      ARG(6);
      ARG(7);
      ARG(8);
      o->call(SYMBOL(emit), args);
    }

    void
    KernelUContextImpl::uobject_unarmorAndSend(const char* str)
    {
      LOCK_KERNEL;
      // Feed this to the ghostconnection.
      kernel::UConnection& ghost = kernel::urbiserver->ghost_connection_get();
      size_t len = strlen(str);
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
    TimerHandle
    KernelUContextImpl::setTimer(UTimerCallback* cb)
    {
      LOCK_KERNEL;
      rObject me = get_base(cb->objname);
      rObject f = me->slot_get(SYMBOL(setTimer));
      rObject p = new object::Float(cb->period / 1000.0);
      std::string stag = object::String::proto->as<object::String>()->fresh();
      rObject tag = new object::String(stag);
      rObject call = MAKE_VOIDCALL(cb, urbi::UTimerCallback, call);
      object::objects_type args = list_of (me)(p) (call)(tag);
      ::kernel::runner().apply(f, SYMBOL(setTimer), args);
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
      if (!h)
        return false;
      LOCK_KERNEL;
      rObject me = get_base(owner_->__name);
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

    void KernelUContextImpl::setHubUpdate(UObjectHub* hub, ufloat period)
    {
      // Call Urbi-side setHubUpdate, passing an rPrimitive wrapping
      // the 'update' call.
      LOCK_KERNEL;
      rObject uob = object::Object::proto->slot_get(SYMBOL(UObject));
      rObject f = uob->slot_get(SYMBOL(setHubUpdate));
      object::objects_type args = list_of
        (uob)
        (rObject(new object::String(hub->get_name())))
        (new object::Float(period / 1000.0))
        (MAKE_VOIDCALL(hub, urbi::UObjectHub, update));
      ::kernel::runner().apply(f, SYMBOL(setHubUpdate), args);
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
      kernel::urbiserver->synchronizer_get().lock();
    }

    void
    KernelUContextImpl::unlock()
    {
      kernel::urbiserver->synchronizer_get().unlock();
    }

    void
    KernelUObjectImpl::initialize(UObject* owner)
    {
      LOCK_KERNEL;
      owner_ = owner;
      LIBPORT_DEBUG("Uobject ctor for " << owner->__name);
      uobject_to_robject[owner_->__name] = owner;
      owner_->ctx_->registerObject(owner);
      rObject o = get_base(owner->__name);
      if (!o)
      {
        // Instanciation occured through ucontext::bind.
        o = uobject_make_proto(owner->__name);
        where->slot_set(Symbol(owner->__name), o);
      }
    }
    void
    KernelUObjectImpl::clean()
    {
      LOCK_KERNEL;
      uobject_to_robject.erase(owner_->__name);
    }
    void
    KernelUObjectImpl::setUpdate(ufloat period)
    {
      LOCK_KERNEL;
      rObject me = get_base(owner_->__name);
      rObject f = me->slot_get(SYMBOL(setUpdate));
      object::objects_type args;
      args << me;
      std::string meId = ::kernel::runner().apply(
        me->slot_get(SYMBOL(DOLLAR_id)), SYMBOL(DOLLAR_id), args)
        ->as<object::String>()->value_get();

      me->slot_update
        (SYMBOL(update),
         object::make_primitive
         (boost::function1<void, rObject>(
           boost::bind(&bounce_update, owner_, me.get(), meId + " update"))));
      args.clear();
      args << me
           << new object::Float(period / 1000.0);
      ::kernel::runner().apply(f, SYMBOL(setUpdate), args);
    }

    void
    KernelUVarImpl::unnotify()
    {
      LOCK_KERNEL;
      rObject r;
      // Try to locate the target urbiscript UVar, which might be gone.
      // In this case, callbacks are gone too.
      try {
        StringPair p = split_name(owner_->get_name());
        r = get_base(p.first);
        if (!r)
          return;
        r = r->slot_get(Symbol(p.second));
      }
      catch(...)
      {
        return;
      }
      foreach(KernelUGenericCallbackImpl* v, callbacks_)
      {
        Symbol s;
        if (v->owner_->type == "var")
          s = Symbol(v->owned_?"changeOwned":"change");
        else if (v->owner_->type == "varaccess")
	{
          r->slot_get(SYMBOL(access))->call(SYMBOL(remove), v->callback_);
          r->slot_get(SYMBOL(accessInLoop))->call(SYMBOL(remove), v->callback_);
          continue;
	}
        else
          continue;
        r->slot_get(s)->call(SYMBOL(remove), v->callback_);
      }
    }

    KernelUVarImpl::KernelUVarImpl()
      : bypassMode_(false)
    {
    }

    void
    KernelUVarImpl::initialize(UVar* owner)
    {
      LOCK_KERNEL;
      // Protect against multiple parallel creation of the same UVar.
      runner::Runner& runner = ::kernel::runner();
      bool prevState = runner.non_interruptible_get();
      FINALLY(
        ((bool, prevState))
        ((runner::Runner&, runner))
        ((UVar*, owner)),
        runner.non_interruptible_set(prevState););
      runner.non_interruptible_set(true);
      owner_ = owner;
      LIBPORT_DEBUG("__init " << owner_->get_name());
      owner_->owned = false;
      bypassMode_ = false;
      StringPair p = split_name(owner_->get_name());
      rObject o = get_base(p.first);
      if (!o)
        FRAISE("UVar creation on non existing object: %s", owner->get_name());
      Symbol varName(p.second);
      // Force kernel-side variable creation, init to void.
      rObject initVal;
      if (o->slot_locate(varName).first == o.get())
      {
        initVal = o->local_slot_get(varName)->value();
        // Check if the variable exists and is an uvar.
        if (initVal->slot_has(SYMBOL(owned)))
        {
          traceOperation(owner, SYMBOL(traceBind));
          return;
        }
        else
          o->slot_remove(varName);
      }
      //clone uvar
      LIBPORT_DEBUG("creating uvar "<<name);
      rObject protouvar = object::Object::proto->slot_get(SYMBOL(UVar));
      rObject uvar = protouvar->call(SYMBOL(new),
                                     o, new object::String(varName));
      // If the variable existed but was not an uvar, copy its old value.
      if (initVal)
        o->slot_get(varName)->slot_update(SYMBOL(val), initVal);
      traceOperation(owner, SYMBOL(traceBind));
    }
    void KernelUVarImpl::clean()
    {
      //noop
    }
    void KernelUVarImpl::setOwned()
    {
      LOCK_KERNEL;
      owner_->owned = true;
      // Write 1 to the Urbi-side uvar owned slot.
      StringPair p = split_name(owner_->get_name());
      rObject o = get_base(p.first);
      o->slot_get(Symbol(p.second))
        ->slot_update(SYMBOL(owned), object::true_class);
      LIBPORT_DEBUG("call to setowned on "<<owner_->get_name());
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
      LOCK_KERNEL;
      traceOperation(owner_, SYMBOL(traceSet));
      LIBPORT_DEBUG("uvar = operator for "<<owner_->get_name());
      object::rUValue ov(new object::UValue());
      ov->put(v, bypassMode_);
      if (owner_->owned)
        uvar_uowned_set(owner_->get_name(), ov);
      else
        uvar_set(owner_->get_name(), ov);
      ov->invalidate();
    }

    const UValue& KernelUVarImpl::get() const
    {
      LOCK_KERNEL;
      traceOperation(owner_, SYMBOL(traceGet));
      LIBPORT_DEBUG("uvar cast operator for "<<owner_->get_name());
      try {
        rObject o = (owner_->owned
                     ? ::uvar_uowned_get(owner_->get_name())
                     : ::uvar_get(owner_->get_name()));
        aver(o);
        if (object::rUValue bv = o->as<object::UValue>())
          return bv->value_get();
        else
        {
          cache_ = ::uvalue_cast(o);
          return cache_;
        }
      }
      catch (object::UrbiException&)
      {
        FRAISE("Invalid read of void UVar '%s'", owner_->get_name());
      }
    }

    ufloat& KernelUVarImpl::in()
    {
      throw std::runtime_error("in() is not implemented");
    }

    ufloat& KernelUVarImpl::out()
    {
      throw std::runtime_error("out() is not implemented");
    }

    UDataType
    KernelUVarImpl::type() const
    {
      LOCK_KERNEL;
      return ::uvalue_type(owner_->owned
                           ? uvar_uowned_get(owner_->get_name())
                           : uvar_get(owner_->get_name()));
    }

    UValue KernelUVarImpl::getProp(UProperty prop)
    {
      LOCK_KERNEL;
      StringPair p = split_name(owner_->get_name());
      rObject o = get_base(p.first);
      return ::uvalue_cast(o->call(SYMBOL(getProperty),
                                   new object::String(p.second),
                                   new object::String(UPropertyNames[prop])));
    }

    void KernelUVarImpl::setProp(UProperty prop, const UValue& v)
    {
      LOCK_KERNEL;
      StringPair p = split_name(owner_->get_name());
      rObject o = get_base(p.first);
      o->call(SYMBOL(setProperty),
              new object::String(p.second),
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
      StringPair p = split_name(owner_->get_name());
      rObject o = get_base(p.first);
      o->slot_get(Symbol(p.second))->slot_set(SYMBOL(rtp),
                                              object::to_urbi(enable));
    }

    time_t
    KernelUVarImpl::timestamp() const
    {
      rObject v = uvar_owner_get(owner_->get_name())
        ->getProperty(uvar_name_get(owner_->get_name()), SYMBOL(timestamp));
      return time_t(v ->as<object::Float>()->value_get());
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
      LOCK_KERNEL;
      initialize(owner, false);
    }

    void
    KernelUGenericCallbackImpl::registerCallback()
    {
      LOCK_KERNEL;
      runner::Runner& runner = ::kernel::runner();
      if (registered_)
      {
        std::cerr << "###UGenericcallback on " << owner_->name
                  << " already registered" << std::endl;
        return;
      }
      registered_ = true;
      StringPair p = split_name(owner_->name);
      std::string method = p.second;
      LIBPORT_DEBUG("ugenericcallback " << owner_->type << " " << p.first << " "
                    << method << "  " << owned_);
      // UObject owning the variable/event to monitor
      rObject me = get_base(p.first); //objname?
      aver(me);
      object::objects_type args = list_of(me);
      std::string meId =
        runner.apply(me->slot_get(SYMBOL(DOLLAR_id)), SYMBOL(DOLLAR_id),
                     args)
        ->as<object::String>()->value_get();
      std::string traceName = meId + "::" + owner_->type + "::"
        + string_cast((void*)this);
      if (owner_->type == "function")
      {
        traceName = owner_->name; // object.function, unique
        LIBPORT_DEBUG("binding " << p.first << "." << owner_->method);
        me->slot_set(libport::Symbol(method), object::make_primitive(
                       boost::function1<rObject, const objects_type&>
                       (boost::bind(&wrap_ucallback, _1, owner_, traceName))));
      }
      if (owner_->type == "var" || owner_->type == "varaccess")
      {
        // traceName is objName__obj__var
        traceName = ((&owner_->owner)? owner_->owner.__name:"unknown")
          + "__" + p.first + "__" + p.second;
        rObject var = me->slot_get(Symbol(method));
        aver(var);
        Symbol sym(SYMBOL(notifyAccess));
        if (owner_->type != "varaccess")
        {
          if (owned_)
            sym = SYMBOL(notifyChangeOwned);
          else
            sym = SYMBOL(notifyChange);
        }
        rObject source = get_base(owner_->objname);
        aver(source);
        rObject handle = source->slot_get(SYMBOL(handle));
        rObject f = var->slot_get(sym);
        aver(f);
        callback_ = object::make_primitive(
            boost::function1<rObject, const objects_type&>
            (boost::bind(&wrap_ucallback_notify, _1, owner_,
                         traceName)));
        callback_->slot_set
          (SYMBOL(target),
           new object::String(&owner_->owner ? owner_->owner.__name:"unknown"));
        object::objects_type args = list_of
          (var)
          (handle)
          (callback_);
        runner.apply(f, sym, args);
        if (owner_->target)
          static_cast<KernelUVarImpl*>(owner_->target->impl_)
            ->callbacks_ << this;
      }
    }
    void
    KernelUGenericCallbackImpl::clear()
    {
    }
  }

  std::string
  baseURBIStarter::getFullName(const std::string& name) const
  {
    return name;
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
    return object::system_class->slot_get(SYMBOL(version))->as<object::String>()
      ->value_get();
  }

}
