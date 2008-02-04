/**
 ** \file object/string-class.cc
 ** \brief Creation of the URBI object string.
 */

#include "object/string-class.hh"
#include "object/object.hh"
#include "object/primitives.hh"
#include "object/atom.hh"

namespace object
{
  rObject string_class;

  /*--------------------.
  | String primitives.  |
  `--------------------*/

  namespace
  {

    // Don't forget our Strings are wrapped in Symbols.
#define PRIMITIVE_OP_STRING(Name, Op)				\
    static							\
    rFloat							\
    Name (rString& a, rString& b)                               \
    {                                                           \
      return new Float (a->value_get ().name_get()		\
			Op					\
			b->value_get ().name_get());		\
    }

    PRIMITIVE_OP_STRING(equ, ==);
    PRIMITIVE_OP_STRING(lth,  <);

#undef PRIMITIVE_OP_STRING

    static
    rString
    add (rString& a, rString& b)
    {
      return new String(libport::Symbol (a->value_get ().name_get()
					 + b->value_get ().name_get()));
    }

    /// Return string's length.
    static rFloat
    size (rString& s)
    {
      return new Float (s->value_get ().name_get ().size ());
    }

  };

#define PRIMITIVE_1_STRING(Name)                  \
  PRIMITIVE_1(string, Name, Name, String)

#define PRIMITIVE_2_STRING(Name, Type2)           \
  PRIMITIVE_2(string, Name, Name, String, Type2)

  PRIMITIVE_2_STRING(add, String);
  PRIMITIVE_2_STRING(equ, String);

  PRIMITIVE_2_STRING(lth, String);

  PRIMITIVE_1_STRING(size);

#undef PRIMITIVE_2_STRING
#undef PRIMITIVE_1_STRING

  void
  string_class_initialize ()
  {
#define DECLARE(Name, Implem)                   \
    DECLARE_PRIMITIVE(string, Name, Implem)

    DECLARE(PLUS,  add);
    DECLARE(EQ_EQ, equ);
    DECLARE(LT,  lth);

    DECLARE(size, size);
    DECLARE(length, size);
#undef DECLARE
  }
}; // namespace object
