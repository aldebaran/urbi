#ifndef RUNNER_RAISE_HH
# define RUNNER_RAISE_HH

# include <libport/compiler.hh>
# include <libport/ufloat.hh>

# include <object/fwd.hh>

namespace runner
{
  /// Raise an URBI exception designed by its name. A lookup will be
  /// performed in the "Global" object.  If arg1 is
  /// "raise_current_method", the innermost method name will be looked
  /// up in the current runner and used instead.
  ATTRIBUTE_NORETURN
  void raise_urbi(libport::Symbol exn_name,
		  object::rObject arg1 = 0,
		  object::rObject arg2 = 0,
		  object::rObject arg3 = 0,
		  object::rObject arg4 = 0);

  extern const object::rObject& raise_current_method;

  ATTRIBUTE_NORETURN
  void raise_arity_error(unsigned effective, unsigned expected);

  ATTRIBUTE_NORETURN
  void raise_arity_error(unsigned effective,
			 unsigned minimum,
			 unsigned maximum);

  ATTRIBUTE_NORETURN
  void raise_bad_integer_error(libport::ufloat effective,
			       const std::string = "expected integer, got %s");

  ATTRIBUTE_NORETURN
  void raise_lookup_error(libport::Symbol msg, const object::rObject& obj);

  ATTRIBUTE_NORETURN
  void raise_primitive_error(const std::string message);

  ATTRIBUTE_NORETURN
  void raise_argument_type_error
    (unsigned idx,
     object::rObject effective,
     object::rObject expected,
     object::rObject method_name = raise_current_method);
}

#endif
