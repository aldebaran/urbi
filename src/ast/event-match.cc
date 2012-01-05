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
 ** \file parser/event-match.cc
 ** \brief EventMatch implementation.
 */

#include <libport/deref.hh>
#include <ast/print.hh>
#include <ast/event-match.hh>

namespace ast
{
  std::ostream&
  EventMatch::dump(std::ostream& o) const
  {
    o << libport::deref << event;
    if (pattern)
      o << " ? " << *pattern;
    if (duration)
      o << " ~ " << *duration << "s";
    if (guard)
      o << " if " << *guard;
    return o;
  }

}
