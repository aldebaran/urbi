/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/// \file urbi/uobject.hxx

#ifndef URBI_UOBJECT_HXX
# define URBI_UOBJECT_HXX

namespace urbi
{

  /*----------.
  | UObject.  |
  `----------*/

  inline
  int
  UObject::update()
  {
    return 0;
  }

  inline
  int
  UObject::voidfun()
  {
    return 0;
  }

  inline
  void
  UObject::clean()
  {
    assert(impl_);
    impl_->clean();
  }

  inline
  void
  UObject::USetUpdate(ufloat period)
  {
    assert(impl_);
    impl_->setUpdate(period);
  }

  inline
  void
  UObject::USync(UVar &v)
  {
    v.keepSynchronized();
  }

}

#endif // !URBI_UOBJECT_HXX
