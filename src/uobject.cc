#include "object/atom.hh"
#include "object/urbi-exception.hh"
#include "object/primitives.hh"
#include "object/idelegate.hh"
#include "object/object.hh"
#include "kernel/userver.hh"
#include "kernel/uconnection.hh"
#include "urbi/uobject.hh"
#include "uvalue_cast.hh"

class UWrapCallback: public object::IDelegate
{
  public:
  UWrapCallback(urbi::UGenericCallback * ugc):ugc(ugc) {}
  virtual ~UWrapCallback() {}
  virtual object::rObject operator() (object::rLobby, object::objects_type);
  private:
  urbi::UGenericCallback * ugc;
};


object::rObject 
UWrapCallback::operator() (object::rLobby, object::objects_type ol)
{
  urbi::UList l;
  bool first = true;
  BOOST_FOREACH(object::rObject co, ol)
  {
    if (first)
    {  
      first = false;
      continue;
    }
    urbi::UValue v = uvalue_cast(co);
    l.array.push_back(new urbi::UValue(v));
  }
  urbi::UValue r = ugc->__evalcall(l);
  return object_cast(r);
}

namespace urbi {

UGenericCallback::UGenericCallback(const std::string& objname,
		     const std::string& type,
		     const std::string& name,
		     int , urbi::UTable&, bool )
{
  if (type == "function")
  {
    std::string method=name.substr(name.find_last_of(".")+1, std::string::npos);
    std::cerr <<"binding "<<objname<<"."<<method<<std::endl;
    object::rObject mylobby = ::urbiserver->getGhostConnection().get_lobby();
    object::rObject uobj = mylobby->slot_get("uobject");
    object::rObject me = uobj->slot_get(objname);
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

UVar::~UVar() 
{
}
}
