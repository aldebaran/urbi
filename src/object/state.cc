/**
 ** \file object/state.cc
 ** \brief Implementation of object::State.
 */

#include <iostream>

#include "libport/indent.hh"

#include "object/state.hh"

namespace object
{
  
  State::State (UConnection& c)
    : connection (c)
  {}

  std::ostream&
  operator<< (std::ostream& o, const State& s)
  {
    return o
      << "State" << libport::iendl
      << '{' << libport::incendl
      << "Connection = " << &s.connection << ',' << libport::decendl
      << '}';
  }


}; // namespace object

