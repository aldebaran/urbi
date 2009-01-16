#ifndef OBJECT_SLOT_HXX
# define OBJECT_SLOT_HXX

# include <object/cxx-conversions.hh>

namespace object
{
  inline
  Slot::Slot()
    : value_(object::void_class)
  {}

  inline
  Slot::Slot(const Slot& model)
    : libport::RefCounted()
    , value_(model.value_)
    , properties_(model.properties_)
  {}

  template <typename T>
  inline
  Slot::Slot(const T& value)
    : value_(0)
  {
    set(value);
  }

  template <typename T>
  inline T
  Slot::get()
  {
    return object::CxxConvert<T>::to(value_, 0);
  }

  template <typename T>
  inline void
  Slot::set(const T& value)
  {
    value_ = object::CxxConvert<T>::from(value);
  }

  template <typename T>
  inline const T&
  Slot::operator=(const T& value)
  {
    set(value);
    return value;
  }

  inline
  Slot::operator rObject ()
  {
    return get<rObject>();
  }

  inline
  Slot::operator bool ()
  {
    return value_.operator bool();
  }

  inline
  Object*
  Slot::operator->()
  {
    return assert_exp(value_.get());
  }

  inline
  const Object*
  Slot::operator->() const
  {
    return assert_exp(value_.get());
  }

  inline
  rObject
  Slot::value() const
  {
    return value_;
  }

  inline
  rObject
  Slot::property_get(libport::Symbol k)
  {
    properties_type::iterator it = properties_.find(k);
    if (it == properties_.end())
      return 0;
    else
      return it->second;
  }

  inline
  bool
  Slot::property_has(libport::Symbol k)
  {
    return properties_.find(k) != properties_.end();
  }

  inline
  bool
  Slot::property_set(libport::Symbol k, rObject value)
  {
    bool res = !property_has(k);
    properties_[k] = value;
    return res;
  }

  inline
  void
  Slot::property_remove(libport::Symbol k)
  {
    properties_.erase(k);
  }

  inline
  Slot::properties_type&
  Slot::properties_get()
  {
    return properties_;
  }

}

#endif
