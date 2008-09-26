#ifndef RUNNER_RAISE_HH
# define RUNNER_RAISE_HH

# include <libport/compilation.hh>
# include <libport/compiler.hh>

# include <object/fwd.hh>

namespace runner
{
  LIBPORT_SPEED_INLINE
  ATTRIBUTE_NORETURN
  void
  raise(const object::rObject& exn);

  LIBPORT_SPEED_INLINE
  ATTRIBUTE_NORETURN
  void
  raise_arity_error(unsigned effective,
                    unsigned expected);

  LIBPORT_SPEED_INLINE
  ATTRIBUTE_NORETURN
  void
  raise_arity_error(unsigned effective,
                    unsigned minimum,
                    unsigned maximum);

  LIBPORT_SPEED_INLINE
  ATTRIBUTE_NORETURN
  void
  raise_lookup_error(libport::Symbol msg,
                     const object::rObject& obj);

  LIBPORT_SPEED_INLINE
  ATTRIBUTE_NORETURN
  void
  raise_argument_type_error(unsigned idx,
                            object::rObject effective,
                            object::rObject expected);

}

# ifdef LIBPORT_SPEED
#  include <runner/raise.hxx>
# endif

#endif
