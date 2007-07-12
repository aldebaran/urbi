#include <iostream>
#include "object-test.hh"
#include "atom.hh"

using namespace object;
using libport::Symbol;

int
main ()
{
  rFloat f2 = new Float (2);
  rFloat f10 = new Float (10);
  rFloat f3 = new Float (3);
  
  ECHO("2: " << *f2);
  ECHO("3: " << *f3);
  rFloat f23 = float_class_add(float_class_mul(f2, f10), f3);
  ECHO("23: " << *f23);
}
