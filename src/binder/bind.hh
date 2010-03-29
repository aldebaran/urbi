/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file binder/bind.hh
 ** \brief Definition of binder::bind().
 */

#ifndef BINDER_BIND_HH
# define BINDER_BIND_HH

# include <ast/nary-fwd.hh>
# include <urbi/export.hh>

namespace binder
{

  /// Bind names in \a a.
  template <typename T>
  URBI_SDK_API
  libport::intrusive_ptr<T> bind(libport::intrusive_ptr<T> a);

} // namespace binder

#endif // !BINDER_BINDER_HH
