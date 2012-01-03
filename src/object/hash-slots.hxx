/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/containers.hh>

#include <urbi/object/object.hh>

namespace object
{
  inline bool
  HashSlots::set(const key_type& key, value_type v)
  {
    std::pair<iterator, bool> where = content_.insert(std::make_pair(key, v));
    if (!where.second)
      return false;
    return true;
  }

  inline void
  HashSlots::update(const key_type& key, value_type v)
  {
    content_[key] = v;
  }

  inline HashSlots::value_type
  HashSlots::get(const key_type& key) const
  {
    const_iterator where = content_.find(key);
    if (where == content_.end())
      return 0;
    return where->second;
  }

  inline void
  HashSlots::erase(const key_type& key)
  {
    content_.erase(key);
  }

  inline bool
  HashSlots::has(const key_type& key) const
  {
    return libport::mhas(content_, key);
  }

  inline HashSlots::iterator
  HashSlots::begin()
  {
    return content_.begin();
  }

  inline HashSlots::iterator
  HashSlots::end()
  {
    return content_.end();
  }

  inline HashSlots::const_iterator
  HashSlots::begin() const
  {
    return content_.begin();
  }

  inline HashSlots::const_iterator
  HashSlots::end() const
  {
    return content_.end();
  }
}
