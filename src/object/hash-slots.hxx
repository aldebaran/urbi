#include <libport/containers.hh>

#include "object/object.hh"

namespace object
{
  bool HashSlots::set(const key_type& key, value_type v)
  {
    std::pair<iterator, bool> where = content_.insert(std::make_pair(key, v));
    if (!where.second)
      return false;
    return true;
  }

  void HashSlots::update(const key_type& key, value_type v)
  {
    content_[key] = v;
  }

  HashSlots::value_type HashSlots::get(const key_type& key) const
  {
    const_iterator where = content_.find(key);
    if (where == content_.end())
      return 0;
    return where->second;
  }

  void HashSlots::erase(const key_type& key)
  {
    content_.erase(key);
  }

  bool HashSlots::has(const key_type& key) const
  {
    return libport::mhas(content_, key);
  }

  const HashSlots::content_type& HashSlots::container() const
  {
    return content_;
  }
}
