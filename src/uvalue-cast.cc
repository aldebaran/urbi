#include "object/atom.hh"
#include "object/urbi-exception.hh"
#include "urbi/uvalue.hh"





urbi::UValue uvalue_cast(object::rObject o)
{
  urbi::UValue v;
  switch(o->kind_get())
  {
  case object::Object::kind_delegate:
  case object::Object::kind_primitive:
  case object::Object::kind_lobby:
  case object::Object::kind_code:
  case object::Object::kind_object:
    throw object::WrongArgumentType
       (object::Object::kind_float, o->kind_get(), "cast");
    break;
    
#define HANDLE_TYPE(k, t) \
  case object::Object::k:  \
    v = o.cast<object::t>()->value_get(); \
    break;
  HANDLE_TYPE(kind_float, Float);
  HANDLE_TYPE(kind_integer, Integer);
  HANDLE_TYPE(kind_string, String);
#undef HANDLE_TYPE    
  case object::Object::kind_list:
    {
      v.type = urbi::DATA_LIST;
      std::list<object::rObject>& t = o.cast<object::List>()->value_get();
      BOOST_FOREACH(object::rObject co, t)
      {
	v.list->array.push_back(new urbi::UValue(uvalue_cast(co)));
      }
    }
    break;  
  }
  return v;
}

object::rObject object_cast(const urbi::UValue& v)
{
  object::rObject r;
  switch(v.type)
  {
  case urbi::DATA_DOUBLE:
      r = new object::Float(v.val);
      break;
    case urbi::DATA_STRING:
      r = new object::String(*v.stringValue);
      break;
    case urbi::DATA_LIST:
      r = new object::List(object::list_traits::type());
      BOOST_FOREACH(urbi::UValue *cv, v.list->array)
      {
	r.cast<object::List>()->value_get().push_back(object_cast(*cv));
      }
      break;
    default:
      throw object::WrongArgumentType(object::Object::kind_float, 
			      object::Object::kind_float, "cast");
      break;
  }
  return r;
}
