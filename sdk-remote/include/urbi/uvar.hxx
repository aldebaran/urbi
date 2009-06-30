/// \file urbi/uvar.hxx

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008, 2009 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

#ifndef URBI_UVAR_HXX
# define URBI_UVAR_HXX

# include <urbi/uvar.hh>

namespace urbi
{
  inline
  UVar::operator bool() const
  {
    return static_cast<int>(*this) != 0;
  }

  inline
  UVar::UVar()
    : owned(false)
    , VAR_PROP_INIT
    , impl_(0)
    , vardata(0)
    , name("noname")
  {
  }

  inline
  void
  UVar::setOwned()
  {
    impl_->setOwned();
  }

  inline
  UDataType
  UVar::type() const
  {
    return impl_->type();
  }

  inline
  void
  UVar::syncValue()
  {
    impl_->sync();
  }

  inline const UValue&
  UVar::val() const
  {
      return impl_->get();
  }
  inline
  void
  UVar::keepSynchronized()
  {
    impl_->keepSynchronized();
  }
  inline
  UVar&
  UVar::operator=(const UValue& v)
  {
    impl_->set(v);
    return *this;
  }
  #define SET(type)               \
  inline UVar& UVar::operator=(type tv)    \
  {                              \
    UValue v(tv, false);       \
    impl_->set(v);               \
    return *this;                \
  }

  SET(ufloat)
  SET(const std::string&)
  SET(const UBinary&)
  SET(const UImage&)
  SET(const USound&)
  SET(const UList&)
  #undef SET

 #define GET(type)                    \
  inline UVar::operator type() const  \
  {                                   \
    return impl_->get();              \
  }

  GET(int)
  GET(const UBinary&)
  GET(UImage)
  GET(USound)
  GET(ufloat)
  GET(std::string)
  GET(UList)
  #undef GET
  inline UVar::operator UBinary*() const
  {
    return new UBinary((const UBinary&)*this);
  }

  inline void UVar::setProp(UProperty prop, const UValue &v)
  {
    impl_->setProp(prop, v);
  }

  inline void UVar::setProp(UProperty prop, const char* v)
  {
    UValue tv(v);
    impl_->setProp(prop, tv);
  }

  inline void UVar::setProp(UProperty prop, ufloat v)
  {
    UValue tv(v);
    impl_->setProp(prop, tv);
  }

  inline bool
  UVar::setBypass(bool enable)
  {
    return impl_->setBypass(enable);
  }
  inline UValue
  UVar::getProp(UProperty prop)
  {
    return impl_->getProp(prop);
  }
} // end namespace urbi

#endif // ! URBI_UVAR_HXX
