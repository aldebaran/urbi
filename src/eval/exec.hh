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
 ** \file runner/eval/exec.hh
 ** \brief Declaration of eval::exec.
 */

#ifndef EVAL_EXEC_HH
# define EVAL_EXEC_HH

# include <eval/action.hh>

namespace eval
{
  /// As opposed to eval::call these are not filling the state fo the
  /// interpreter and neither the stack, they only provide way to execute
  /// transparently executable such as Code or primitive.

  Action  exec(object::rExecutable e,
               const object::objects_type& args);

  rObject exec(Job& job,
               object::rExecutable e,
               const object::objects_type& args);

  /// Execute a boost function with no expected result. This is inefficient,
  /// and should be avoided. You should prefer adding an rObject result to
  /// the function and make it accept a Job& as argument in order to
  /// create an Action out of it.
  Action  exec(const boost::function0<void>& e,
               rObject self);

  rObject exec(Job& job,
               const boost::function0<void>& e,
               rObject self);

} // namespace eval

# include <eval/exec.hxx>

#endif // ! EVAL_EXEC_HH
