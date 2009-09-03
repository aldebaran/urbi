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
 ** \file object/system-class.hh
 ** \brief Definition of the URBI object system.
 */

#ifndef OBJECT_SYSTEM_CLASS_HH
# define OBJECT_SYSTEM_CLASS_HH

# include <object/fwd.hh>
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
  rObject execute_parsed(parser::parse_result_type p,
                         libport::Symbol fun, std::string e);
  /// Set the current script name
  void system_set_program_name(const std::string& name);
  /// Register a new user-argument for the script
  void system_push_argument(const std::string& arg);

  /// Initialize the System class.
  void system_class_initialize();
}; // namespace object

#endif // !OBJECT_SYSTEM_CLASS_HH
