#include <iostream>
#include "object-test.hh"

using namespace object;
using libport::Symbol;

int
main ()
{
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
}
