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
  case object::Object::kind_code:
  case object::Object::kind_delegate:
  case object::Object::kind_global:
  case object::Object::kind_lobby:
  case object::Object::kind_primitive:
  case object::Object::kind_scope:
  case object::Object::kind_tag:
  case object::Object::kind_task:
    throw object::WrongArgumentType
      (object::Object::kind_float, o->kind_get(), "cast");
    break;
  case object::Object::kind_object:
    if (!is_a(o, object::global_class->slot_get(SYMBOL(Binary))))
      boost::throw_exception (object::WrongArgumentType
       (object::Object::kind_object, o->kind_get(), "bin-cast"));
      {
	const std::string& data =
	  o->slot_get(SYMBOL(data))->value<object::String>().name_get();
	const std::string& keywords =
	  o->slot_get(SYMBOL(keywords))->value<object::String>().name_get();
	std::list<urbi::BinaryData> l;
	l.push_back(urbi::BinaryData(const_cast<char*>(data.c_str()),
			      data.size()));
	std::list<urbi::BinaryData>::iterator i = l.begin();
	res.type = urbi::DATA_BINARY;
	res.binary = new urbi::UBinary();
	res.binary->parse(
		   (boost::lexical_cast<std::string>(data.size()) +
		   " " + keywords + '\n').c_str(), 0, l, i);
	return res;
      }
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
      res.list = new urbi::UList;
      object::List::value_type& t = o.cast<object::List>()->value_get();
      foreach (const object::rObject& co, t)
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
      return object::Float::fresh(v.val);
      break;
    case urbi::DATA_STRING:
      return object::String::fresh(libport::Symbol(*v.stringValue));
      break;
    case urbi::DATA_LIST:
      res = object::List::fresh(object::list_traits::type());
      foreach (urbi::UValue *cv, v.list->array)
	res.cast<object::List>()->value_get().push_back(object_cast(*cv));
      break;
    case urbi::DATA_BINARY:
      {
	res = object::Object::fresh();
	res->proto_add(object::global_class->slot_get(SYMBOL(Binary)));
	std::string msg = v.binary->getMessage();
	// Trim it.
	if (!msg.empty() && msg[0] == ' ' )
	  msg = msg.substr(1, msg.npos);
	res->slot_set(SYMBOL(keywords), object::String::fresh
		    (libport::Symbol(msg)));
	res->slot_set(SYMBOL(data), object::String::fresh(libport::Symbol(
		std::string(static_cast<char*>(v.binary->common.data),
	      v.binary->common.size))));
      }
      break;
    default:
      throw
	object::WrongArgumentType(object::Object::kind_float,
				  object::Object::kind_float, "backcast");
      break;
  }
  return res;
}
