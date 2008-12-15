/**
 ** \file object/global-class.hh
 ** \brief Definition of the URBI object global.
 */

#ifndef OBJECT_GLOBAL_CLASS_HH
# define OBJECT_GLOBAL_CLASS_HH

# include <libport/symbol.hh>

# include <object/fwd.hh>
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
