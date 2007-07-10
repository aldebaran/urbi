#include <iostream>
#include "object/object.hh"
#include "object/atom.hh"

#define ECHO(This) std::cerr << This << std::endl
#define NEWLINE()  std::cerr << std::endl

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

  NEWLINE();
  rObject top = new Object ();
  (*top)["name"] = new String("Top_Class");
  (*top)["val"]  = new Integer(42);
  ECHO(*top);

  rObject left = new Object ();
  (*left)["name"] = new String("Left_class");
  (*left).parent_add (top);
  ECHO(*left);
  (*left).parent_add (top);
  ECHO(*left);
  (*left).parent_remove (top);
  ECHO(*left);
  (*left).parent_remove (top);
  ECHO(*left);
  (*left).parent_add (top);
  ECHO(*left);

  NEWLINE();
  rObject top_name = (*top).lookup ("name");
  ECHO(*top_name);
  rObject top_val = (*top).lookup ("val");
  ECHO(*top_val);
  rObject left_name = (*left).lookup ("name");
  ECHO(*left_name);
  rObject left_val = (*left).lookup ("val");
  ECHO(*left_val);

  NEWLINE();
  try
  {
  rObject left_nosuch = (*left).lookup ("no such attr");
  ECHO(*left_nosuch);
  }
  catch (std::exception e)
  { }

  return 0;
}
