/**
 ** \file object/dictionary-class.cc
 ** \brief Creation of the URBI object dictionary.
 */

#include <libport/containers.hh>

#include <object/dictionary-class.hh>

namespace object
{
  rObject dictionary_class;

  Dictionary::Dictionary()
  {}

  Dictionary::Dictionary(const value_type& value)
    : content_(value)
  {}

  Dictionary::Dictionary(rDictionary model)
    : content_(model->content_)
  {}

  const Dictionary::value_type& Dictionary::value_get() const
  {
    return content_;
  }

  Dictionary::value_type& Dictionary::value_get()
  {
    return content_;
  }

  rDictionary Dictionary::set(rString key, rObject val)
  {
    content_[key->value_get()] = val;
    return this;
  }

  rObject Dictionary::get(rString key)
  {
    return libport::mhas(content_, key->value_get()) ?
      content_[key->value_get()] : void_class;
  }

  rDictionary Dictionary::clear()
  {
    content_.clear();
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

  rObject Dictionary::has(rString key)
  {
    return libport::mhas(content_, key->value_get()) ?
      true_class : false_class;
  }

  std::string Dictionary::type_name_get() const
  {
    return type_name;
  }

  void Dictionary::initialize(CxxObject::Binder<Dictionary>& bind)
  {
    bind(SYMBOL(set), &Dictionary::set);
    bind(SYMBOL(get), &Dictionary::get);
    bind(SYMBOL(has), &Dictionary::has);
    bind(SYMBOL(clear), &Dictionary::clear);
    bind(SYMBOL(keys), &Dictionary::keys);
  }

  bool Dictionary::dictionary_added =
    CxxObject::add<Dictionary>("Dictionary", dictionary_class);
  const std::string Dictionary::type_name = "Dictionary";
}
