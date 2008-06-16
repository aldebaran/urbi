#include <boost/bind.hpp>
#include <boost/assign.hpp>

#ifdef ENABLE_DEBUG_TRACES_UOBJECT
# define ENABLE_DEBUG_TRACES
#endif

#include <libport/compiler.hh>
#include <libport/foreach.hh>
#include <libport/hash.hh>


#include <object/atom.hh>
#include <object/urbi-exception.hh>
#include <object/primitives.hh>
#include <object/idelegate.hh>
#include <object/object.hh>
#include <object/object-class.hh>
#include <object/global-class.hh>
#include <kernel/userver.hh>
#include <kernel/uconnection.hh>
#include <urbi/uobject.hh>
#include <kernel/uvalue-cast.hh>
#include <kernel/uobject.hh>


// Make it more readable.
using namespace boost::assign;
using object::rObject;
using object::rLobby;
using object::objects_type;
using object::void_class;
using object::nil_class;
using object::object_class;
using runner::Runner;
using libport::Symbol;

static inline runner::Runner& getCurrentRunner()
{
  return  ::urbiserver->getCurrentRunner();
}

/// UObject read to an urbi variable.
static rObject urbi_get(rObject r, const std::string& slot)
{
  object::objects_type args = list_of(r);
  ECHO("applying get for " << slot << "...");
  rObject ret =  getCurrentRunner().apply(r->slot_get(Symbol(slot)),
                                          Symbol(slot), args);
  ECHO("done");
  return ret;
}

