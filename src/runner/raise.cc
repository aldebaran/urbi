/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <kernel/userver.hh>

#include <object/symbols.hh>

#include <urbi/runner/raise.hh>
#include <runner/runner.hh>
#include <runner/sneaker.hh>

#include <sched/scheduler.hh>

#include <urbi/object/cxx-conversions.hh>
#include <urbi/object/global.hh>
#include <urbi/object/location.hh>

#define assert_user_mode(Exn, Msg)                              \
  __passert((::kernel::urbiserver->mode_get()                   \
             == kernel::UServer::mode_user),                    \
            Exn << " exception thrown in kernel mode: "         \
            << Msg                                              \
            << dbg::runner_or_sneaker_get().backtrace_get())

namespace runner
{
  using namespace object;
  RaiseCurrent raise_current_method;
  void
  raise_urbi(libport::Symbol exn_name,
             RaiseCurrent,
	     rObject arg1,
	     rObject arg2,
	     rObject arg3,
             bool skip)
  {
    Runner& r = dbg::runner_or_sneaker_get();
    raise_urbi(exn_name, to_urbi(r.innermost_call_get()), arg1, arg2, arg3,
               skip);
  }

  void
  raise_urbi(libport::Symbol exn_name,
	     rObject arg1,
	     rObject arg2,
	     rObject arg3,
	     rObject arg4,
             bool skip)
  {
    // Too dangerous to try to print arg1 etc. here, as it certainly
    // involves running urbiscript code.
    // assert_user_mode(exn_name, "");
    assert_ne(exn_name, SYMBOL(Exception));
    Runner& r = dbg::runner_or_sneaker_get();
    CAPTURE_GLOBAL(Exception);
    const rObject& exn = Exception->slot_get(exn_name);
    if (arg1 == void_class)
      raise_unexpected_void_error();
    r.raise(exn->call(SYMBOL(new), arg1, arg2, arg3, arg4), skip);
    pabort("Unreachable");
  }

  void
  raise_urbi_skip(libport::Symbol exn_name,
                  RaiseCurrent,
                  rObject arg1,
                  rObject arg2,
                  rObject arg3)
  {
    raise_urbi(exn_name, raise_current_method, arg1, arg2, arg3, true);
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
  raise_argument_type_error(unsigned idx,
                            rObject effective,
                            rObject expected,
			    rObject method_name)
  {
    if (method_name)
      raise_urbi_skip(SYMBOL(ArgumentType),
                    method_name,
                    to_urbi(idx),
                    effective,
                    expected);
    else
      raise_urbi_skip(SYMBOL(ArgumentType),
                      raise_current_method,
                      to_urbi(idx),
                      effective,
                      expected);
  }

  void
  raise_arity_error(unsigned effective,
                    unsigned expected)
  {
    raise_urbi_skip(SYMBOL(Arity),
                    raise_current_method,
                    to_urbi(effective),
                    to_urbi(expected));
  }

  void
  raise_arity_error(unsigned effective,
                    unsigned minimum,
                    unsigned maximum)
  {
    raise_urbi_skip(SYMBOL(Arity),
                    raise_current_method,
                    to_urbi(effective),
                    to_urbi(minimum),
                    to_urbi(maximum));
  }

  void
  raise_bad_integer_error(libport::ufloat effective,
			  const std::string& fmt)
  {
    raise_urbi_skip(SYMBOL(BadInteger),
                    raise_current_method,
                    to_urbi(fmt),
                    to_urbi(effective));
  }

  void
  raise_const_error()
  {
    raise_urbi_skip(SYMBOL(Constness));
  }

  void
  raise_lookup_error(libport::Symbol msg, const object::rObject& obj,
                     bool deep)
  {
    // assert_user_mode("Lookup", msg);
    raise_urbi_skip(SYMBOL(Lookup),
                    to_urbi(msg),
                    obj,
                    to_urbi(deep));
  }

  void
  raise_negative_number_error(libport::ufloat effective)
  {
    raise_urbi_skip(SYMBOL(NegativeNumber),
                    raise_current_method,
                    to_urbi(effective));
  }

  void
  raise_non_positive_number_error(libport::ufloat effective)
  {
    raise_urbi_skip(SYMBOL(NonPositiveNumber),
                    raise_current_method,
                    to_urbi(effective));
  }

  void
  raise_primitive_error(const std::string& message)
  {
    raise_urbi_skip(SYMBOL(Primitive),
                    raise_current_method,
                    to_urbi(message));
  }

  void
  raise_scheduling_error(const std::string& message)
  {
    raise_urbi_skip(SYMBOL(Scheduling), to_urbi(message));
  }

  void
  raise_syntax_error(const ast::loc& location,
                     const std::string& message,
                     const std::string& input,
                     bool error)
  {
    raise_urbi_skip(SYMBOL(Syntax),
                    to_urbi(location), to_urbi(message), to_urbi(input),
                    to_urbi(error));
  }

  void
  raise_type_error(rObject effective,
                   rObject expected)
  {
    raise_urbi_skip(SYMBOL(Type), effective, expected);
  }

  void
  raise_unexpected_void_error()
  {
    if (::kernel::runner().void_error_get())
      raise_urbi_skip(SYMBOL(UnexpectedVoid));
  }
}
