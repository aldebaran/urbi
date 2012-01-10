/*
 * Copyright (C) 2009, 2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef RUNNER_RAISE_HH
# define RUNNER_RAISE_HH

# include <boost/optional.hpp>

# include <libport/compiler.hh>
# include <libport/ufloat.hh>

# include <urbi/object/fwd.hh>
# include <urbi/parser/location.hh>

// Do not leak yy::loc in K2 headers.
namespace yy
{
  class location;
}

namespace ast
{
  typedef yy::location loc;
}

namespace runner
{

  class RaiseCurrent {};
  extern RaiseCurrent raise_current_method;

  /// Raise an Urbi exception denoted by its name, looked up in
  /// "Global.Exception".  If "raise_current" is passed, the
  /// innermost method name will be looked up in the current runner
  /// and used as first argument (inserted before arg1..arg4).
  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_urbi(libport::Symbol exn_name,
                  RaiseCurrent,
		  object::rObject arg1 = 0,
		  object::rObject arg2 = 0,
		  object::rObject arg3 = 0,
                  bool skip = false,
                  const boost::optional<ast::loc>& loc
                  = boost::optional<ast::loc>());

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_urbi(libport::Symbol exn_name,
		  object::rObject arg1 = 0,
		  object::rObject arg2 = 0,
		  object::rObject arg3 = 0,
		  object::rObject arg4 = 0,
                  bool skip = false,
                  const boost::optional<ast::loc>& loc
                  = boost::optional<ast::loc>());

  /// Like raise_urbi, but skip the last callstack element.
  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_urbi_skip(libport::Symbol exn_name,
                       RaiseCurrent,
                       object::rObject arg1 = 0,
                       object::rObject arg2 = 0,
                       object::rObject arg3 = 0);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_urbi_skip(libport::Symbol exn_name,
                       object::rObject arg1 = 0,
                       object::rObject arg2 = 0,
                       object::rObject arg3 = 0,
                       object::rObject arg4 = 0,
                       const boost::optional<ast::loc>& loc
                       = boost::optional<ast::loc>());

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_argument_error(unsigned idx, const object::rObject& exn,
                            object::rObject method_name = 0);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_argument_type_error(unsigned idx,
                                 object::rObject effective,
                                 object::rObject expected,
                                 object::rObject method_name = 0);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_arity_error(unsigned effective, unsigned expected);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_arity_error(unsigned effective,
			 unsigned minimum,
			 unsigned maximum);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_bad_integer_error(libport::ufloat effective,
			       const std::string& msg
                               = "expected integer, got %s");

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_const_error();

  /// \param deep
  ///   whether error correction should consider slotNames,
  ///   instead of localSlotNames.
  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_lookup_error(libport::Symbol msg,
                          const object::rObject& obj,
                          bool deep = true);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_negative_number_error(libport::ufloat effective);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_non_positive_number_error(libport::ufloat effective);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_primitive_error(const std::string& message);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_scheduling_error(const std::string& message);

  /// \param error  whether an error, or a warning.
  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_syntax_error(const ast::loc& location,
                          const std::string& message,
                          const std::string& input,
                          bool error = true);

  ATTRIBUTE_NORETURN
  URBI_SDK_API
  void raise_type_error(object::rObject effective,
                        object::rObject expected,
                        const boost::optional<ast::loc>& loc
                        = boost::optional<ast::loc>());

  URBI_SDK_API
  void raise_unexpected_void_error();
}

#endif
