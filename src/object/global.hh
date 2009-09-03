/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/**
 ** \file object/global-class.hh
 ** \brief Definition of the URBI object global.
 */

#ifndef OBJECT_GLOBAL_CLASS_HH
# define OBJECT_GLOBAL_CLASS_HH

# include <libport/symbol.hh>

# include <object/fwd.hh>
# include <object/object.hh>
# include <object/symbols.hh>
# include <urbi/export.hh>

namespace object
{
  /// The prototype for Global objects.
  extern URBI_SDK_API rObject global_class;

  /// Initialize the Global class.
  void global_class_initialize ();
}; // namespace object

# define CAPTURE_GLOBAL(Name)					\
  static ::object::rObject Name =				\
    ::object::global_class->slot_get(::libport::Symbol(#Name))

#endif // !OBJECT_GLOBAL_CLASS_HH
