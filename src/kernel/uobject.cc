#include <cstdarg>

#include <boost/assign.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#ifdef ENABLE_DEBUG_TRACES_UOBJECT
# define ENABLE_DEBUG_TRACES
#endif

#include <libport/compiler.hh>
#include <libport/contract.hh>
#include <libport/foreach.hh>
#include <libport/hash.hh>

#include <kernel/config.h>

#include <kernel/uconnection.hh>
#include <kernel/userver.hh>
#include <kernel/uvalue-cast.hh>
#include <kernel/uobject.hh>

#include <object/cxx-primitive.hh>
#include <object/float.hh>
#include <object/global.hh>
#include <object/object.hh>
#include <object/object.hh>
#include <object/string.hh>
#include <object/symbols.hh>
#include <object/system.hh>
#include <object/urbi-exception.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>

#include <urbi/uobject.hh>


// Make it more readable.
using namespace boost::assign;
using object::rObject;
using object::rLobby;
using object::objects_type;
using object::void_class;
using object::nil_class;
using runner::Runner;
using libport::Symbol;

// Where to store uobjects
static rObject where;
typedef libport::hash_map<std::string, urbi::UObject*> uobject_to_robject_type;
static uobject_to_robject_type uobject_to_robject;

#define MAKE_VOIDCALL(ptr, cls, meth)                   \
  object::make_primitive(                               \
    boost::function1<void, rObject>(                    \
      boost::bind(&cls::meth, ptr)))

#ifdef SCHED_CORO_OSTHREAD
# define CHECK_MAINTHREAD()
#else
# define CHECK_MAINTHREAD()				\
  if (::kernel::urbiserver->isAnotherThread())		\
    pabort("UObject API isn't thread safe. "		\
	   "Do the last call within main thread.")
#endif

static inline runner::Runner& getCurrentRunner()
{
  return ::kernel::urbiserver->getCurrentRunner();
}

static void periodic_call(rObject, ufloat interval, rObject method,
                          libport::Symbol msg, object::objects_type args)
{
  runner::Runner& r = getCurrentRunner();
  libport::utime_t delay = libport::seconds_to_utime(interval);
  while(true)
  {
    r.apply(method, msg, args);
    libport::utime_t target = libport::utime() + delay;
    r.yield_until(target);
  }
}

/// UObject read to an urbi variable.
static rObject urbi_get(rObject r, const std::string& slot)
{
  object::objects_type args;
  args.push_back(r);
  ECHO("applying get for " << slot << "...");
  rObject ret =  getCurrentRunner().apply(r->slot_get(Symbol(slot)),
                                          Symbol(slot), args);
  ECHO("done");
  return ret;
}

/// UObject write to an urbi variable.
static rObject urbi_set(rObject r, const std::string& slot, rObject v)
{
  rObject name = new object::String(slot);
  object::objects_type args = list_of (r)(name)(v);
  ECHO("applying set...");
  rObject ret = getCurrentRunner().apply(r->slot_get(SYMBOL(updateSlot)),
					 SYMBOL(updateSlot), args);
  ECHO("done");
  return ret;
}


static rObject uvar_get(const std::string& name);
static rObject uvar_set(const std::string& name, rObject val);

static rObject uvar_uowned_get(const std::string& name);
static rObject uvar_uowned_set(const std::string& name, rObject val);


typedef std::pair<std::string, std::string> StringPair;

/// Split a string of the form "a.b" in two
static StringPair split_name(const std::string& name)
{
  size_t p = name.find_last_of(".");
  std::string oname = name.substr(0, p);
  std::string slot = name.substr(p + 1, name.npos);
  return StringPair(oname, slot);
}

void uobjects_reload()
{
  static std::set<void*> initialized;
  foreach (urbi::baseURBIStarterHub* i, urbi::baseURBIStarterHub::list())
    if (!libport::mhas(initialized, i))
    {
      i->init(i->name);
      initialized.insert(i);
    }
  foreach (urbi::baseURBIStarter* i, urbi::baseURBIStarter::list())
    if (!libport::mhas(initialized, i))
    {
      object::rObject proto = uobject_make_proto(i->name);
      where->slot_set(libport::Symbol(i->name + "_class"), proto);
      // Make our first instance.
      where->slot_set(libport::Symbol(i->name), uobject_new(proto, true));
      initialized.insert(i);
    }
}

/*! Initialize plugin UObjects.
 \param args object in which the instances will be stored.
*/
rObject uobject_initialize(const objects_type& args)
{
  where = args.front();
  uobjects_reload();
  return object::void_class;
}

// No rObject here as we do not want to prevent object destruction.
static libport::hash_map<std::string, object::Object*> uobject_map;

