#ifndef OBJECT_SLOT_HXX
# define OBJECT_SLOT_HXX

# include <object/cxx-conversions.hh>

namespace object
{
  inline
  Slot::Slot()
    : value_(object::void_class)
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
}

#endif
