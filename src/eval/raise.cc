/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */


/**
 ** \file eval/raise.cc
 ** \brief Implementation of raise urbi exceptions.
 */

#include <libport/debug.hh>

#include <eval/raise.hh>

#include <ast/ast.hh>

#include <urbi/object/global.hh>
#include <urbi/object/location.hh>
#include <urbi/object/job.hh>

#include <urbi/object/urbi-exception.hh>

#include <runner/job.hh>
#include <runner/state.hh>

namespace eval
{

  void
  raise(Job& job,
        rObject exn, bool skip_last)
  {
    raise(job, exn, skip_last, boost::optional<ast::loc>());
    pabort("Unreachable");
  }

  void
  raise(Job& job,
        rObject exn, bool skip_last,
        const boost::optional<ast::loc>& loc)
  {
    CAPTURE_GLOBAL(Exception);

    // innermost_node_ can be empty if the interpreter has not interpreted
    // any urbiscript.  E.g., slot_set can raise an exception only from the
    // C++ side.  It would be better to produce a C++ backtrace instead.
    if (is_a(exn, Exception) && job.state.innermost_node_get())
    {
      boost::optional<ast::loc> l =
        loc ? loc : job.state.innermost_node_get()->location_get();
      exn->slot_update(SYMBOL(DOLLAR_location), object::to_urbi(l));
      exn->slot_update(SYMBOL(DOLLAR_backtrace),
                       job.as_job()->as<object::Job>()->backtrace());
    }
    // FIXME: This cause a second duplication of the backtrace.
    runner::State::call_stack_type bt = job.state.call_stack_get();
    if (skip_last && !bt.empty())
      bt.pop_back();
    throw object::UrbiException(exn, bt);
  }

} // namespace eval
