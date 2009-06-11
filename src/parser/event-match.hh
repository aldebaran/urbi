/**
 ** \file parser/fwd.hh
 ** \brief Forward declarations of all parser classes
 ** (needed by the visitors).
 */

#ifndef PARSER_EVENT_MATCH_HH
# define PARSER_EVENT_MATCH_HH

# include <ast/fwd.hh>
# include <ast/exp.hh>
# include <ast/exps-type.hh>

namespace parser
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

#endif // ! PARSER_EVENT_MATCH_HH
