#ifndef RUNNER_RAISE_HH
# define RUNNER_RAISE_HH

# include <libport/compiler.hh>

# include <object/fwd.hh>

namespace runner
{
  ATTRIBUTE_NORETURN
  void raise_urbi(const object::rObject& exn,
		  object::rObject arg1,
		  object::rObject arg2,
		  object::rObject arg3 = 0,
		  object::rObject arg4 = 0);

  ATTRIBUTE_NORETURN
  void raise_arity_error(unsigned effective, unsigned expected);

  ATTRIBUTE_NORETURN
  void raise_arity_error(unsigned effective,
			 unsigned minimum,
			 unsigned maximum);

  ATTRIBUTE_NORETURN
  void raise_lookup_error(libport::Symbol msg, const object::rObject& obj);

  ATTRIBUTE_NORETURN
  void raise_argument_type_error(unsigned idx,
				 object::rObject effective,
				 object::rObject expected);
}

#endif
