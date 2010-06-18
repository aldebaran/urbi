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
    Dictionary::Dictionary()
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
      if (libport::mhas(content_, key))
        return content_[key];
      else
      {
        static rObject exn = slot_get(SYMBOL(KeyError));
        ::kernel::runner().raise(exn->call("new", new String(key)));
      }
      unreachable();
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


    void
    Dictionary::initialize(CxxObject::Binder<Dictionary>& bind)
    {
      bind(SYMBOL(asBool), &Dictionary::as_bool);
#define DECLARE(Name)                           \
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

    URBI_CXX_OBJECT_REGISTER(Dictionary)
    {}
  }
}
