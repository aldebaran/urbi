#include <object/object.hh>
#include <object/sorted-vector-slots.hh>

namespace object
{
  bool
  SortedVectorSlots::set(const key_type& key, value_type v)
  {
    std::pair<bool, iterator> res = where(key);
    if (res.first)
      return false;
    content_.insert(res.second, std::make_pair(key, v));
    return true;
  }

  void
  SortedVectorSlots::update(const key_type& key, value_type v)
  {
    std::pair<bool, iterator> res = where(key);
    if (res.first)
      res.second->second = v;
    else
      content_.insert(res.second, std::make_pair(key, v));
  }

  SortedVectorSlots::value_type
  SortedVectorSlots::get(const key_type& key) const
  {
    std::pair<bool, iterator> res =
      const_cast<SortedVectorSlots*>(this)->where(key);
    if (res.first)
      return res.second->second;
    else
      return 0;
  }

  std::pair<bool, SortedVectorSlots::iterator>
  SortedVectorSlots::where(const key_type& key)
  {
    int size = content_.size();
    int start = 0;
    int end = size - 1;
    int middle = 0;

    while (start <= end)
    {
      middle = start + (end - start) / 2;
      if (content_[middle].first == key)
        return std::make_pair(true, content_.begin() + middle);
      if (content_[middle].first < key)
      {
        middle++;
        start = middle;
      }
      else
        end = middle - 1;
    }
    return std::make_pair(false, content_.begin() + middle);
  }

  void SortedVectorSlots::erase(const key_type& key)
  {
    std::pair<bool, iterator> res = where(key);
    if (res.first)
      content_.erase(res.second);
  }

  bool SortedVectorSlots::has(const key_type& key) const
  {
    return const_cast<SortedVectorSlots*>(this)->where(key).first;
  }

  const SortedVectorSlots::content_type&
  SortedVectorSlots::container() const
  {
    return content_;
  }
}
