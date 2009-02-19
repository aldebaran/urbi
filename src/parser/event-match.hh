/**
 ** \file parser/fwd.hh
 ** \brief Forward declarations of all parser classes
 ** (needed by the visitors).
 */

#ifndef PARSER_EVENT_MATCH_HH
# define PARSER_EVENT_MATCH_HH

# include <ast/fwd.hh>
# include <ast/exp.hh>

namespace parser
{
  struct EventMatch
  {
    ast::rExp event;
    ast::exps_type* pattern;
    ast::rExp guard;

    EventMatch(ast::rExp event_,
               ast::exps_type* pattern_,
               ast::rExp guard_)
      : event(event_)
      , pattern(pattern_)
      , guard(guard_)
    {}

    EventMatch()
      : event(0)
      , pattern(0)
      , guard(0)
    {}
  };

}

#endif // ! PARSER_EVENT_MATCH_HH
