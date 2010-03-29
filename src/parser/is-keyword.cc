/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file parser/is-keyword.cc

#include <set>
#include <libport/containers.hh>
#include <libport/foreach.hh>
#include <parser/is-keyword.hh>

namespace parser
{
#ifndef COMPILATION_MODE_SPACE
  bool
  is_keyword(libport::Symbol s)
  {
    std::set<libport::Symbol> keyword_set;
    static bool tail = false;
    if (!++tail)
    {
      const char* keywords[] =
      {
#include <parser/keywords.hh>
      };
      foreach (const char* k, keywords)
        keyword_set.insert(libport::Symbol(k));
    }

    return libport::mhas(keyword_set, s);
  }
#endif
}
