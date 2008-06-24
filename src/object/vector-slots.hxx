#include <object/object.hh>
#include <object/vector-slots.hh>

namespace object
{
  bool
  VectorSlots::set(const key_type& key, value_type v)
  {
    where(key);
    if (found_)
      return false;
    content_.push_back(std::make_pair(key, v));
    return true;
  }

  void
  VectorSlots::update(const key_type& key, value_type v)
  {
    where(key);
    if (found_)
      it_->second = v;
    else
      content_.push_back(std::make_pair(key, v));
  }

  VectorSlots::value_type
  VectorSlots::get(const key_type& key) const
  {
    where(key);
    return found_ ? it_->second : value_type(0);
  }

  void
  VectorSlots::where(const key_type& key) const
  {
    for (content_type::iterator it =
           const_cast<VectorSlots*>(this)->content_.begin();
         it != const_cast<VectorSlots*>(this)->content_.end(); ++it)
      if (it->first == key)
      {
        found_ = true;
        it_ = it;
        return;
      }
    found_ = false;
  }

  void
  VectorSlots::erase(const key_type& key)
  {
    where(key);
    if (found_)
      content_.erase(it_);
  }

  bool
  VectorSlots::has(const key_type& key) const
  {
    where(key);
    return found_;
  }

  const VectorSlots::content_type&
  VectorSlots::container() const
  {
    return content_;
  }

}
