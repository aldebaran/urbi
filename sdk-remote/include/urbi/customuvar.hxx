/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_CUSTOMUVAR_HXX
# define URBI_CUSTOMUVAR_HXX

namespace urbi
{
  template<typename T>
  CustomUVar<T>::CustomUVar()
  {
  }

  template<typename T>
  CustomUVar<T>::CustomUVar(const std::string& name, impl::UContextImpl* impl)
  : UVar(name, impl)
  {}

  template<typename T>
  CustomUVar<T>::CustomUVar(const std::string& a, const std::string& b,
                            impl::UContextImpl* c)
  : UVar(a, b, c)
  {}

  template<typename T>
  CustomUVar<T>::CustomUVar(UObject& a, const std::string& b,
                            impl::UContextImpl* c)
  : UVar(a, b, c)
  {}

  template<typename T>
  T& CustomUVar<T>::data()
  {
    return data_;
  }

  template<typename T>
  T& CustomUVar<T>::data(UVar& v)
  {
    return static_cast<CustomUVar<T>&>(v).data();
  }

  template<typename T> void
  CustomUVar<T>::updateCache(UVar& v)
  {
    v.fill(static_cast<CustomUVar<T>&>(v).data());
  }

  template<typename T>
  template<typename U>
  void CustomUVar<T>::operator = (U v)
  {
    this->UVar::operator = (v);
  }
}

#endif
