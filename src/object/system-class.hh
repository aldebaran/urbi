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

  /// Parse, bind etc. and execute code.
  ///
  /// Used internally and from the debugging routines.
  ///
  /// \param r   runner that must be an Interpreter.
  /// \param p   the parser result.
  /// \param fun name of the caller.
  /// \param e   exception to raise on error.
  /// \returns the result of the evaluation.
  rObject
  execute_parsed (runner::Runner& r, parser::parse_result_type p,
                  libport::Symbol fun, UrbiException e);

  /// Initialize the System class.
  void system_class_initialize ();
}; // namespace object

#endif // !OBJECT_SYSTEM_CLASS_HH
