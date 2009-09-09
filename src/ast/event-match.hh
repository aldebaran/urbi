/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */


#ifndef AST_EVENT_MATCH_HH
# define AST_EVENT_MATCH_HH

# include <ast/fwd.hh>
# include <ast/exp.hh>
# include <ast/exps-type.hh>

namespace ast
{
  struct EventMatch
  {
    EventMatch(ast::rExp event,
               ast::exps_type* pattern,
               ast::rExp guard)
      : event(event)
      , pattern(pattern)
      , guard(guard)
    {}

    EventMatch()
      : event(0)
      , pattern(0)
      , guard(0)
    {}

    std::ostream& dump(std::ostream& o) const;

    ast::rExp event;
    ast::exps_type* pattern;
    ast::rExp guard;
  };

  std::ostream&
  operator<<(std::ostream& o, const EventMatch& e);
}

#endif // ! AST_EVENT_MATCH_HH
