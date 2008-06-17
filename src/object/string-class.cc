/**
 ** \file object/string-class.cc
 ** \brief Creation of the URBI object string.
 */

#include <libport/escape.hh>
#include <libport/lexical-cast.hh>

#include <libport/tokenizer.hh>

#include <object/atom.hh>
#include <object/float-class.hh>
#include <object/list-class.hh>
#include <object/object.hh>
#include <object/primitives.hh>
#include <runner/runner.hh>
#include <object/string-class.hh>

namespace object
{
  rObject string_class;

  /*--------------------.
  | String primitives.  |
  `--------------------*/

  String::String()
    : content_()
  {}

  String::String(rString model)
    : content_(model->content_)
  {}

  String::String(const value_type& v)
    : content_(v)
  {
    assert(string_class);
    proto_add(string_class);
  }

  const String::value_type& String::value_get() const
  {
    return content_;
  }

  String::value_type& String::value_get()
  {
    return content_;
  }

  rString String::operator+ (rString rhs)
  {
    return new String(libport::Symbol(content_.name_get()
                                      + rhs->value_get().name_get()));
  }

  rFloat String::size ()
  {
    return new Float(content_.name_get().length());
  }

  rString as_printable (rObject from)
  {
    if (from.get() == string_class.get())
      return as_string(from);
    else
    {
      type_check<String>(from, SYMBOL(asPrintable));
      rString str = from->as<String>();
      return new String(
        libport::Symbol('"'
                        + string_cast(libport::escape(str->value_get()))
                        + '"'));
    }
  }

  rString
  as_string (rObject from)
  {
    if (from.get() == string_class.get())
      return new String(SYMBOL(LT_String_GT));
    else
    {
      type_check<String>(from, SYMBOL(asString));
      return from->as<String>();
    }
  }

  rObject
  String::lt(rString rhs)
  {
    return value_get().name_get() < rhs->value_get().name_get() ?
      true_class : false_class;
  }

  rString
  String::set(rString rhs)
  {
    content_ = rhs->value_get();
    return this;
  }

  rString
  String::fresh ()
  {
    return new String(libport::Symbol::fresh(value_get()));
  }

  rList
  String::split(rString sep)
  {
    boost::tokenizer< boost::char_separator<char> > tok =
      libport::make_tokenizer(value_get().name_get(),
                              sep->value_get().name_get().c_str());
  List::value_type ret;
    foreach(const std::string& i, tok)
      ret.push_back(new String(libport::Symbol(i)));
    return new List(ret);
  }

  void String::initialize(CxxObject::Binder<String>& bind)
  {
    bind(SYMBOL(asPrintable), &as_printable);
    bind(SYMBOL(asString), &as_string);
    bind(SYMBOL(fresh), &String::fresh);
    bind(SYMBOL(LT), &String::lt);
    bind(SYMBOL(PLUS), &String::operator+);
    bind(SYMBOL(set), &String::set);
    bind(SYMBOL(size), &String::size);
    bind(SYMBOL(split), &String::split);
  }

  bool String::string_added = CxxObject::add<String>("String", string_class);
  const std::string String::type_name = "String";
  std::string String::type_name_get() const
  {
    return type_name;
  }

}; // namespace object