/// UObject write to an urbi variable.
static rObject urbi_set(rObject r, const std::string& slot, rObject v)
{
  object::objects_type args = list_of
    (r) (new object::String(Symbol(slot))) (v);
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
static  StringPair split_name(const std::string& name)
{
  int p = name.find_last_of(".");
  std::string oname = name.substr(0, p);
  std::string slot = name.substr(p + 1, name.npos);
  return StringPair(oname, slot);
}

/*! Initialize plugin UObjects.
 \param args object in which the instances will be stored.
*/
rObject uobject_initialize(runner::Runner&, objects_type args)
{
  rObject where = args.front();
  foreach (urbi::baseURBIStarterHub* i, *urbi::objecthublist)
    i->init(i->name);
  foreach (urbi::baseURBIStarter* i, *urbi::objectlist)
  {
    object::rObject proto = uobject_make_proto(i->name);
    where->slot_set(libport::Symbol(i->name + "_class"), proto);
    // Make our first instance.
    where->slot_set(libport::Symbol(i->name), uobject_new(proto, true));
  }
  return object::void_class;
}

static libport::hash_map<std::string, rObject> uobject_map;


class UWrapCallback: public object::IDelegate
{
  public:
  UWrapCallback(urbi::UGenericCallback* ugc)
  :ugc_(ugc)
  {}
  virtual ~UWrapCallback() {}
  virtual rObject operator() (runner::Runner&, object::objects_type);
  private:
  urbi::UGenericCallback * ugc_;
};

template<typename F>
class UWrapFunction: public object::IDelegate
{
  public:
  UWrapFunction(const F& f)
  :f_(f)
  {}
  virtual ~UWrapFunction() {}
  virtual rObject operator() (runner::Runner&, object::objects_type)
  {
    f_();
    return object::void_class;
  }
  private:
    F f_;
};

template<typename F>
UWrapFunction<F>* uwrapfunction(const F& f)
{
  return new UWrapFunction<F>(f);
}


class UWrapNotify: public object::IDelegate
{
  public:
    UWrapNotify(urbi::UGenericCallback* ugc, const std::string& obj,
      const std::string& slot, bool owned, bool setter)
  :ugc_(ugc),
  obj(obj),
  slot(slot),
  owned_(owned),
  setter_(setter)
  {}
  virtual ~UWrapNotify() {}
  virtual rObject operator() (runner::Runner&, object::objects_type);
  private:
  urbi::UGenericCallback * ugc_;
  std::string obj, slot;
  bool owned_;
  bool setter_;
};
rObject
UWrapNotify::operator() (runner::Runner&, object::objects_type)
{
  ECHO("uvwrapnotify "<<obj<<" "<<slot<<" o="<<owned_
    << "  s="<<setter_);
  urbi::UVar v(obj, slot);
  if (owned_)
    v.setOwned();
  urbi::UList l;
  l.array.push_back(new urbi::UValue());
  l[0].storage = &v;
  ugc_->__evalcall(l);
  return object::void_class;
}

rObject
UWrapCallback::operator() (runner::Runner&, object::objects_type ol)
{
  urbi::UList l;
  bool first = true;
  foreach (const rObject& co, ol)
  {
    if (first)
    {
      first = false;
      continue;
    }
    urbi::UValue v = uvalue_cast(co);
    l.array.push_back(new urbi::UValue(v));
  }
  urbi::UValue r = ugc_->__evalcall(l);
  return object_cast(r);
}



static rObject
uobject_clone(runner::Runner&, object::objects_type l)
{
  rObject proto = l.front();
  return uobject_new(proto);
}

/*! Create the prototype for an UObject class.
*/
rObject
uobject_make_proto(const std::string& name)
{
  rObject oc = object_class->slot_get(SYMBOL(UObject))->clone();
  object::objects_type args = list_of(oc);
  getCurrentRunner().apply(oc->slot_get(SYMBOL(init)), SYMBOL(init), args);
  oc->slot_set(SYMBOL(__uobject_cname),
	       new object::String(libport::Symbol(name)));
  oc->slot_set(SYMBOL(__uobject_base), oc);
  oc->slot_set(SYMBOL(clone), new object::Primitive(&uobject_clone));
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
  rObject r = object::object_class->clone();

  // Reparent r to proto base.
  r->proto_remove(object::object_class);
  r->proto_add(proto->slot_get(SYMBOL(__uobject_base)));

  // Get UObject name.
  rObject rcName = proto->slot_get(SYMBOL(__uobject_cname));
  const std::string& cname = rcName.cast<object::String>()->value_get();

  // Get the name we will pass to uobject.
  std::string name;
  if (forceName)
    name = cname;
  else
  {
    // boost::lexical_cast does not work on the way back, so dont use it here
    std::stringstream ss;
    ss << r.get();
    name = ss.str();
  }
  uobject_map[name] = r;
  // Instanciate UObject.
  foreach (urbi::baseURBIStarter* i, *urbi::objectlist)
  {
    if (i->name == cname)
    {
      ECHO( "Instanciating a new " << cname << "named "<<name);
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
  return res;
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
  return getCurrentRunner().apply(v->slot_get(SYMBOL(writeOwned)), SYMBOL(writeOwned), args);
}

namespace urbi
{

  struct CallbackStorage
  {
    std::string type;
    std::string name;
    bool owned;
  };
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
    // Cheock if we handle this callback type.
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
		   new object::Delegate(new UWrapCallback(this)));
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
	(new object::Delegate(new UWrapNotify(this, p.first,
					      method, s.owned, true)));
      getCurrentRunner().apply(f, sym, args);
    }
    delete &s;
  }

  UGenericCallback::UGenericCallback(const std::string&,
				     const std::string&,
				     const std::string&, urbi::UTable&)
  :storage(0)
  {
  }

  UGenericCallback::~UGenericCallback() {}

  UTimerCallback::UTimerCallback(const std::string& objname,
				 ufloat period, UTimerTable&)
  {
    rObject me = get_base(objname);
    rObject f = me->slot_get(SYMBOL(setTimer));
    object::objects_type args = list_of
      (me)
      (new object::Float(period))
      (new object::Delegate(
	uwrapfunction(boost::bind(&urbi::UTimerCallback::call,this))));
    getCurrentRunner().apply(f, SYMBOL(setTimer), args);
  }
  UTimerCallback::~UTimerCallback()
  {
  }
  void UObject::USetUpdate(ufloat t)
  {
    rObject me = get_base(__name);
    rObject f = me->slot_get(SYMBOL(setUpdate));
    me->slot_update(getCurrentRunner(), SYMBOL(update),
		    new object::Delegate(
		      uwrapfunction(boost::bind(&urbi::UObject::update, this))));
    object::objects_type args = list_of (me) (new object::Float(t));
    getCurrentRunner().apply(f, SYMBOL(setUpdate), args);
  }
  UObject::~UObject()
  {
  }

  UObject::UObject(const std::string& name)
    :__name(name),
    load(name, "load")
  {
    ECHO( "Uobject ctor for " << __name );
    load = 1;
  }
  void
  UObject::UJoinGroup(const std::string&)
  {
    ECHO( "Groups not supported yet" );
  }

  typedef std::pair<std::string, std::string> StringPair;
  /// Split a string of the form "a.b" in two
  static StringPair
  split_name(const std::string& name)
  {
    int p = name.find_last_of(".");
    std::string oname = name.substr(0, p);
    std::string slot = name.substr(p + 1, name.npos);
    return StringPair(oname, slot);
  }

#define UVAR_OPERATORS(T, DT)				\
  void UVar::operator = (DT t)				\
  {							\
  ECHO("uvar = operator for "<<name);  \
  if (owned)                                            \
    uvar_uowned_set(name, ::object_cast(urbi::UValue(t))); \
  else                                                  \
    uvar_set(name, ::object_cast(urbi::UValue(t)));	\
  }							\
  UVar::operator T()					\
  {							\
  ECHO("uvar cast operator for "<<name); \
  if (owned)                                            \
    return ::uvalue_cast(uvar_uowned_get(name));        \
  else                                                  \
    return ::uvalue_cast(uvar_get(name));		\
  }

  UVAR_OPERATORS(ufloat, ufloat);
  UVAR_OPERATORS(std::string, const std::string&);
  UVAR_OPERATORS(UBinary, const UBinary&);
  UVAR_OPERATORS(UList, const UList&);
  UVAR_OPERATORS(USound, const USound&);
  UVAR_OPERATORS(UImage, const UImage&);
  //no corresponding operator= for this one...
  UVar::operator int()
  {
  if (owned)
    return ::uvalue_cast(uvar_uowned_get(name));
  else
    return ::uvalue_cast(uvar_get(name));
  }

  void
  UVar::__init()
  {
    ECHO("__init "<< name );
    owned = false;
    StringPair p = split_name(name);
    rObject o = get_base(p.first);
    assertion(o);
    Symbol varName(p.second);
    // Force kernel-side variable creation, init to void.
    rObject initVal;
    if (o->slot_locate(varName) == o.get())
    {
      initVal = o->own_slot_get(varName);
      // Check if the variable exists and is an uvar.
      if (initVal->slot_locate(SYMBOL(owned)))
	return;
      else
	o->slot_remove(varName);
    }
    //clone uvar
    ECHO("creating uvar "<<name);
    rObject protouvar = object::object_class->slot_get(SYMBOL(uvar));
    rObject uvar = urbi_call(getCurrentRunner(), protouvar, SYMBOL(new),
			     o, new object::String(varName));
    // If the variable existed but was not an uvar, copy its old value.
    if (initVal)
      o->slot_get(varName)->slot_update(getCurrentRunner(),
					SYMBOL(val), initVal);
  }

  void
  UVar::setOwned()
  {
    owned = true;
    // Write 1 to the Urbi-side uvar owned slot.
    StringPair p = split_name(name);
    rObject o = get_base(p.first);
    o->slot_get(Symbol(p.second))
      ->slot_update(getCurrentRunner(), SYMBOL(owned), new object::Float(1));
    ECHO("call to setowned on "<<name);
  }

  void
  UVar::setProp(urbi::UProperty prop, ufloat v)
  {
    StringPair p = split_name(name);
    rObject o = get_base(p.first);
    urbi_call(getCurrentRunner(), o, SYMBOL(setProperty),
	      new object::String(Symbol(p.second)),
	      new object::String(Symbol(UPropertyNames[prop])),
	      new object::Float(v));
  }

  UValue
  UVar::getProp(urbi::UProperty prop)
  {
    StringPair p = split_name(name);
    rObject o = get_base(p.first);
    return ::uvalue_cast(
      urbi_call(getCurrentRunner(), o, SYMBOL(getProperty),
		new object::String(Symbol(p.second)),
		new object::String(Symbol(UPropertyNames[prop]))));
  }

  UVar::~UVar()
  {
  }

  void
  echo(const char * format, ...)
  {
    va_list arg;
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
  }
  UObjectHub::~UObjectHub()
  {
  }

  void
  UObjectHub::USetUpdate(ufloat t)
  {
    /* Call Urbi-side setHubUpdate, passing an IDelegate wrapping the 'update'
     * call.
     */
    rObject uob = object_class->slot_get(SYMBOL(UObject));
    rObject f = uob->slot_get(SYMBOL(setHubUpdate));
    object::objects_type args = list_of
      (uob)
      (new object::String(Symbol(name)))
      (new object::Float(t))
      (new object::Delegate
       (uwrapfunction(boost::bind(&urbi::UObjectHub::update, this))));
    getCurrentRunner().apply(f, SYMBOL(setHubUpdate), args);
  }

  void
  uobject_unarmorAndSend(const char* str)
  {
    // Feed this to the ghostconnection.
    UConnection& ghost = urbiserver->getGhostConnection();
    if (strlen(str)>=2 && str[0]=='(')
      ghost.received(static_cast<const char *>(str+1), strlen(str)-2);
    else
      ghost.received(str);
  }

  void
  send(const char* str)
  {
    // Feed this to the ghostconnection.
    urbiserver->getGhostConnection().received(str);
  }

  void
  send(void* buf, int size)
  {
    // Feed this to the ghostconnection.
    urbiserver->getGhostConnection().received(static_cast<const char*>(buf),
					      size);
  }
}
