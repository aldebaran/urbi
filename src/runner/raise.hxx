#ifndef RAISE_HXX
# define RAISE_HXX

# include <scheduler/scheduler.hh>
# include <kernel/userver.hh>
# include <object/global.hh>
# include <object/string.hh>
# include <runner/runner.hh>
# include <runner/call.hh>

namespace runner
{
  LIBPORT_SPEED_INLINE
  void
  raise(const object::rObject& exn)
  {
    ::urbiserver->getCurrentRunner().raise(exn);
    pabort("Unreachable");
  }

  LIBPORT_SPEED_INLINE
  void
  raise_lookup_error(libport::Symbol msg, const object::rObject& obj)
  {
    Runner& r = ::urbiserver->getCurrentRunner();
    object::rObject exn =
      urbi_call(r,
                object::global_class->slot_get(SYMBOL(LookupError)),
                SYMBOL(new), new object::String(msg), obj);
    r.raise(exn);
    pabort("Unreachable");
  }


  LIBPORT_SPEED_INLINE
  void
  raise_arity_error(unsigned effective,
                    unsigned expected)
  {
    Runner& r = ::urbiserver->getCurrentRunner();
    object::rObject exn =
      urbi_call(r, object::global_class->slot_get(SYMBOL(ArityError)),
                SYMBOL(new),
                new object::String(r.innermost_call_get()),
                new object::Float(effective),
                new object::Float(expected));
    r.raise(exn, true);
    pabort("Unreachable");
  }

  LIBPORT_SPEED_INLINE
  void
  raise_arity_error(unsigned effective,
                    unsigned minimum,
                    unsigned maximum)
  {
    Runner& r = ::urbiserver->getCurrentRunner();
    object::rObject exn =
      urbi_call(r, object::global_class->slot_get(SYMBOL(ArityError)),
                SYMBOL(new),
                new object::String(r.innermost_call_get()),
                new object::Float(effective),
                new object::Float(minimum),
                new object::Float(maximum));
    r.raise(exn, true);
    pabort("Unreachable");
  }

  LIBPORT_SPEED_INLINE
  void
  raise_argument_type_error(unsigned idx,
                            object::rObject effective,
                            object::rObject expected)
  {
    Runner& r = ::urbiserver->getCurrentRunner();
    r.raise(
      urbi_call(r, object::global_class->slot_get(SYMBOL(ArgumentTypeError)),
                SYMBOL(new),
                new object::String(r.innermost_call_get()),
                new object::Float(idx),
                expected,
                effective),
      true);
    pabort("Unreachable");
  }

}

#endif
