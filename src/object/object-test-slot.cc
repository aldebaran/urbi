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

  // Print hierarchy
  NEWLINE();
  ECHO(*top);
  ECHO(*left);
  ECHO(*right);
  ECHO(*bottom);

  // Update parent value
  NEWLINE();
  rObject update_val (new String("update_val"));
  (*left).update_slot ("val", update_val);
  ECHOVAL(top, val);
  ECHOVAL(left, val);

  // Redefine parent attribute in local class
  NEWLINE();
  rObject set_val (new String("set_val"));
  (*left).set_slot ("val", set_val);
  ECHOVAL(top, val);
  ECHOVAL(left, val);

  // Add new attribute to local class
  NEWLINE();
  rObject new_val (new String("new_val"));
  (*left).set_slot ("new_val", new_val);
  ECHO(*top);
  ECHO(*left);

  // Check operator[] behavior
  NEWLINE();
  (*top)["val"] = new String("Top_value");
  ECHOVAL(top, val);
  (*top)["extraval"] = new String("Top_extra_value");
  ECHOVAL(top, extraval);

  // Remove slot
  NEWLINE();
  (*left).remove_slot ("new_val");
  ECHO(*left);

  return 0;
}
