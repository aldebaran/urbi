/*
 * Copyright (C) 2007, 2008, 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uvar.hxx
#ifndef URBI_UVAR_HXX
# define URBI_UVAR_HXX

# include <stdexcept>
# include <libport/cassert>
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
    , temp(false)
    , local(false)
  {
  }

  inline
  void
  UVar::init(const std::string& objname, const std::string& varname,
             impl::UContextImpl* ctx)
  {
    init(objname + '.' + varname, ctx);
  }


  inline
  void
  UVar::setOwned()
  {
    check();
    impl_->setOwned();
  }

  inline
  UDataType
  UVar::type() const
  {
    check();
    return impl_->type();
  }

  inline
  void
  UVar::syncValue()
  {
    check();
    impl_->sync();
  }

  inline const UValue&
  UVar::val() const
  {
    check();
    return impl_->get();
  }

  inline
  void
  UVar::keepSynchronized()
  {
    check();
    impl_->keepSynchronized();
  }

  inline
  UVar&
  UVar::operator=(const UValue& v)
  {
    check();
    impl_->set(v);
    return *this;
  }

# define SET(Type)                         \
  inline UVar& UVar::operator=(Type tv)    \
  {                                        \
    /*no need to copy, impl will do it*/   \
    check();                              \
    UValue v(tv, false);                   \
    impl_->set(v);                         \
    return *this;                          \
  }

  SET(const UBinary&)
  SET(const UImage&)
  SET(const UList&)
  SET(const UDictionary&)
  SET(const USound&)
  SET(const std::string&)
  SET(ufloat)
# undef SET

  template<typename T>
  UVar& UVar::operator = (const T& v)
  {
    check();
    UValue val;
    val, v;
    impl_->set(val);
    return *this;
  }

# define GET(Type)                    \
  inline UVar::operator Type() const  \
  {                                   \
    check();                         \
    return impl_->get();              \
  }

  GET(UImage)
  GET(UList)
  GET(UDictionary)
  GET(USound)
  GET(const UBinary&)
  GET(int)
  GET(std::string)
  GET(ufloat)
# undef GET

  inline UVar::operator UBinary*() const
  {
    return new UBinary(static_cast<const UBinary&>(*this));
  }

  inline void UVar::setProp(UProperty prop, const UValue &v)
  {
    check();
    impl_->setProp(prop, v);
  }

  inline void UVar::setProp(UProperty prop, const char* v)
  {
    check();
    UValue tv(v);
    impl_->setProp(prop, tv);
  }

  inline void UVar::setProp(UProperty prop, ufloat v)
  {
    check();
    UValue tv(v);
    impl_->setProp(prop, tv);
  }

  inline bool
  UVar::setBypass(bool enable)
  {
    check();
    return impl_->setBypass(enable);
  }

  inline UValue
  UVar::getProp(UProperty prop)
  {
    check();
    return impl_->getProp(prop);
  }

  inline void
  UVar::unnotify()
  {
    check();
    impl_->unnotify();
  }

  inline void
  UVar::check() const
  {
    if (!impl_)
      throw std::runtime_error("invalid use of unbound UVar");
  }

  inline void
  UVar::useRTP(bool state)
  {
    rtp = state?RTP_YES:RTP_NO;
    impl_->useRTP(state);
  }

  template<typename T>
  T UVar::as() const
  {
    return uvalue_cast<T>(const_cast<UValue&>(val()));
  }

  template<typename T>
  T UVar::as(T*) const
  {
     return uvalue_cast<T>(const_cast<UValue&>(val()));
  }

  template<typename T>
  T& UVar::fill(T& v) const
  {
    v = uvalue_cast<T>(const_cast<UValue&>(val()));
    return v;
  }

  template<typename T>
  bool
  UVar::operator == (const T& v) const
  {
    return uvalue_cast<T>(const_cast<UValue&>(val())) == v;
  }
} // end namespace urbi

#endif // ! URBI_UVAR_HXX
