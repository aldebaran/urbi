/**
 ** \file object/system-class.hh
 ** \brief Definition of the URBI object system.
 */

#ifndef OBJECT_SYSTEM_CLASS_HH
# define OBJECT_SYSTEM_CLASS_HH

# include <object/fwd.hh>
# include <object/urbi-exception.hh>
# include <parser/parse.hh>
# include <parser/parse-result.hh>

namespace object
{
  extern rObject system_class;

  /// Parse and execute code. Used internally and from the debugging routines.
  rObject execute_parsed
    (runner::Runner&, parser::parse_result_type, UrbiException);

  /// Initialize the System class.
  void system_class_initialize ();
}; // namespace object

#endif // !OBJECT_SYSTEM_CLASS_HH
