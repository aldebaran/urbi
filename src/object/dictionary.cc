/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
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
    proto_add(proto ? proto : Object::proto);
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
  Dictionary::empty() const
  {
    return content_.empty();
  }

  bool
  Dictionary::as_bool() const
  {
    return !empty();
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
  Dictionary::has(libport::Symbol key) const
  {
    return libport::mhas(content_, key);
  }

  URBI_CXX_OBJECT_REGISTER(Dictionary);

  void
  Dictionary::initialize(CxxObject::Binder<Dictionary>& bind)
  {
    bind(SYMBOL(asBool), &Dictionary::as_bool);
#define DECLARE(Name)                      \
    bind(SYMBOL(Name), &Dictionary::Name);

    DECLARE(clear);
    DECLARE(empty);
    DECLARE(erase);
    DECLARE(get);
    DECLARE(has);
    DECLARE(keys);
    DECLARE(set);
#undef DECLARE
  }

  rObject
  Dictionary::proto_make()
  {
    return new Dictionary();
  }
}
