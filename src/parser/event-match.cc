/**
 ** \file parser/event-match.cc
 ** \brief EventMatch implementation.
 */

#include <ast/print.hh>
#include <parser/event-match.hh>

namespace parser
{
  std::ostream&
  EventMatch::dump(std::ostream& o) const
  {
    return o << libport::deref << event
             << " ? "
             << libport::deref << pattern
             << " if "
             << libport::deref << guard;
  }

  std::ostream&
  operator<<(std::ostream& o, const EventMatch& e)
  {
    return e.dump(o);
  }

}