static rObject wrap_ucallback_notify(const object::objects_type&,
                                     urbi::UGenericCallback* ugc)
{
  ECHO("uvwrapnotify");
  urbi::UList l;
  l.array.push_back(new urbi::UValue());
  l[0].storage = ugc->storage;
  ugc->__evalcall(l);
  return object::void_class;
}

static rObject wrap_ucallback(const object::objects_type& ol,
                              urbi::UGenericCallback* ugc)
{
  urbi::UList l;
  object::check_arg_count(ol.size() - 1, ugc->nbparam);
  bool tail = false;
  foreach (const rObject& co, ol)
  {
    if (!tail++)
      continue;
    urbi::UValue v = uvalue_cast(co);
    l.array.push_back(new urbi::UValue(v));
  }
  urbi::UValue r = ugc->__evalcall(l);
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
  urbi::UObject* uob = urbi::getUObject(objName);
  if (!uob)
    runner::raise_primitive_error("uobject_finalize: No uobject by the name of "
                                  +objName+" found");
  foreach (urbi::baseURBIStarter* i, urbi::baseURBIStarter::list())
  {
    if (i->name == objName)
    {
      i->clean();
      break;
    }
  }
  return object::void_class;
}

/*! Create the prototype for an UObject class.
*/
rObject
uobject_make_proto(const std::string& name)
{
  rObject oc = object::Object::proto->slot_get(SYMBOL(UObject))->clone();
  object::objects_type args;
  args.push_back(oc);
  getCurrentRunner().apply(oc->slot_get(SYMBOL(init)), SYMBOL(init), args);
  oc->slot_set(SYMBOL(finalize), new object::Primitive(&uobject_finalize));
  oc->slot_set(SYMBOL(__uobject_cname),
	       new object::String(name));
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
  rObject r = proto->clone();

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
  // Instanciate UObject.
  foreach (urbi::baseURBIStarter* i, urbi::baseURBIStarter::list())
  {
    if (i->name == cname)
    {
      ECHO( "Instanciating a new " << cname << "named "<<name);
      if (i->getUObject())
        i->copy(name);
      else
        i->init(name);
      return r;
    }
  }
  return r;
}

