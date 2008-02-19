#include "object/atom.hh"
#include "object/urbi-exception.hh"
#include "urbi/uvalue.hh"
#include "uvalue-cast.hh"




urbi::UValue uvalue_cast(object::rObject o)
{
  urbi::UValue res;
  switch(o->kind_get())
  {
  case object::Object::kind_alien:
  case object::Object::kind_call:
  case object::Object::kind_code:
  case object::Object::kind_delegate:
  case object::Object::kind_lobby:
  case object::Object::kind_object:
  case object::Object::kind_primitive:
  case object::Object::kind_task:
    throw object::WrongArgumentType
       (object::Object::kind_float, o->kind_get(), "cast");
    break;

#define HANDLE_TYPE(k, t) \
  case object::Object::k:  \
    res = o.cast<object::t>()->value_get(); \
    break;
  HANDLE_TYPE(kind_float, Float);
  HANDLE_TYPE(kind_integer, Integer);
  HANDLE_TYPE(kind_string, String);
#undef HANDLE_TYPE
  case object::Object::kind_list:
    {
      res.type = urbi::DATA_LIST;
      std::list<object::rObject>& t = o.cast<object::List>()->value_get();
      BOOST_FOREACH(object::rObject co, t)
	res.list->array.push_back(new urbi::UValue(uvalue_cast(co)));
    }
    break;
  }
  return res;
}

object::rObject object_cast(const urbi::UValue& v)
{
  object::rObject res;
  switch(v.type)
  {
  case urbi::DATA_DOUBLE:
      return new object::Float(v.val);
      break;
    case urbi::DATA_STRING:
      return new object::String(libport::Symbol(*v.stringValue));
      break;
    case urbi::DATA_LIST:
      res = new object::List(object::list_traits::type());
      BOOST_FOREACH(urbi::UValue *cv, v.list->array)
	res.cast<object::List>()->value_get().push_back(object_cast(*cv));
      break;
    default:
      throw object::WrongArgumentType(object::Object::kind_float,
			      object::Object::kind_float, "cast");
      break;
  }
  return res;
}
