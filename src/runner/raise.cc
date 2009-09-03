/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <kernel/userver.hh>
#include <object/cxx-conversions.hh>
#include <object/global.hh>
#include <object/symbols.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>
#include <runner/sneaker.hh>
#include <sched/scheduler.hh>

#define assert_user_mode(Exn, Msg)                              \
  __passert((::kernel::urbiserver->mode_get()                   \
             == kernel::UServer::mode_user),                    \
            Exn << " exception thrown in kernel mode: "         \
            << Msg                                              \
            << dbg::runner_or_sneaker_get().backtrace_get())

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
	     rObject arg4,
             bool skip)
  {
    // Too dangerous to try to print arg1 etc. here, as it certainly
    // involves running urbiScript code.
    assert_user_mode(exn_name, "");
    Runner& r = dbg::runner_or_sneaker_get();

    const rObject& exn = global_class->slot_get(exn_name);
    if (arg1 == raise_current_method)
      arg1 = to_urbi(r.innermost_call_get());
    r.raise(exn->call(SYMBOL(new), arg1, arg2, arg3, arg4),
            skip);
    pabort("Unreachable");
  }

  void
  raise_urbi_skip(libport::Symbol exn_name,
                  rObject arg1,
                  rObject arg2,
                  rObject arg3,
                  rObject arg4)
  {
    raise_urbi(exn_name, arg1, arg2, arg3, arg4, true);
  }

  void
  raise_lookup_error(libport::Symbol msg, const object::rObject& obj)
  {
    assert_user_mode("Lookup", msg);
    raise_urbi_skip(SYMBOL(LookupError),
                    to_urbi(msg),
                    obj);
  }

  void
  raise_const_error()
  {
    raise_urbi_skip(SYMBOL(ConstError));
  }


  void
  raise_arity_error(unsigned effective,
                    unsigned expected)
  {
    raise_urbi_skip(SYMBOL(ArityError),
                    raise_current_method,
                    to_urbi(effective),
                    to_urbi(expected));
  }

  void
  raise_arity_error(unsigned effective,
                    unsigned minimum,
                    unsigned maximum)
  {
    raise_urbi_skip(SYMBOL(ArityError),
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
    raise_urbi_skip(SYMBOL(ArgumentTypeError),
                    method_name,
                    to_urbi(idx),
                    expected,
                    effective);
  }

  void
  raise_bad_integer_error(libport::ufloat effective,
			  const std::string& fmt)
  {
    raise_urbi_skip(SYMBOL(BadIntegerError),
                    raise_current_method,
                    to_urbi(fmt),
                    to_urbi(effective));
  }

  void
  raise_primitive_error(const std::string& message)
  {
    raise_urbi_skip(SYMBOL(PrimitiveError),
                    raise_current_method,
                    to_urbi(message));
  }

  void
  raise_unexpected_void_error()
  {
    raise_urbi_skip(SYMBOL(UnexpectedVoidError));
  }

}
