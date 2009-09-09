/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/**
 ** \file parser/event-match.cc
 ** \brief EventMatch implementation.
 */

#include <ast/print.hh>
#include <ast/event-match.hh>

namespace ast
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
