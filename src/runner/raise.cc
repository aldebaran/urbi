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

  // We can use void as the current method because it cannot be used
  // as a function call parameter.

  const rObject& raise_current_method = void_class;

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
    do
    {
      if (arg1)
      {
	if (arg1 == raise_current_method)
	  args.push_back(to_urbi(r.innermost_call_get()));
	else
	  args.push_back(arg1);
      }
      else
	break;
      if (arg2)
	args.push_back(arg2);
      else
	break;
      if (arg3)
	args.push_back(arg3);
      else
	break;
      if (arg4)
	args.push_back(arg4);
      else
	break;
    } while (false);
    r.raise(urbi_call(r, exn, SYMBOL(new), args), true);
    pabort("Unreachable");
  }

  void
  raise_lookup_error(libport::Symbol msg, const object::rObject& obj)
  {
    assert(global_class->slot_has(SYMBOL(LookupError)));
    raise_urbi(SYMBOL(LookupError),
	       to_urbi(msg),
	       obj);
  }


  void
  raise_arity_error(unsigned effective,
                    unsigned expected)
  {
    raise_urbi(SYMBOL(ArityError),
	       raise_current_method,
	       to_urbi(effective),
	       to_urbi(expected));
  }

  void
  raise_arity_error(unsigned effective,
                    unsigned minimum,
                    unsigned maximum)
  {
    raise_urbi(SYMBOL(ArityError),
	       raise_current_method,
	       to_urbi(effective),
	       to_urbi(minimum),
	       to_urbi(maximum));
  }

  void
  raise_argument_type_error(unsigned idx,
                            rObject effective,
                            rObject expected,
			    rObject method_name)
  {
    raise_urbi(SYMBOL(ArgumentTypeError),
	       method_name,
	       to_urbi(idx),
	       expected,
	       effective);
  }

  void
  raise_bad_integer_error(libport::ufloat effective,
			  const std::string fmt)
  {
    raise_urbi(SYMBOL(BadIntegerError),
	       raise_current_method,
	       to_urbi(fmt),
	       to_urbi(effective));
  }

  void
  raise_primitive_error(const std::string message)
  {
    raise_urbi(SYMBOL(PrimitiveError),
	       raise_current_method,
	       to_urbi(message));
  }

  void
  raise_unexpected_void_error()
  {
    raise_urbi(SYMBOL(UnexpectedVoidError));
  }

}
