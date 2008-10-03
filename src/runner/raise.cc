#include <kernel/userver.hh>
#include <object/cxx-conversions.hh>
#include <object/global.hh>
#include <runner/runner.hh>
#include <runner/call.hh>
#include <runner/raise.hh>
#include <scheduler/scheduler.hh>

namespace runner
{
  using namespace object;

  void
  raise_urbi(const rObject& exn,
	     rObject arg1,
	     rObject arg2,
	     rObject arg3,
	     rObject arg4)
  {
    Runner& r = ::urbiserver->getCurrentRunner();
    objects_type args;
    args.push_back(arg1);
    args.push_back(arg2);
    if (arg3)
      args.push_back(arg3);
    if (arg4)
      args.push_back(arg4);
    r.raise(urbi_call(r, exn, SYMBOL(new), args), true);
    pabort("Unreachable");
  }

  void
  raise_lookup_error(libport::Symbol msg, const object::rObject& obj)
  {
    raise_urbi(global_class->slot_get(SYMBOL(LookupError)),
	       to_urbi(msg),
	       obj);
    pabort("Unreachable");
  }


  void
  raise_arity_error(unsigned effective,
                    unsigned expected)
  {
    Runner& r = ::urbiserver->getCurrentRunner();
    raise_urbi(global_class->slot_get(SYMBOL(ArityError)),
	       to_urbi(r.innermost_call_get()),
	       to_urbi(effective),
	       to_urbi(expected));
    pabort("Unreachable");
  }

  void
  raise_arity_error(unsigned effective,
                    unsigned minimum,
                    unsigned maximum)
  {
    Runner& r = ::urbiserver->getCurrentRunner();
    raise_urbi(global_class->slot_get(SYMBOL(ArityError)),
	       to_urbi(r.innermost_call_get()),
	       to_urbi(effective),
	       to_urbi(minimum),
	       to_urbi(maximum));
    pabort("Unreachable");
  }

  void
  raise_argument_type_error(unsigned idx,
                            rObject effective,
                            rObject expected)
  {
    Runner& r = ::urbiserver->getCurrentRunner();
    raise_urbi(global_class->slot_get(SYMBOL(ArgumentTypeError)),
	       to_urbi(r.innermost_call_get()),
	       to_urbi(idx),
	       expected,
	       effective);
    pabort("Unreachable");
  }

}
