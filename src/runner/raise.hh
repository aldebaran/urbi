#ifndef RUNNER_RAISE_HH
# define RUNNER_RAISE_HH

# include <libport/compilation.hh>

# include <object/fwd.hh>

namespace runner
{
  LIBPORT_SPEED_INLINE
  void
  raise(const object::rObject& exn)
    __attribute__ ((noreturn));

  LIBPORT_SPEED_INLINE
  void
  raise_arity_error(unsigned effective,
                    unsigned expected)
    __attribute__ ((noreturn));

  LIBPORT_SPEED_INLINE
  void
  raise_arity_error(unsigned effective,
                    unsigned minimum,
                    unsigned maximum)
    __attribute__ ((noreturn));

  LIBPORT_SPEED_INLINE
  void
  raise_lookup_error(libport::Symbol msg,
                     const object::rObject& obj)
    __attribute__ ((noreturn));

  LIBPORT_SPEED_INLINE
  void
  raise_argument_type_error(unsigned idx,
                            object::rObject effective,
                            object::rObject expected)
    __attribute__ ((noreturn));

}

# ifdef LIBPORT_SPEED
#  include <runner/raise.hxx>
# endif

#endif
