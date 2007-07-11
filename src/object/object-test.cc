#include <iostream>
#include "object-test.hh"

using namespace object;
using libport::Symbol;

int
main ()
{
  Integer i (23);
  i["value 2"] = new Integer(5);
  ECHO(i);

  NEWLINE();
  Object o;
  o["drink"] = new Integer(51);
  o["name"]  = new String("Pastis");
  o["degre"] = new Float(.45);
  o["x"] = new Object();
  (*o["x"])["val"] = new String("o.x.val");
  ECHO(o);

  return 0;
}
