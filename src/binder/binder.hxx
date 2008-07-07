/**
 ** \file binder/binder.hxx
 ** \brief Inline implementation of binder::Binder.
 */

#ifndef BINDER_BINDER_HXX
# define BINDER_BINDER_HXX

namespace binder
{

  inline
  ast::Error&
  Binder::errors_get()
  {
    return errors_;
  }

} // namespace binder

#endif // !BINDER_BINDER_HXX
