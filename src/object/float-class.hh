/**
 ** \file object/float-class.hh
 ** \brief Definition of the URBI object float.
 */

#ifndef OBJECT_FLOAT_CLASS_HH
# define OBJECT_FLOAT_CLASS_HH

# include <libport/ufloat.hh>

# include "object/fwd.hh"

namespace object
{
  extern rObject float_class;

  /// Initialize the Float class.
  void float_class_initialize ();

  /// Convert an ufloat to an integer, and raise BadInteger with primitive
  /// name func if the conversion went wrong.
  int ufloat_to_int (libport::ufloat val, const std::string& func);
}; // namespace object

#endif // !OBJECT_FLOAT_CLASS_HH
