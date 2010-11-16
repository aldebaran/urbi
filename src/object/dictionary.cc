/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/dictionary.cc
 ** \brief Creation of the Urbi object dictionary.
 */

#include <libport/containers.hh>

#include <kernel/userver.hh>
#include <object/symbols.hh>
#include <runner/runner.hh>
#include <urbi/object/dictionary.hh>
#include <urbi/object/list.hh>
#include <urbi/object/string.hh>

namespace urbi
{
  namespace object
  {
    std::size_t
    unordered_map_hash::operator()(rObject val) const
    {
      return hash_value(*val);
    }

    bool
    unordered_map_equal_to::operator()(rObject lhs, rObject rhs) const
    {
      bool res = from_urbi<bool>(lhs->call("==", rhs));
      // std::cerr << "equal_to() => " << res << std::endl;
      return res;
    }

    Dictionary::Dictionary()
      : content_()
    {
      proto_add(proto ? rObject(proto) : Object::proto);
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

    URBI_CXX_OBJECT_INIT(Dictionary)
    {
      bind(SYMBOL(asBool), &Dictionary::as_bool);

#define DECLARE(Name)                       \
      bind(SYMBOL(Name), &Dictionary::Name)

      DECLARE(clear);
      DECLARE(empty);
      DECLARE(erase);
      DECLARE(get);
      DECLARE(has);
      DECLARE(keys);
      DECLARE(set);
      DECLARE(size);

#undef DECLARE
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

    void
    Dictionary::key_check(rObject key) const
    {
      if (!libport::mhas(content_, key))
      {
        static rObject exn = slot_get(SYMBOL(KeyError));
        ::kernel::runner().raise(exn->call("new", key));
      }
    }

    rDictionary
    Dictionary::set(rObject key, rObject val)
    {
      content_[key] = val;
      changed();
      return this;
    }

    rObject
    Dictionary::get(rObject key)
    {
      key_check(key);
      return content_[key];
      unreachable(); // wtf?
    }

    rDictionary
    Dictionary::clear()
    {
      if (!empty())
      {
        content_.clear();
        changed();
      }

      return this;
    }

    bool
    Dictionary::empty() const
    {
      return content_.empty();
    }

    size_t
    Dictionary::size() const
    {
      return content_.size();
    }

    bool
    Dictionary::as_bool() const
    {
      return !empty();
    }

    rDictionary
    Dictionary::erase(rObject key)
    {
      key_check(key);
      content_.erase(key);
      changed();
      return this;
    }

    rList
    Dictionary::keys()
    {
      List::value_type res;
      typedef const std::pair<rObject, rObject> elt_type;
      foreach (elt_type& elt, content_)
        res.push_back(elt.first);
      return new List(res);
    }

    bool
    Dictionary::has(rObject key) const
    {
      return libport::mhas(content_, key);
    }
  }
}
