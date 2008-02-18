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

    PRIMITIVE_OP_STRING(EQ_EQ, ==);
    PRIMITIVE_OP_STRING(LT,  <);

#undef PRIMITIVE_OP_STRING

    static
    rString
    PLUS (rString& a, rString& b)
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
  PRIMITIVE_1(string, Name, String)

#define PRIMITIVE_2_STRING(Name, Type2)           \
  PRIMITIVE_2(string, Name, String, Type2)

  PRIMITIVE_2_STRING(PLUS, String);
  PRIMITIVE_2_STRING(EQ_EQ, String);

  PRIMITIVE_2_STRING(LT, String);

  PRIMITIVE_1_STRING(size);

#undef PRIMITIVE_2_STRING
#undef PRIMITIVE_1_STRING

  void
  string_class_initialize ()
  {
#define DECLARE(Name)                   \
    DECLARE_PRIMITIVE(string, Name)

    DECLARE(PLUS);
    DECLARE(EQ_EQ);
    DECLARE(LT);

    DECLARE(size);
#undef DECLARE
  }
}; // namespace object
