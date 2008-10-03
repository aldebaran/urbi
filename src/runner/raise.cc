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
  raise_urbi(libport::Symbol exn_name,
	     rObject arg1,
	     rObject arg2,
	     rObject arg3,
	     rObject arg4)
  {
    assert(global_class->slot_has(exn_name));
    const rObject& exn = global_class->slot_get(exn_name);
    Runner& r = ::urbiserver->getCurrentRunner();
    objects_type args;
    if (arg1)
      args.push_back(arg1);
    else
      args.push_back(to_urbi(r.innermost_call_get()));
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
    raise_urbi(SYMBOL(LookupError),
	       to_urbi(msg),
	       obj);
  }


  void
  raise_arity_error(unsigned effective,
                    unsigned expected)
  {
    raise_urbi(SYMBOL(ArityError),
	       0,
	       to_urbi(effective),
	       to_urbi(expected));
  }

  void
  raise_arity_error(unsigned effective,
                    unsigned minimum,
                    unsigned maximum)
  {
    raise_urbi(SYMBOL(ArityError),
	       0,
	       to_urbi(effective),
	       to_urbi(minimum),
	       to_urbi(maximum));
  }

  void
  raise_argument_type_error(unsigned idx,
                            rObject effective,
                            rObject expected)
  {
    raise_urbi(SYMBOL(ArgumentTypeError),
	       0,
	       to_urbi(idx),
	       expected,
	       effective);
  }

  void
  raise_bad_integer_error(libport::ufloat effective,
			  const std::string fmt)
  {
    raise_urbi(SYMBOL(BadIntegerError),
	       0,
	       to_urbi(fmt),
	       to_urbi(effective));
  }

}
