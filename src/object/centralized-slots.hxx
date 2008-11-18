#ifndef OBJECT_CENTRALIZED_SLOTS_HXX
# define OBJECT_CENTRALIZED_SLOTS_HXX

# include <libport/containers.hh>
# include <libport/foreach.hh>
# include <iostream>

# include <object/object.hh>

namespace object
{

  inline Object*
  CentralizedSlots::get_owner(const q_slot_type& slot)
  {
    return slot.first.first;
  }

  static inline CentralizedSlots::q_slot_type
    content(Object* owner, libport::Symbol key, rObject& value)
  {
    return CentralizedSlots::q_slot_type(
      CentralizedSlots::location_type(owner, key),
      value);
  }

  inline bool
  CentralizedSlots::set(Object* owner, const key_type& key, value_type v)
  {
    if (has(owner, key))
      return false;
    content_->insert(content(owner, key, v));
    return true;
  }

  inline void
  CentralizedSlots::update(Object* owner, const key_type& key, value_type v)
  {
    loc_index_type::iterator it = where(owner, key);
    if (it == content_->end())
      content_->insert(content(owner, key, v));
    else
      loc_index_.replace(it, content(owner, key, v));
  }

  inline CentralizedSlots::value_type
  CentralizedSlots::get(const Object* owner, const key_type& key)
  {
    loc_index_type::iterator it = where(owner, key);
    if (it == content_->end())
      return 0;
    return it->second;
  }

  inline void
  CentralizedSlots::erase(Object* owner, const key_type& key)
  {
    loc_index_type::iterator it = where(owner, key);
    if (it != content_->end())
      loc_index_.erase(it);
  }

  inline bool
  CentralizedSlots::has(Object* owner, const key_type& key)
  {
    loc_index_type::iterator it = where(owner, key);
    return it != content_->end();
  }

  inline CentralizedSlots::loc_index_type::iterator
  CentralizedSlots::where(const Object* owner, const key_type& key)
  {
    return loc_index_.find(location_type(const_cast<Object*>(owner), key));
  }

  inline void
  CentralizedSlots::finalize(Object* owner)
  {
    obj_index_.erase(owner);
  }

  inline CentralizedSlots::iterator
  CentralizedSlots::begin(Object* owner)
  {
    return obj_index_.equal_range(owner).first;
  }

  inline CentralizedSlots::iterator
  CentralizedSlots::end(Object* owner)
  {
    return obj_index_.equal_range(owner).second;
  }

  inline CentralizedSlots::const_iterator
  CentralizedSlots::begin(const Object* owner)
  {
    return obj_index_.equal_range(const_cast<Object*>(owner)).first;
  }

  inline CentralizedSlots::const_iterator
  CentralizedSlots::end(const Object* owner)
  {
    return obj_index_.equal_range(const_cast<Object*>(owner)).second;
  }

}

#endif
