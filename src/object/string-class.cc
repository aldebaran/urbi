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
#include "runner/runner.hh"

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
      return Float::fresh(a->value_get ().name_get()    \
                          Op				\
                          b->value_get ().name_get());	\
    }

    PRIMITIVE_OP_STRING(EQ_EQ, ==);
    PRIMITIVE_OP_STRING(LT,  <);

#undef PRIMITIVE_OP_STRING

    /// Return string's length.
    static rFloat
    size (rString& s)
    {
      return Float::fresh(s->value_get ().name_get ().size ());
    }

/*-------------------------------------------------------------------.
| String Primitives.                                                 |
|                                                                    |
| I.e., the signature is runner::Runner& x objects_type -> rObject.  |
`-------------------------------------------------------------------*/

    static rObject
    string_class_PLUS (runner::Runner& r, objects_type args)
    {
      CHECK_ARG_COUNT (2);
      FETCH_ARG (0, String);
      std::string str0 = arg0->value_get ().name_get ();
      std::string str1;
      if (args[1]->type_is<String> ())
      {
	FETCH_ARG (1, String);
	str1 = arg1->value_get ().name_get ();
      }
      else
      {
	rObject as = urbi_call(r, args[1], SYMBOL(asString));
	TYPE_CHECK (as, String);
	rString arg1 = as.unsafe_cast<String> ();
	str1 = arg1->value_get ().name_get ();
      }
      return String::fresh(libport::Symbol (str0 + str1));
    }

    static rObject
    string_class_asString(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT (1);
      if (args[0] == string_class)
	return String::fresh(SYMBOL(LT_String_GT));
      else
      {
	FETCH_ARG(0, String);
        return arg0;
      }
    }

    static rObject
    string_class_asPrintable(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(1);
      FETCH_ARG(0, String);
      return String::fresh(
        libport::Symbol('"'
                        + string_cast(libport::escape(arg0->value_get().name_get()))
                        + '"'));

    }

    /// Clone.
    static rObject
    string_class_clone(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(1);
      FETCH_ARG(0, String);
      return arg0->clone();
    }


    /// Change the value.
    static rObject
    string_class_set(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(2);
      FETCH_ARG(0, String);
      FETCH_ARG(1, String);
      arg0->value_set (arg1->value_get());
      return arg0;
    }

    static rObject
    string_class_split(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT (2);
      boost::tokenizer< boost::char_separator<char> > tok =
	libport::make_tokenizer(args[0]->value<String>().name_get(),
                                args[1]->value<String>().name_get().c_str());
      list_traits::type ret;
      foreach(std::string i, tok)
	ret.push_back(String::fresh(libport::Symbol(i)));
      return List::fresh(ret);
    }

    static rObject
    string_class_fresh (runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT (1);
      FETCH_ARG (0, String);
      return String::fresh(libport::Symbol::fresh (arg0->value_get ()));
    }
  }

#define PRIMITIVE_1_STRING(Name)                  \
  PRIMITIVE_1(string, Name, String)

#define PRIMITIVE_2_STRING(Name, Type2)		\
  PRIMITIVE_2(string, Name, String, Type2)

  PRIMITIVE_2_STRING(EQ_EQ, String);

  PRIMITIVE_2_STRING(LT, String);

  PRIMITIVE_1_STRING(size);

#undef PRIMITIVE_2_STRING
#undef PRIMITIVE_1_STRING

  void
  string_class_initialize ()
  {
    // String.asString was already defined to function () {self} in
    // CLASS_INIT in primitives.cc
    string_class->slot_remove(SYMBOL(asString));
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(string, Name)
    DECLARE(EQ_EQ);
    DECLARE(LT);
    DECLARE(PLUS);
    DECLARE(asPrintable);
    DECLARE(asString);
    DECLARE(clone);
    DECLARE(fresh);
    DECLARE(set);
    DECLARE(size);
    DECLARE(split);
#undef DECLARE

  }
}; // namespace object
