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
    impl_->clean();
  }

  inline
  void
  UObject::USetUpdate(ufloat period)
  {
    impl_->setUpdate(period);
  }

  inline
  void
  UObject::USync(UVar &v)
  {
    v.keepSynchronized();
  }

  /*-------------.
  | Standalone.  |
  `-------------*/

}

#endif // !URBI_UOBJECT_HXX
