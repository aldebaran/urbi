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
static libport::hash_map<std::string, rObject> uobject_map;

class UWrapCallback: public object::IDelegate
{
  public:
  UWrapCallback(urbi::UGenericCallback* ugc)
  :ugc_(ugc) 
  {}
  virtual ~UWrapCallback() {}
  virtual rObject operator() (object::rLobby, object::objects_type);
  private:
  urbi::UGenericCallback * ugc_;
};


rObject 
UWrapCallback::operator() (object::rLobby, object::objects_type ol)
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
uobject_clone(object::rLobby, object::objects_type l)
{
  rObject proto = l.front();
  return uobject_new(proto);
}

rObject uobject_make_proto(std::string& name)
{
  rObject oc = object::clone(object::object_class);
  oc->slot_set("__uobject_cname", new object::String(name));
  oc->slot_set("__uobject_base", 
	       oc);
  oc->slot_set("clone", new object::Primitive(&uobject_clone));
  return oc;
}

rObject uobject_new(rObject proto, bool forceName)
{
  rObject r = object::clone(object::object_class);
  
  //proto to proto base
  r->proto_remove(object::object_class);
  r->proto_add(proto->slot_get("__uobject_base")); 
  
  //get proto name
  rObject rcName = proto->slot_get("__uobject_cname");
  const std::string& cname = rcName.cast<object::String>()->value_get();
  
  //get the name we will pass to uobject
  std::string name;
  if (forceName)
    name = cname;
  else
  {
    //boost::lexical_cast does not work on the way back, so dont use it here
    std::stringstream ss;
    ss << r.get();
    name = ss.str();
  }
  uobject_map[name] = r;
  //instanciate UObject
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


/// Get an rObject from its uvar name
static rObject uvar_get(const std::string& name)
{
  int p = name.find_last_of(".");
  std::string oname = name.substr(0, p);
  std::string slot = name.substr(p + 1, name.npos);
  rObject o = get_base(oname);
  return o->slot_get(slot);
}

/// Write an rObject to a slot from its uvar name
static void uvar_set(const std::string& name, rObject val)
{
  int p = name.find_last_of(".");
  std::string oname = name.substr(0, p);
  std::string slot = name.substr(p + 1, name.npos);
  rObject o = get_base(oname);
  try 
  {
    o->slot_set(slot,val);
  }
  catch(object::RedefinitionError)
  {
    o->slot_update(slot, val);
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
