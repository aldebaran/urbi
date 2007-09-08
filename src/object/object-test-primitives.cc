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
  objects_type args;

  ECHO("Float: " << *float_class);

  ECHO("2: " << *f2);
  ECHO("3: " << *f3);
  args.push_back (f2);
  args.push_back (f10);
  rObject f20 = float_class_mul(0, args);
  ECHO("20: " << *f20);

  args.clear();
  args.push_back (f20);
  args.push_back (f3);
  rObject f23 = float_class_add(0, args);
  ECHO("23: " << *f23);
}
