/**
 ** \file ast/flavor.cc
 ** \brief Implementation of ast::flavor.
 */

#include <cstdlib>
#include "ast/flavor.hh"

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
     abort();
  }

} // namespace ast

