#ifndef OBJECT_SLOT_HXX
# define OBJECT_SLOT_HXX

# include <object/cxx-conversions.hh>
# include <object/symbols.hh>
# include <runner/raise.hh>

namespace object
{
  inline
  Slot::Slot()
    : value_(object::void_class)
    , constant_(false)
  {}

  inline
  Slot::Slot(const Slot& model)
    : libport::RefCounted()
    , value_(model.value_)
    , constant_(false)
    , properties_(model.properties_)
  {}

  template <typename T>
  inline
  Slot::Slot(const T& value)
    : value_(0)
    , constant_(false)
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
    if (constant_)
      runner::raise_const_error();
    value_ = object::CxxConvert<T>::from(value);
    if (changed_)
      changed_->call(SYMBOL(emit));
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
}

#endif