/** Find an UObject from its name.

The UObject class expects to know the variable name,i.e. a = new b;
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
  rObject res = uobject_map[objname];
  // The user may be using the Urbi variable name.
  if (!res)
    res = object::global_class->slot_get(libport::Symbol(objname));
  if (!res)
    res = where->slot_get(libport::Symbol(objname));
  return res;
}

namespace urbi
{
  UObjectHub*
  getUObjectHub(const std::string& n)
  {
    return baseURBIStarterHub::find(n);
  }

  UObject*
  getUObject(const std::string& n)
  {
    UObject* res = baseURBIStarter::find(n);
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
    return getUObject(name);
  }
}

/// Get an rObject from its uvar name
static rObject
uvar_get(const std::string& name)
{
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  return urbi_get(o, p.second);
}

/// Write an rObject to a slot from its uvar name
static rObject
uvar_set(const std::string& name, rObject val)
{
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  return urbi_set(o, p.second, val);
}

/// UVar get in owned mode: directly access the value.
static rObject
uvar_uowned_get(const std::string& name)
{
  ECHO("uowned get for "<<name);
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  return o->slot_get(Symbol(p.second))
    ->slot_get(SYMBOL(val));
}
/// UVar set in owned mode: call Urbi-side writeOwned.
static rObject
uvar_uowned_set(const std::string& name, rObject val)
{
  ECHO("uowned set for "<<name);
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  rObject v = o->slot_get(Symbol(p.second));
  object::objects_type args = list_of (v) (val);
  return getCurrentRunner().apply(v->slot_get(SYMBOL(writeOwned)),
                                  SYMBOL(writeOwned),
                                  args);
}

namespace urbi
{

  UObjectMode getRunningMode()
  {
    return MODE_PLUGIN;
  }

  struct CallbackStorage
  {
    std::string type;
    std::string name;
    bool owned;
  };


  /*-------------------.
  | UGenericCallback.  |
  `-------------------*/

  /* We cannot make any call that might trigger the callback, as this is
   * the base class constructor.
   * So just store the constructor arguments, and perform the real work in
   * registerCallback()
   */
  UGenericCallback::UGenericCallback(const std::string&,
				     const std::string& type,
				     const std::string& name,
				     int, urbi::UTable&, bool owned)
  {
    CallbackStorage& s = *new CallbackStorage;
    s.type = type;
    s.name = name;
    s.owned = owned;
    storage = &s;
  }

  /** Called by UNotifyChange, UNotifyAcces and UBindFunction calls.
   */
  void UGenericCallback::registerCallback(UTable& )
  {
    // Check if we handle this callback type.
    if (!storage)
      return;
    CallbackStorage& s = *reinterpret_cast<CallbackStorage*>(storage);
    StringPair p = split_name(s.name);
    std::string method = p.second;
    ECHO("ugenericcallback " << s.type << " " << p.first << " "
         << method << "  " << s.owned);
    rObject me = get_base(p.first); //objname?
    assertion(me);
    if (s.type == "function")
    {
      ECHO( "binding " << p.first << "." << method );
      me->slot_set(libport::Symbol(method),
		   object::make_primitive(
	boost::function1<rObject, const objects_type&>
	(boost::bind(&wrap_ucallback, _1, this))));
    }
    if (s.type == "var" || s.type == "varaccess")
    {
      rObject var = me->slot_get(Symbol(method));
      assertion(var);
      Symbol sym(SYMBOL(notifyAccess));
      if (s.type != "varaccess")
      {
	if (s.owned)
          sym = SYMBOL(notifyChangeOwned);
	else
	  sym = SYMBOL(notifyChange);
      }
      rObject f = var->slot_get(sym);
      assertion(f);
      object::objects_type args = list_of
        (var)
	(object::make_primitive(
	boost::function1<rObject, const objects_type&>
	(boost::bind(&wrap_ucallback_notify, _1, this))));
      getCurrentRunner().apply(f, sym, args);
    }
    delete &s;
  }

  UGenericCallback::UGenericCallback(const std::string&,
				     const std::string&,
				     const std::string&, urbi::UTable&)
    : storage(0)
  {
  }

  UGenericCallback::~UGenericCallback() {}

  UTimerCallback::UTimerCallback(const std::string& objname,
				 ufloat period, UTimerTable&)
  {
    rObject me = get_base(objname);
    rObject f = me->slot_get(SYMBOL(setTimer));
    rObject p = new object::Float(period / 1000.0);
    rObject call = MAKE_VOIDCALL(this, urbi::UTimerCallback, call);
    object::objects_type args = list_of (me)(p) (call);
    getCurrentRunner().apply(f, SYMBOL(setTimer), args);
  }

  UTimerCallback::~UTimerCallback()
  {
  }


  /*----------.
  | UObject.  |
  `----------*/

  void UObject::USetUpdate(ufloat t)
  {
    CHECK_MAINTHREAD();
    rObject me = get_base(__name);
    rObject f = me->slot_get(SYMBOL(setUpdate));
    me->slot_update(SYMBOL(update), MAKE_VOIDCALL(this, urbi::UObject, update));
    object::objects_type args;// = list_of(new object::Float(t / 1000.0));
    args.push_back(me);
    args.push_back(new object::Float(t / 1000.0));
    getCurrentRunner().apply(f, SYMBOL(setUpdate), args);
  }

  UObject::~UObject()
  {
    uobject_to_robject.erase(__name);
  }

  UObject::UObject(const std::string& name)
    : __name(name)
    , autogroup(false)
    , load(name, "load")
  {
    ECHO( "Uobject ctor for " << __name );
    load = 1;
    uobject_to_robject[name] = this;
  }

  void
  UObject::UJoinGroup(const std::string&)
  {
    ECHO("Groups not supported yet");
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


  /*-------.
  | UVar.  |
  `-------*/

  void UVar::syncValue()
  {
    // Nothing to do
  }

#define UVAR_OPERATORS(T, DT)                                   \
  UVar& UVar::operator= (DT t)                                  \
  {                                                             \
    ECHO("uvar = operator for "<<name);                         \
    if (owned)                                                  \
      uvar_uowned_set(name, ::object_cast(urbi::UValue(t)));    \
    else                                                        \
      uvar_set(name, ::object_cast(urbi::UValue(t)));           \
    return *this;                                               \
  }                                                             \
  UVar::operator T() const                                      \
  {                                                             \
    ECHO("uvar cast operator for "<<name);                      \
    try {                                                       \
      if (owned)                                                \
        return ::uvalue_cast(uvar_uowned_get(name));            \
      else                                                      \
        return ::uvalue_cast(uvar_get(name));                   \
    }                                                           \
    catch (object::UrbiException&)				\
    {                                                           \
      runner::raise_primitive_error				\
	("Invalid read of void UVar '" + name + "'");		\
    }								\
  }

  UVAR_OPERATORS(ufloat, ufloat);
  UVAR_OPERATORS(std::string, const std::string&);
  UVAR_OPERATORS(UBinary, const UBinary&);
  UVAR_OPERATORS(UList, const UList&);
  UVAR_OPERATORS(USound, const USound&);
  UVAR_OPERATORS(UImage, const UImage&);

#undef UVAR_OPERATORS

  //no corresponding operator= for this one...
  UVar::operator int() const
  {
    return libport::numeric_cast<int>(static_cast<ufloat>(*this));
  }

  UDataType
  UVar::type() const
  {
    return ::uvalue_type(owned ? uvar_uowned_get(name) : uvar_get(name));
  }

  void
  UVar::__init()
  {
    ECHO("__init " << name);
    owned = false;
    StringPair p = split_name(name);
    rObject o = get_base(p.first);
    assertion(o);
    Symbol varName(p.second);
    // Force kernel-side variable creation, init to void.
    rObject initVal;
    if (o->slot_locate(varName) == o.get())
    {
      initVal = o->own_slot_get(varName)->value();
      // Check if the variable exists and is an uvar.
      if (initVal->slot_locate(SYMBOL(owned)))
	return;
      else
	o->slot_remove(varName);
    }
    //clone uvar
    ECHO("creating uvar "<<name);
    rObject protouvar = object::Object::proto->slot_get(SYMBOL(UVar));
    rObject uvar = protouvar->call(SYMBOL(new),
                                   o, new object::String(varName));
    // If the variable existed but was not an uvar, copy its old value.
    if (initVal)
      o->slot_get(varName)->slot_update(SYMBOL(val), initVal);
  }

  void
  UVar::setOwned()
  {
    owned = true;
    // Write 1 to the Urbi-side uvar owned slot.
    StringPair p = split_name(name);
    rObject o = get_base(p.first);
    o->slot_get(Symbol(p.second))
      ->slot_update(SYMBOL(owned), object::true_class);
    ECHO("call to setowned on "<<name);
  }

#define SET_PROP(argtype, out)                                   \
  void                                                           \
  UVar::setProp(urbi::UProperty prop, argtype v)                 \
  {                                                              \
    StringPair p = split_name(name);                             \
    rObject o = get_base(p.first);                               \
    o->call(SYMBOL(setProperty),                                 \
            new object::String(p.second),			 \
            new object::String(UPropertyNames[prop]),		 \
            out);                                                \
  }

  SET_PROP(const char*, new object::String(v))
  SET_PROP(ufloat, new object::Float(v))
  SET_PROP(const urbi::UValue&, object_cast(v))

#undef SET_PROP

  UValue
  UVar::getProp(urbi::UProperty prop)
  {
    StringPair p = split_name(name);
    rObject o = get_base(p.first);
    return ::uvalue_cast(
      o->call(SYMBOL(getProperty),
              new object::String(p.second),
              new object::String(UPropertyNames[prop])));
  }

  UVar::~UVar()
  {
  }

  /*-------------.
  | UObjectHub.  |
  `-------------*/

  UObjectHub::~UObjectHub()
  {
  }

  void
  UObjectHub::USetUpdate(ufloat t)
  {
    /* Call Urbi-side setHubUpdate, passing an rPrimitive wrapping the 'update'
     * call.
     */
    CHECK_MAINTHREAD();
    rObject uob = object::Object::proto->slot_get(SYMBOL(UObject));
    rObject f = uob->slot_get(SYMBOL(setHubUpdate));
    object::objects_type args = list_of
      (uob)
      (rObject(new object::String(name)))
      (new object::Float(t / 1000.0))
      (MAKE_VOIDCALL(this, urbi::UObjectHub, update));
    getCurrentRunner().apply(f, SYMBOL(setHubUpdate), args);
  }

  void
  uobject_unarmorAndSend(const char* str)
  {
    // Feed this to the ghostconnection.
    kernel::UConnection& ghost = kernel::urbiserver->ghost_connection_get();
    size_t len = strlen(str);
    if (2 <= len && str[0] == '(')
      ghost.received(str + 1, len - 2);
    else
      ghost.received(str);
  }

  int
  UObject::send(const std::string& str)
  {
    kernel::urbiserver->ghost_connection_get().received(str);
    return 0;
  }


  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  void
  echo(const char* format, ...)
  {
    if (format)
    {
      va_list arg;
      va_start(arg, format);
      vfprintf(stderr, format, arg);
      va_end(arg);
    }
  }

  void
  send(const char* str)
  {
    kernel::urbiserver->ghost_connection_get().received(str);
  }

  void
  send(void* buf, size_t size)
  {
    // Feed this to the ghostconnection.
    kernel::urbiserver->ghost_connection_get()
      .received(static_cast<const char*>(buf), size);
  }
  void yield()
  {
    getCurrentRunner().yield();
  }
  void yield_until(libport::utime_t deadline)
  {
    getCurrentRunner().yield_until(deadline);
  }
  void yield_until_things_changed()
  {
    getCurrentRunner().yield_until_things_changed();
  }
  void side_effect_free_set(bool s)
  {
    getCurrentRunner().side_effect_free_set(s);
  }
  bool side_effect_free_get()
  {
    return getCurrentRunner().side_effect_free_get();
  }

  std::string
  baseURBIStarter::getFullName(const std::string& name)
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
