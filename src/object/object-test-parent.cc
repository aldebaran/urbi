#include <iostream>
#include "object-test.hh"

using namespace object;
using libport::Symbol;

int
main ()
{
  NEWLINE();
  rObject top = new Object ();
  (*top)["type"] = new String("Top_Class");
  (*top)["val"]  = new Integer(42);
  ECHO("top: " << *top);

  rObject left = new Object();
  (*left)["type"] = new String("Left_class");
  (*left).parent_add (top);
  ECHO("left 2: " << *left);
  (*left).parent_add (top);
  ECHO("left 3: " << *left);
  (*left).parent_remove (top);
  ECHO("left 4: " << *left);
  (*left).parent_remove (top);
  ECHO("left 5: " << *left);
  (*left).parent_add (top);
  ECHO("left 6: " << *left);

  rObject right = clone (top);
  (*left)["type"] = new String("Right_class");
  ECHO("right 1: " << *right);
}
