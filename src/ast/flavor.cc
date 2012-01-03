/*
 * Copyright (C) 2007-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file ast/flavor.cc
 ** \brief Implementation of ast::flavor_type.
 */

#include <libport/cstdlib>

#include <libport/cassert>

#include <ast/flavor.hh>

namespace ast
{

  std::ostream&
  operator<<(std::ostream& o, flavor_type e)
  {
    switch (e)
    {
#define CASE(E, S) case flavor_ ## E: return o << S;
      CASE(none,      "???")
      CASE(and,       '&')
      CASE(comma,     ',')
      CASE(pipe,      '|')
      CASE(semicolon, ';')
#undef CASE
    }
    pabort(e);
  }

} // namespace ast
