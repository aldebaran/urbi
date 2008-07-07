/**
 ** \file object/dictionary-class.cc
 ** \brief Creation of the URBI object dictionary.
 */

#include <libport/containers.hh>

#include <object/dictionary-class.hh>
#include <object/list-class.hh>

namespace object
{
  rObject dictionary_class;

  Dictionary::Dictionary()
  {
    proto_add(dictionary_class);
  }

  Dictionary::Dictionary(const value_type& value)
    : content_(value)
  {
    proto_add(dictionary_class);
  }

  Dictionary::Dictionary(rDictionary model)
    : content_(model->content_)
  {
    proto_add(dictionary_class);
  }

  const Dictionary::value_type& Dictionary::value_get() const
  {
    return content_;
  }

  Dictionary::value_type& Dictionary::value_get()
  {
    return content_;
  }

  rDictionary Dictionary::set(const libport::Symbol& key, rObject val)
  {
    content_[key] = val;
    return this;
  }

  rObject Dictionary::get(const libport::Symbol& key)
  {
    return libport::mhas(content_, key) ? content_[key] : rObject();
  }

  rDictionary Dictionary::clear()
  {
    content_.clear();
    return this;
  }

  rDictionary
  Dictionary::erase(libport::Symbol key)
  {
    content_.erase(key);
    return this;
  }

  rList Dictionary::keys()
  {
    List::value_type res;
    typedef const std::pair<libport::Symbol, rObject> elt_type;
    foreach (elt_type& elt, content_)
      res.push_back(new String(elt.first));
    return new List(res);
  }

  bool Dictionary::has(const libport::Symbol& key)
  {
    return libport::mhas(content_, key);
  }

  std::string Dictionary::type_name_get() const
  {
    return type_name;
  }

  void Dictionary::initialize(CxxObject::Binder<Dictionary>& bind)
  {
    bind(SYMBOL(clear), &Dictionary::clear);
    bind(SYMBOL(erase), &Dictionary::erase);
    bind(SYMBOL(get), &Dictionary::get);
    bind(SYMBOL(has), &Dictionary::has);
    bind(SYMBOL(keys), &Dictionary::keys);
    bind(SYMBOL(set), &Dictionary::set);
  }

  bool Dictionary::dictionary_added =
    CxxObject::add<Dictionary>("Dictionary", dictionary_class);
  const std::string Dictionary::type_name = "Dictionary";
}
