#ifndef RAISE_HXX
# define RAISE_HXX

# include <scheduler/scheduler.hh>
# include <kernel/userver.hh>
# include <object/global.hh>

namespace runner
{
  inline void
  raise(const object::rObject& exn)
  {
    ::urbiserver->getCurrentRunner().raise(exn);
  }

  inline void
  raise_lookup_error(libport::Symbol msg, const object::rObject& obj)
  {
    runner::Runner& r = ::urbiserver->getCurrentRunner();
    object::rObject exn =
      urbi_call(r,
                object::global_class->slot_get(SYMBOL(LookupError)),
                SYMBOL(new), new object::String(msg), obj);
    r.raise(exn);
    pabort("Unreachable");
  }

}

#endif
