/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
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
#include <urbi/object/dictionary.hh>
#include <urbi/object/event.hh>
#include <urbi/object/list.hh>
#include <urbi/object/string.hh>

#include <runner/job.hh>

#include <eval/raise.hh>

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
      return res;
    }

    Dictionary::Dictionary()
      : content_()
      , elementAdded_(0)
      , elementChanged_(0)
      , elementRemoved_(0)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Dictionary::Dictionary(const value_type& value)
      : content_(value)
      , elementAdded_(0)
      , elementChanged_(0)
      , elementRemoved_(0)
    {
      proto_add(proto);
    }

    Dictionary::Dictionary(rDictionary model)
      : content_(model->content_)
      , elementAdded_(0)
      , elementChanged_(0)
      , elementRemoved_(0)
    {
      proto_add(proto);
    }

    URBI_CXX_OBJECT_INIT(Dictionary)
      : content_()
      , elementAdded_(0)
      , elementChanged_(0)
      , elementRemoved_(0)
    {
      BIND(asBool, as_bool);
      BIND(clear);
      BIND(elementAdded, elementAdded_get);
      BIND(elementChanged, elementChanged_get);
      BIND(elementRemoved, elementRemoved_get);
      BIND(empty);
      BIND(erase);
      BIND(get);
      BIND(has);
      BIND(keys);
      BIND(set);
      BIND(size);
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
      if (!libport::has(content_, key))
      {
        static rObject exn = slot_get(SYMBOL(KeyError));
        eval::raise(::kernel::runner(), exn->call("new", key));
      }
    }

    rDictionary
    Dictionary::set(rObject key, rObject val)
    {
      value_type::iterator it = content_.find(key);
      if (it == content_.end())
      {
        content_[key] = val;
        elementAdded();
      }
      else
      {
        it->second = val;
        elementChanged();
      }
      return this;
    }

    rObject
    Dictionary::get(rObject key)
    {
      URBI_AT_HOOK(elementChanged);
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
        elementRemoved();
      }
      return this;
    }

    bool
    Dictionary::empty() const
    {
      bool res = content_.empty();
      if (res)
        URBI_AT_HOOK(elementAdded);
      else
        URBI_AT_HOOK(elementRemoved);
      return res;
    }

    size_t
    Dictionary::size() const
    {
      URBI_AT_HOOK(elementAdded);
      URBI_AT_HOOK(elementRemoved);
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
      elementRemoved();
      return this;
    }

    rList
    Dictionary::keys()
    {
      URBI_AT_HOOK(elementRemoved);
      URBI_AT_HOOK(elementAdded);
      List::value_type res;
      typedef const std::pair<rObject, rObject> elt_type;
      foreach (elt_type& elt, content_)
        res.push_back(elt.first);
      return new List(res);
    }

    bool
    Dictionary::has(rObject key) const
    {
      bool res = libport::has(content_, key);
      if (res)
        URBI_AT_HOOK(elementRemoved);
      else
        URBI_AT_HOOK(elementAdded);
      return res;
    }

    /*
      SYMBOL(elementAdded)
      SYMBOL(elementChanged)
      SYMBOL(elementRemoved)
    */
    URBI_ATTRIBUTE_ON_DEMAND_IMPL(Dictionary, Event, elementAdded);
    URBI_ATTRIBUTE_ON_DEMAND_IMPL(Dictionary, Event, elementChanged);
    URBI_ATTRIBUTE_ON_DEMAND_IMPL(Dictionary, Event, elementRemoved);
  }
}
