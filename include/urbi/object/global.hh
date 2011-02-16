/*
 * Copyright (C) 2009, 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/global.hh
 ** \brief Definition of the Urbi object global.
 */

#ifndef OBJECT_GLOBAL_HH
# define OBJECT_GLOBAL_HH

# include <libport/debug.hh>
# include <libport/symbol.hh>

# include <urbi/object/fwd.hh>
# include <urbi/object/object.hh>
# include <urbi/export.hh>

namespace urbi
{
  namespace object
  {
    /// The prototype for Global objects.
    extern URBI_SDK_API rObject global_class;

    /// Initialize the Global class.
    void global_class_initialize();

    URBI_SDK_API
    rObject capture(libport::Symbol name, const rObject& from);
  }; // namespace object
}

# define CAPTURE_(Name, From)                                           \
  static ::urbi::object::rObject Name =                                 \
    ::urbi::object::capture(libport::Symbol(#Name), From)

# define CAPTURE_GLOBAL(Name)                           \
  CAPTURE_(Name, ::urbi::object::global_class)

# define CAPTURE_GLOBAL2(Name, Member)          \
  CAPTURE_GLOBAL(Name);                         \
  CAPTURE_(Member, Name)

#endif // !OBJECT_GLOBAL_HH
