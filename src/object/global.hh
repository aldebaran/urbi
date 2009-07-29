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

  /// Default formatting function for objects.
  std::string global_format(const rObject, const std::string& str, rFormatInfo finfo);

  /// Initialize the Global class.
  void global_class_initialize ();
}; // namespace object

# define CAPTURE_GLOBAL(Name)					\
  static ::object::rObject Name =				\
    ::object::global_class->slot_get(::libport::Symbol(#Name))

#endif // !OBJECT_GLOBAL_CLASS_HH
