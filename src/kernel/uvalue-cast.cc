/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/format.hh>
#include <libport/lexical-cast.hh>

#include <kernel/uvalue-cast.hh>
#include <urbi/object/cxx-conversions.hh>
#include <urbi/object/float.hh>
#include <urbi/object/global.hh>
#include <urbi/object/list.hh>
#include <urbi/object/dictionary.hh>
#include <urbi/object/string.hh>
#include <object/symbols.hh>
#include <object/uvalue.hh>
#include <object/uvar.hh>
#include <urbi/runner/raise.hh>
#include <urbi/uvalue.hh>

urbi::UDataType uvalue_type(const object::rObject& o)
{
  CAPTURE_GLOBAL(Binary);
  if (object::rUValue bv = o->as<object::UValue>())
    return bv->value_get().type;
  if (o->as<object::Float>())
    return urbi::DATA_DOUBLE;
  if (o->as<object::String>())
    return urbi::DATA_STRING;
  if (o->as<object::List>())
    return urbi::DATA_LIST;
  if (o->as<object::Dictionary>())
    return urbi::DATA_DICTIONARY;
  if (is_a(o, Binary))
    return urbi::DATA_BINARY;
  if (o == object::void_class)
    return urbi::DATA_VOID;
  return urbi::DATA_OBJECT;
}

urbi::UValue uvalue_cast(const object::rObject& o)
{
  CAPTURE_GLOBAL(Binary);
  CAPTURE_GLOBAL(UObject);
  CAPTURE_GLOBAL(UVar);
  urbi::UValue res;
  if (object::rUValue bv = o->as<object::UValue>())
    return bv->value_get();
  else if (object::rFloat f = o->as<object::Float>())
    res = f->value_get();
  else if (o == object::true_class)
    res = 1;
  else if (o == object::false_class)
    res = 0;
  else if (o == object::nil_class)
    ; // Nothing to do, DATA_VOID is fine.
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
  else if (object::rDictionary s = o->as<object::Dictionary>())
  {
    res.type = urbi::DATA_DICTIONARY;
    res.dictionary = new urbi::UDictionary;
    object::Dictionary::value_type& r = s->value_get();
    foreach (const object::Dictionary::value_type::value_type& d, r)
      (*res.dictionary)[d.first] = uvalue_cast(d.second);
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
    std::list<urbi::BinaryData>::const_iterator i = l.begin();
    res.type = urbi::DATA_BINARY;
    res.binary = new urbi::UBinary();
    res.binary->parse(
      (boost::lexical_cast<std::string>(data.size()) +
       " " + keywords + '\n').c_str(), 0, l, i);
  }
  else if (is_a(o, UObject))
  {
    res = o->slot_get(SYMBOL(__uobjectName))->as<object::String>()
      ->value_get();
  }
  else if (is_a(o, UVar))
  {
    res =
      o->slot_get(SYMBOL(ownerName))->as<object::String>()->value_get()
      + "."
      + o->slot_get(SYMBOL(initialName))->as<object::String>()->value_get();
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

    case urbi::DATA_SLOTNAME:
      return object::global_class->slot_get(libport::Symbol(*v.stringValue));
      break;

    case urbi::DATA_LIST:
    {
      object::rList l = new object::List();
      foreach (urbi::UValue *cv, v.list->array)
	l->value_get().push_back(object_cast(*cv));
      res = l;
      break;
    }

    case urbi::DATA_DICTIONARY:
    {
      object::rDictionary r = new object::Dictionary();
      foreach (const urbi::UDictionary::value_type& d, *v.dictionary)
        r->set(libport::Symbol(d.first), object_cast(d.second));
      res = r;
      break;
    }

    case urbi::DATA_BINARY:
    {
      CAPTURE_GLOBAL(Binary);
      res = new object::Object();
      res->proto_add(Binary);
      std::string msg = v.binary->getMessage();
      res->slot_set(SYMBOL(keywords),
                    new object::String(msg));
      res->slot_set(SYMBOL(data),
                    new object::String
                    (std::string(static_cast<char*>(v.binary->common.data),
				 v.binary->common.size)));
    }
    break;

    case urbi::DATA_VOID:
    // UObject: cast 'void' UValue (default ctor) into nil, not void.
    // Otherwise, you can end up with nasty stuffs like void in Lists.
    // Void is no value, UValue is, so UValue cannot be void.
      res = object::nil_class;
    break;

    default:
      static boost::format m("<external data with type %1%>");
      runner::raise_argument_type_error
	(0, object::to_urbi((m % v.type).str()),
	 object::Object::proto,
	 object::to_urbi(SYMBOL(backcast)));
  }
  return res;
}
