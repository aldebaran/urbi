#include "libport/hash.hh"
#include "object/atom.hh"
#include "object/urbi-exception.hh"
#include "object/primitives.hh"
#include "object/idelegate.hh"
#include "object/object.hh"
#include "kernel/userver.hh"
#include "kernel/uconnection.hh"
#include "urbi/uobject.hh"
#include "uvalue-cast.hh"
#include "uobject.hh"

using object::rObject;

/*! Initialize plugin UObjects.
 \param where object in which the instances will be stored.
*/
void uobject_initialize(object::rObject where)
{
   BOOST_FOREACH (urbi::baseURBIStarter* i, *urbi::objectlist)
  {
    //make our prototype
    object::rObject proto =  uobject_make_proto(i->name);
    where->slot_set(i->name+"_class", proto);
    //make our first instance
    where->slot_set(i->name, uobject_new(proto, true));
  }
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


rObject 
UWrapCallback::operator() (runner::Runner&, object::objects_type ol)
{
  urbi::UList l;
  bool first = true;
  BOOST_FOREACH(rObject co, ol)
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

rObject uobject_make_proto(const std::string& name)
{
  rObject oc = object::clone(object::object_class);
  oc->slot_set("__uobject_cname", new object::String(name));
  oc->slot_set("__uobject_base", oc);
  oc->slot_set("clone", new object::Primitive(&uobject_clone));
  return oc;
}

/*! Instanciate a new prototype inheriting from a UObject.
 A new instance of UObject is created
 \param proto proto object, created by uobject_make_proto() or uobject_new()
 \param forceName force the reported C++ name to be the class name
*/
rObject uobject_new(rObject proto, bool forceName)
{
  rObject r = object::clone(object::object_class);
  
  // Reparent r to proto base.
  r->proto_remove(object::object_class);
  r->proto_add(proto->slot_get("__uobject_base")); 
  
  // Get UObject name.
  rObject rcName = proto->slot_get("__uobject_cname");
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
  BOOST_FOREACH (urbi::baseURBIStarter* i, *urbi::objectlist)
  {
    if (i->name == cname)
    {
      i->init(name);
      return r;
    }
  }
  return r;
}

static rObject
get_base(const std::string& objname)
{
  return uobject_map[objname];
}

namespace urbi 
{

UGenericCallback::UGenericCallback(const std::string& objname,
		     const std::string& type,
		     const std::string& name,
		     int, urbi::UTable&, bool)
{
  if (type == "function")
  {
    std::string method = name.substr(name.find_last_of(".") + 1,
				     std::string::npos);
    rObject me = get_base(objname);
    std::cerr << "binding " << objname << "." << method << std::endl;
    me->slot_set(method, new object::Delegate(new UWrapCallback(this)));
  }
}
UGenericCallback::UGenericCallback(const std::string& ,
		     const std::string& ,
		     const std::string& , urbi::UTable&)
{
}

UGenericCallback::~UGenericCallback() {}

void UGenericCallback::registerCallback(UTable& ) 
{
}

UObject::~UObject() 
{
}

UObject::UObject(const std::string& name) 
:__name(name)
{

}
void UObject::UJoinGroup(const std::string& ) 
{
}

typedef std::pair<std::string, std::string> StringPair;

/// Split a string of the form "a.b" in two
static  StringPair split_name(const std::string& name)
{
  int p = name.find_last_of(".");
  std::string oname = name.substr(0, p);
  std::string slot = name.substr(p + 1, name.npos);
  return StringPair(oname, slot);
}
/// Get an rObject from its uvar name
static rObject uvar_get(const std::string& name)
{
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  return o->slot_get(p.second);
}

/// Write an rObject to a slot from its uvar name
static void uvar_set(const std::string& name, rObject val)
{
  StringPair p = split_name(name);
  rObject o = get_base(p.first);
  try 
  {
    o->slot_set(p.second,val);
  }
  catch(object::RedefinitionError)
  {
    o->slot_update(p.second, val);
  }
}

#define UVAR_OPERATORS(T, DT)     \
void UVar::operator = (DT t)      \
{                                 \
  uvar_set(name, ::object_cast(urbi::UValue(t))); \
}                                 \
UVar::operator T()                \
{                                 \
  return ::uvalue_cast(uvar_get(name)); \
}

UVAR_OPERATORS(ufloat, ufloat);
UVAR_OPERATORS(std::string, const std::string&);
UVAR_OPERATORS(UBinary, const UBinary&);
UVAR_OPERATORS(UList, const UList&);

//no corresponding operator= for this one...
UVar::operator int()                
{                                 
  return ::uvalue_cast(uvar_get(name)); 
}

void UVar::__init()
{
  //nothing to do
}

UVar::~UVar() 
{
}

void echo(const char * format, ...)
{
  va_list arg;
  va_start(arg, format);
  vfprintf(stderr, format, arg);
  va_end(arg);
}
}
