/**
 ** \file object/string-class.cc
 ** \brief Creation of the URBI object string.
 */

#include <libport/lexical-cast.hh>

#include "libport/tokenizer.hh"

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
#define PRIMITIVE_OP_STRING(Name, Op)			\
    static						\
    rFloat						\
    Name (rString& a, rString& b)			\
    {							\
      return new Float (a->value_get ().name_get()	\
			Op				\
			b->value_get ().name_get());	\
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

    static rObject
    string_class_asString(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT (1);
      libport::Symbol res;
      if (args[0] == string_class)
	res = SYMBOL(LT_String_GT);
      /// FIXME: Fix escape to be able to return a string then fix this.
      else
      {
	res = libport::Symbol('"'
			      + string_cast(libport::escape(VALUE(args[0], String).name_get()))
			      + '"');
      }
      return new String(res);
    }

    static rObject
    string_class_split(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT (2);
      boost::tokenizer< boost::char_separator<char> > tok
      = libport::make_tokenizer(VALUE(args[0], String).name_get(),
				VALUE(args[1], String).name_get().c_str());
      list_traits::type ret;
      foreach(std::string i, tok)
	ret.push_back(new String(libport::Symbol(i)));
      return new List(ret);
    }
  };

#define PRIMITIVE_1_STRING(Name)                  \
  PRIMITIVE_1(string, Name, String)

#define PRIMITIVE_2_STRING(Name, Type2)		\
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
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(string, Name)

    DECLARE(PLUS);
    DECLARE(EQ_EQ);
    DECLARE(LT);

    DECLARE(asString);
    DECLARE(size);
    DECLARE(split);
#undef DECLARE
  }
}; // namespace object
