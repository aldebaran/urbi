/*
 * Copyright (C) 2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file parser/event-match.cc
 ** \brief EventMatch inline implementation.
 */

namespace ast
{
  inline
  EventMatch::EventMatch(ast::rExp event,
                         ast::exps_type* pattern,
                         ast::rExp duration,
                         ast::rExp guard)
    : event(event)
    , pattern(pattern)
    , duration(duration)
    , guard(guard)
  {}

  inline
  EventMatch::EventMatch()
    : event(0)
    , pattern(0)
    , duration(0)
    , guard(0)
  {}

  inline
  std::ostream&
  operator<<(std::ostream& o, const EventMatch& e)
  {
    return e.dump(o);
  }

}
