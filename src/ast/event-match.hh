/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file parser/event-match.hh
 ** \brief EventMatch declaration.
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
               ast::exps_type* pattern, ast::rExp duration, ast::rExp guard);
    EventMatch();

    std::ostream& dump(std::ostream& o) const;

    /// Cannot be 0 in regular use.
    ast::rExp event;
    /// Can be 0.
    ast::exps_type* pattern;
    /// Can be 0.
    ast::rExp duration;
    /// Can be 0.
    ast::rExp guard;
  };

  std::ostream&
  operator<<(std::ostream& o, const EventMatch& e);
}

# include <ast/event-match.hxx>

#endif // ! AST_EVENT_MATCH_HH
