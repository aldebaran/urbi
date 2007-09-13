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

#define PRIMITIVE_OP_STRING(Name, Op, TypeRet)                  \
    static rObject                                              \
    Name (rString& a, rString& b)                               \
    {                                                           \
      return new TypeRet (a->value_get () Op b->value_get ());  \
    }

    PRIMITIVE_OP_STRING(equ, ==, Integer);
    PRIMITIVE_OP_STRING(nequ, !=, Integer);
    PRIMITIVE_OP_STRING(lth, <, Integer);

#undef PRIMITIVE_OP_STRING

    /// Return string's length.
    static rObject
    size (rString& s)
    {
      return new Integer (s->value_get ().name_get ().size ());
    }

  };

#define PRIMITIVE_1_STRING(Name)                  \
  PRIMITIVE_1(string, Name, Name, String)

#define PRIMITIVE_2_STRING(Name, Type2)           \
  PRIMITIVE_2(string, Name, Name, String, Type2)

  PRIMITIVE_2_STRING(equ, String);
  PRIMITIVE_2_STRING(nequ, String);

  PRIMITIVE_2_STRING(lth, String);

  PRIMITIVE_1_STRING(size);

#undef PRIMITIVE_2_STRING
#undef PRIMITIVE_1_STRING

  void
  string_class_initialize ()
  {
#define DECLARE(Name, Implem)                   \
    DECLARE_PRIMITIVE(string, Name, Implem)

    DECLARE(==, equ);
    DECLARE(!=, nequ);
    DECLARE(<, lth);

    DECLARE(size, size);
    DECLARE(length, size);
#undef DECLARE
  }
}; // namespace object
