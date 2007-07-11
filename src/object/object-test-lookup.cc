#include <iostream>
#include "object-test.hh"

using namespace object;
using libport::Symbol;

int
main ()
{
  rObject top = new Object ();
  (*top)["name"] = new String("Top_Class");
  (*top)["val"]  = new Integer(42);

  rObject left = new Object ();
  (*left)["name"] = new String("Left_class");
  (*left)["lval"] = new String("Left_Value");

  rObject right = new Object ();
  (*right)["name"] = new String("Right_class");
  (*right)["rval"] = new String("Right_Value");

  rObject bottom = new Object ();
  (*bottom)["name"] = new String("Bottom_class");
  (*bottom)["bval"] = new String("Bottom_Value");

  (*left).parent_add (top);
  (*right).parent_add (top);
  (*bottom).parent_add (left);
  (*bottom).parent_add (right);

  NEWLINE();
  ECHO(*top);
  ECHO(*left);
  ECHO(*right);
  ECHO(*bottom);

  NEWLINE();
  ECHOVAL(top, name);
  ECHOVAL(top, val);
  ECHOVAL(left, name);
  ECHOVAL(left, val);
  ECHOVAL(left, lval);
  ECHOVAL(right, rval);
  ECHOVAL(bottom, name);
  ECHOVAL(bottom, val);
  ECHOVAL(bottom, rval);
  ECHOVAL(bottom, bval);

  NEWLINE();
  rObject right_top = new Object ();
  (*right_top)["name"] = new String("RightTop_Class");
  (*right_top)["rightonly"]  = new String("Right_only_value");
  (*right).parent_add (right_top);
  ECHOVAL(bottom, rightonly);

  return 0;
}
