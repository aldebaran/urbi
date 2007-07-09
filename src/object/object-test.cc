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
  i["value 2"] = new Integer(5);
  ECHO(i);

  Object o;
  o["drink"] = new Integer(51);
  o["name"]  = new String("Pastis");
  o["degre"] = new Float(.45);
  ECHO(o);
}
