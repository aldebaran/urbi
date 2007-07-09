#include <iostream>
#include "object/object.hh"
#include "object/atom.hh"

#define ECHO(This) std::cerr << This << std::endl

using namespace object;
using libport::Symbol;

int
main ()
{
  Integer i (23);
  ECHO(i);

  Object o;
  o[Symbol("num")] = new Integer(23);
  o[Symbol("drink")] = new Integer(51);
  ECHO(o);
}
