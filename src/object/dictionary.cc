/**
 ** \file object/dictionary-class.cc
 ** \brief Creation of the URBI object dictionary.
 */

#include <libport/containers.hh>

#include <object/dictionary.hh>
#include <object/list.hh>
#include <object/string.hh>
#include <object/symbols.hh>

namespace object
{
  Dictionary::Dictionary()
  {
    proto_add(proto ? proto : object_class);
  }

  Dictionary::Dictionary(const value_type& value)
    : content_(value)
  {
    proto_add(proto);
  }

  Dictionary::Dictionary(rDictionary model)
    : content_(model->content_)
  {
    proto_add(proto);
  }

  const Dictionary::value_type&
  Dictionary::value_get() const
  {
    return content_;
  }

  Dictionary::value_type&
  Dictionary::value_get()
  {
    return content_;
  }

  rDictionary
  Dictionary::set(libport::Symbol key, rObject val)
  {
    content_[key] = val;
    return this;
  }

  rObject
  Dictionary::get(libport::Symbol key)
  {
    return libport::mhas(content_, key) ? content_[key] : rObject();
  }

  rDictionary
  Dictionary::clear()
  {
    content_.clear();
    return this;
  }

  bool
  Dictionary::empty()
  {
    return content_.empty();
  }

  rDictionary
  Dictionary::erase(libport::Symbol key)
  {
    content_.erase(key);
    return this;
  }

  rList
  Dictionary::keys()
  {
    List::value_type res;
    typedef const std::pair<libport::Symbol, rObject> elt_type;
    foreach (elt_type& elt, content_)
      res.push_back(new String(elt.first));
    return new List(res);
  }

  bool
  Dictionary::has(libport::Symbol key)
  {
    return libport::mhas(content_, key);
  }

  URBI_CXX_OBJECT_REGISTER(Dictionary);

  void
  Dictionary::initialize(CxxObject::Binder<Dictionary>& bind)
  {
    bind(SYMBOL(clear), &Dictionary::clear);
    bind(SYMBOL(empty), &Dictionary::empty);
    bind(SYMBOL(erase), &Dictionary::erase);
    bind(SYMBOL(get),   &Dictionary::get);
    bind(SYMBOL(has),   &Dictionary::has);
    bind(SYMBOL(keys),  &Dictionary::keys);
    bind(SYMBOL(set),   &Dictionary::set);
  }

  rObject
  Dictionary::proto_make()
  {
    return new Dictionary();
  }
}
