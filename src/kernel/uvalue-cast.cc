#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <kernel/uvalue-cast.hh>
#include <object/cxx-conversions.hh>
#include <object/float.hh>
#include <object/global.hh>
#include <object/list.hh>
#include <object/string.hh>
#include <runner/raise.hh>
#include <urbi/uvalue.hh>

urbi::UDataType uvalue_type(object::rObject o)
{
  CAPTURE_GLOBAL(Binary);
  if (object::rFloat f = o->as<object::Float>())
    return urbi::DATA_DOUBLE;
  if (object::rString s = o->as<object::String>())
    return urbi::DATA_STRING;
  if (object::rList s = o->as<object::List>())
    return urbi::DATA_LIST;
  if (is_a(o, Binary))
    return urbi::DATA_BINARY;
  if (o == object::void_class)
    return urbi::DATA_VOID;
  return urbi::DATA_OBJECT;
}

urbi::UValue uvalue_cast(object::rObject o)
{
  CAPTURE_GLOBAL(Binary);
  urbi::UValue res;
  if (object::rFloat f = o->as<object::Float>())
    res = f->value_get();
  else if (object::rString s = o->as<object::String>())
    res = s->value_get();
  else if (object::rList s = o->as<object::List>())
  {
    res.type = urbi::DATA_LIST;
    res.list = new urbi::UList;
    object::List::value_type& t = o.cast<object::List>()->value_get();
    foreach (const object::rObject& co, t)
      res.list->array.push_back(new urbi::UValue(uvalue_cast(co)));
  }
  else if (is_a(o, Binary))
  {
    const std::string& data =
      o->slot_get(SYMBOL(data))->as<object::String>()->value_get();
    const std::string& keywords =
      o->slot_get(SYMBOL(keywords))->
      as<object::String>()->value_get();
    std::list<urbi::BinaryData> l;
    l.push_back(urbi::BinaryData(const_cast<char*>(data.c_str()),
				 data.size()));
    std::list<urbi::BinaryData>::iterator i = l.begin();
    res.type = urbi::DATA_BINARY;
    res.binary = new urbi::UBinary();
    res.binary->parse(
      (boost::lexical_cast<std::string>(data.size()) +
       " " + keywords + '\n').c_str(), 0, l, i);
  }
  else // We could not find how to cast this value
  {
    const object::rString& rs =
      o->slot_get(SYMBOL(type))->as<object::String>();
    runner::raise_argument_type_error
      (0, rs, object::to_urbi(SYMBOL(LT_exportable_SP_object_GT)),
       object::to_urbi(SYMBOL(cast)));
  }
  return res;
}

object::rObject
object_cast(const urbi::UValue& v)
{
  object::rObject res;
  switch(v.type)
  {
    case urbi::DATA_DOUBLE:
      return new object::Float(v.val);
      break;

    case urbi::DATA_STRING:
      return new object::String(*v.stringValue);
      break;

    case urbi::DATA_LIST:
    {
      object::rList l = new object::List();
      foreach (urbi::UValue *cv, v.list->array)
	l->value_get().push_back(object_cast(*cv));
      res = l;
      break;
    }

    case urbi::DATA_BINARY:
    {
      CAPTURE_GLOBAL(Binary);
      res = new object::Object();
      res->proto_add(Binary);
      std::string msg = v.binary->getMessage();
      // Trim it.
      if (!msg.empty() && msg[0] == ' ')
        msg = msg.substr(1, msg.npos);
      res->slot_set(SYMBOL(keywords),
                    new object::String(msg));
      res->slot_set(SYMBOL(data),
                    new object::String
                    (std::string(static_cast<char*>(v.binary->common.data),
				 v.binary->common.size)));
    }
    break;

    case urbi::DATA_VOID:
      res = object::void_class;
    break;

    default:
      static boost::format m("<external data with type %1%>");
      runner::raise_argument_type_error
	(0, object::to_urbi((m % v.type).str()),
	 object::object_class,
	 object::to_urbi(SYMBOL(backcast)));
  }
  return res;
}
