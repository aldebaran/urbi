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
 ** \file runner/eval/call.hh
 ** \brief Definition of eval::call.
 */

#ifndef EVAL_RAISE_HH
# define EVAL_RAISE_HH

# include <eval/action.hh>

# include <ast/loc.hh>

namespace eval
{

  void
  raise(Job& job,
        rObject exn, bool skip_last = false);

  void
  raise(Job& job,
        rObject exn, bool skip_last,
        const boost::optional<ast::loc>& loc);


} // namespace eval

#endif // ! EVAL_RAISE_HH
