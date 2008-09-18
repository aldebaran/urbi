#ifndef RUNNER_RAISE_HH
# define RUNNER_RAISE_HH

# include <object/fwd.hh>

namespace runner
{
  inline void
  raise(const object::rObject& exn) __attribute__ ((noreturn));

  inline void
  raise_lookup_error(libport::Symbol msg,
                     const object::rObject& obj) __attribute__ ((noreturn));
}

# include <runner/raise.hxx>

#endif
