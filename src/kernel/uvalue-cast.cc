/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
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
#include <kernel/uobject.hh>
#include <urbi/object/cxx-conversions.hh>
#include <urbi/object/event.hh>
#include <urbi/object/float.hh>
#include <urbi/object/global.hh>
#include <urbi/object/list.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/dictionary.hh>
#include <urbi/object/matrix.hh>
#include <urbi/object/string.hh>
#include <urbi/object/vector.hh>
#include <urbi/object/symbols.hh>
#include <object/uvalue.hh>
#include <runner/job.hh>
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
  pabort(*o);
}

urbi::UValue uvalue_cast(const object::rObject& o, int recursionLevel)
{
  // Protect ourselve against user errors.
  static int maxRecursionLevel = 0;
  if (!maxRecursionLevel)
  {
    maxRecursionLevel = 50;
    if (getenv("URBI_MAX_CAST_RECURSION_LEVEL"))
      maxRecursionLevel = strtol(getenv("URBI_MAX_CAST_RECURSION_LEVEL"), 0,0);
  }
  if (recursionLevel > maxRecursionLevel)
    FRAISE("maximum level of recursion reached while casting to uvalue. Loop?");
  CAPTURE_GLOBAL(Binary);
  CAPTURE_GLOBAL(UObject);
  CAPTURE_GLOBAL(UVar);
  urbi::UValue res;
  if (object::rUValue bv = o->as<object::UValue>())
    res = bv->value_get();
  else if (object::rFloat f = o->as<object::Float>())
    res = f->value_get();
  else if (o == object::true_class)
    res = 1;
  else if (o == object::false_class)
    res = 0;
  else if (o == object::nil_class || o == object::void_class)
    ; // Nothing to do, DATA_VOID is fine.
  else if (object::rString s = o->as<object::String>())
    res = s->value_get();
  else if (object::rList s = o->as<object::List>())
  {
    res.type = urbi::DATA_LIST;
    res.list = new urbi::UList;
    object::List::value_type& t = o.cast<object::List>()->value_get();
    foreach (const object::rObject& co, t)
      res.list->array << new urbi::UValue(uvalue_cast(co));
  }
  else if (object::rDictionary s = o->as<object::Dictionary>())
  {
    res.type = urbi::DATA_DICTIONARY;
    res.dictionary = new urbi::UDictionary;
    urbi::UDictionary& r = *res.dictionary;
    typedef object::Dictionary::value_type::value_type value_type;
    foreach (const value_type& p, s->value_get())
    {
      // Currently, only strings are valid keys.
      if (const object::rString s = p.first->as<object::String>())
        r[s->value_get()] = uvalue_cast(p.second);
      else
        // Keep message sync with share/urbi/uobject.u
        // (Dictionary.uvalueSerialize).
        FRAISE("Dictionaries exchanged with UObjects can"
               " only have String keys: %s (%s)",
               *p.first, *p.first->getSlotValue(SYMBOL(type)));
    }
  }
  else if (is_a(o, Binary))
  {
    const std::string& data =
      o->slot_get_value(SYMBOL(data))->as<object::String>()->value_get();
    std::string keywords =
      o->slot_get_value(SYMBOL(keywords))->as<object::String>()->value_get();
    std::list<urbi::BinaryData> l;
    l << urbi::BinaryData(const_cast<char*>(data.c_str()), data.size());
    std::list<urbi::BinaryData>::const_iterator i = l.begin();
    res.type = urbi::DATA_BINARY;
    res.binary = new urbi::UBinary();
    res.binary->parse(libport::format("%s%s%s\n",
                                      data.size(),
                                      keywords.empty() ? "" : " ",
                                      keywords).c_str(),
                      0, l, i);
  }
  else if (is_a(o, UObject))
    res = o->slot_get_value(SYMBOL(__uobjectName))
      ->as<object::String>()->value_get();
  else if (is_a(o, UVar))
  {
    // Storing the address is Safe: either the callback will use it, in
    // which case KernelUVarImpl::ruvar_ will hold a ref, or it will be
    // ignored
    std::stringstream ss;
    ss << "@." << o.get();
    res = ss.str();
  }
  else if (object::rVector ov = o->as<object::Vector>())
    res, ov->value_get();
  else if (object::rMatrix om = o->as<object::Matrix>())
    res, om->value_get();
  else if (o->slot_has(SYMBOL(uvalueSerialize)))
    res = uvalue_cast(o->call(SYMBOL(uvalueSerialize)), recursionLevel+1);
  else // We could not find how to cast this value
  {
    const object::rString& rs =
      o->slot_get_value(SYMBOL(type))->as<object::String>();
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
  switch (v.type)
  {
    case urbi::DATA_DOUBLE:
      return new object::Float(v.val);
      break;

    case urbi::DATA_STRING:
      return new object::String(*v.stringValue);
      break;

    case urbi::DATA_SLOTNAME:
        // could be an uobject
        res = urbi::uobjects::get_base(*v.stringValue);
        // or anything
        if (!res)
          res =
        object::global_class->slot_get_value(libport::Symbol(*v.stringValue));
      break;

    case urbi::DATA_LIST:
    {
      object::rList l = new object::List();
      foreach (urbi::UValue *cv, v.list->array)
	l->value_get() << object_cast(*cv);
      res = l;
      break;
    }

    case urbi::DATA_DICTIONARY:
    {
      object::rDictionary r = new object::Dictionary();
      foreach (const urbi::UDictionary::value_type& d, *v.dictionary)
        r->set(new object::String(d.first), object_cast(d.second));
      res = r;
      if (libport::has(*v.dictionary, "$sn")) // Serialized class
        res = r->call(SYMBOL(uvalueDeserialize));
      break;
    }

    case urbi::DATA_BINARY:
    {
      CAPTURE_GLOBAL(Binary);
      res = new object::Object();
      res->proto_add(Binary);
      std::string msg = v.binary->getMessage();
      // Try to interpret vector and matrix
      std::string kw; int elemSize; std::string elemType; int count1=-1;
      int count2 = -1;
      std::stringstream s(msg);
      s >> kw >> elemSize >> elemType >> count1 >> count2;
      using libport::ufloat;
      if (kw == "packed" && elemSize == sizeof(ufloat))
      {
        if (count2 == -1)
          return new object::Vector(
            urbi::uvalue_caster<libport::vector_type>()
                                    (const_cast<urbi::UValue&>(v)));
        else
          return new object::Matrix(
            urbi::uvalue_caster<libport::matrix_type>()
                                    (const_cast<urbi::UValue&>(v)));
      }
      // The rest goes into a generic Binary.
      res->slot_set_value(SYMBOL(keywords), new object::String(msg));
      res->slot_set_value(SYMBOL(data),
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


object::rObject
uvalue_deserialize(object::rObject s)
{
  using namespace object;
  CAPTURE_GLOBAL(Serializables);
  CAPTURE_GLOBAL(Object);
  rObject lobby =
    ::kernel::urbiserver->getCurrentRunner().state.lobby_get();
  if (rList l = s->as<List>())
  {
    // Recurse
    rList res = new List;
    foreach (rObject& r, l->value_get())
      res->insertBack(uvalue_deserialize(r));
    return res;
  }
  else if (rDictionary d = s->as<Dictionary>())
  {
    // Check if this is a serialized class
    static object::rString sn = new object::String(SYMBOL(DOLLAR_sn));
    if (d->has(sn))
    {
      // Look for the class
      libport::Symbol cn = from_urbi<libport::Symbol>(d->get(sn));
      rObject proto = lobby->slot_get_value(cn, false);
      if (!proto)
      {
        proto = Serializables->slot_get_value(cn, false);
        if (!proto)
          proto = Object;
      }
      rObject res = proto->call(SYMBOL(new));
      foreach (Dictionary::value_type::value_type& v, d->value_get())
      {
        libport::Symbol key = object::from_urbi<libport::Symbol>(v.first);
        if (key != SYMBOL(DOLLAR_sn))
        {
          if (rObject s = res->local_slot_get(key))
            res->slot_update(key, uvalue_deserialize(v.second));
          else
            res->slot_set_value(key, uvalue_deserialize(v.second));
        }
      }
      return res;
    }
    else
    {
      // Recurse
      rDictionary res = new Dictionary;
      foreach (Dictionary::value_type::value_type& v, d->value_get())
        res->set(v.first, uvalue_deserialize(v.second));
      return res;
    }
  }
  else
    // Everything else deserializes into itself
    return s;
}
