/**
 ** \file object/string-class.cc
 ** \brief Creation of the URBI object string.
 */

#include <boost/algorithm/string.hpp>
#include <boost/multi_array.hpp>

#include <libport/escape.hh>
#include <libport/lexical-cast.hh>
#include <libport/tokenizer.hh>

#include <object/float.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/primitives.hh>
#include <object/string.hh>

#include <runner/call.hh>
#include <runner/runner.hh>

namespace object
{
  /*--------------------.
  | String primitives.  |
  `--------------------*/

  String::String()
    : content_()
  {
    proto_add(proto ? proto : object_class);
  }

  String::String(rString model)
    : content_(model->content_)
  {
    proto_add(proto);
  }

  String::String(const value_type& v)
    : content_(v)
  {
    assert(proto);
    proto_add(proto);
  }

  const String::value_type& String::value_get() const
  {
    return content_;
  }

  String::value_type& String::value_get()
  {
    return content_;
  }

  static unsigned int
  damerau_levenshtein_distance(const std::string& s1, const std::string& s2)
  {
    boost::multi_array<unsigned int, 2> d(boost::extents[s1.size()+1]
					                [s2.size()+1]);

    for (unsigned int i = 0; i <= s1.size(); ++i)
      d[i][0] = i;
    for (unsigned int j = 1; j <= s2.size(); ++j)
      d[0][j] = j;

    for (unsigned int i = 1; i <= s1.size(); ++i)
      for (unsigned int j = 1; j <= s2.size(); ++j)
      {
	unsigned int cost = s1[i-1] == s2[j-1] ? 0 : 1;
	d[i][j] = std::min(std::min(d[i-1][j] + 1,        // Deletion
				    d[i][j-1] + 1),       // Insertion
			   d[i-1][j-1] + cost);           // Substitution
	if (i > 1 && j > 1 && s1[i-1] == s2[j-2] && s1[i-2] == s2[j-1])
	  d[i][j] = std::min(d[i][j],
			     d[i-2][j-2] + cost);         // Transposition
      }
    return d[s1.size()][s2.size()];
  }

  unsigned int
  String::distance(rString other)
  {
    return damerau_levenshtein_distance(value_get(), other->value_get());
  }

  std::string String::plus (runner::Runner& r, rObject rhs)
  {
    rObject str = urbi_call(r, rhs, SYMBOL(asString));
    type_check(str, String::proto);
    return content_ + str->as<String>()->value_get();
  }

  String::size_type
  String::size()
  {
    return content_.size();
  }

  std::string
  as_printable (rObject from)
  {
    type_check(from, String::proto);
    const std::string& str = from->as<String>()->value_get();
    return '"' + string_cast(libport::escape(str)) + '"';
  }

  std::string
  as_string (rObject from)
  {
    type_check(from, String::proto);
    return from->as<String>()->value_get();
  }

  bool
  String::lt(const std::string& rhs)
  {
    return value_get() < rhs;
  }

  std::string
  String::set(const std::string& rhs)
  {
    content_ = libport::Symbol(rhs);
    return rhs;
  }

  std::string
  String::fresh ()
  {
    return libport::Symbol::fresh(libport::Symbol(value_get())).name_get();
  }

  rList
  String::split(const std::string& sep)
  {
    boost::tokenizer< boost::char_separator<char> > tok =
      libport::make_tokenizer(value_get(),
                              sep.c_str());
    List::value_type ret;
    foreach(const std::string& i, tok)
      ret.push_back(new String(i));
    return new List(ret);
  }

  std::string
  String::star(unsigned int times)
  {
    std::string res;
    res.reserve(times * size());
    for (unsigned int i = 0; i < times; i++)
      res += value_get();
    return res;
  }

  std::string String::format(runner::Runner& r, rList _values)
  {
    const char* str = content_.c_str();
    const char* next;
    std::string res;

    const objects_type& values = _values->value_get();
    objects_type::const_iterator it  = values.begin();
    objects_type::const_iterator end = values.end();

    while ((next = strchr(str, '%')))
    {
      res += std::string(str, next - str);
      str = next + 2;
      const char format = next[1];
      if (format == 0)
        throw PrimitiveError(SYMBOL(PERCENT), "Trailing %");
      switch (format)
      {
        case '%':
          res += '%';
          break;
        case 's':
        {
          if (it == end)
            throw PrimitiveError(SYMBOL(PERCENT), "Too few arguments for format");
          rObject as_string = urbi_call(r, *it, SYMBOL(asString));
          res += as_string->as<String>()->value_get();
          it++;
          break;
        }
        default:
          throw PrimitiveError(SYMBOL(PERCENT),
                               std::string("Unrecognized format: %") + format);
          break;
      }
    }
    if (it != end)
      throw PrimitiveError(SYMBOL(PERCENT), "Too many arguments for format");
    res += str;
    return res;
  }

  void String::check_bounds(unsigned int from, unsigned int to,
                            const libport::Symbol& msg)
  {
    if (from >= content_.length())
      throw PrimitiveError(msg, "invalid index: " + string_cast(from));
    if (to >  content_.length())
      throw PrimitiveError(msg, "invalid index: " + string_cast(to));
    if (from > to)
      throw PrimitiveError(msg, "range starting after its end does not make sense: "
                           + string_cast(from) + ", " + string_cast(to));
  }


  std::string String::sub(unsigned int idx)
  {
    return sub(idx, idx + 1);
  }

  std::string String::sub_eq(unsigned int idx, const std::string& v)
  {
    return sub_eq(idx, idx + 1, v);
  }

  std::string String::sub(unsigned int from, unsigned int to)
  {
    check_bounds(from, to, SYMBOL(SBL_SBR));
    return content_.substr(from, to - from);
  }

  std::string String::to_lower()
  {
    return boost::to_lower_copy(value_get());
  }

  std::string String::to_upper()
  {
    return boost::to_upper_copy(value_get());
  }

  std::string String::sub_eq(unsigned int from, unsigned int to,
                             const std::string& v)
  {
    check_bounds(from, to, SYMBOL(SBL_SBR_EQ));
    content_ = content_.substr(0, from)
      + v
      + content_.substr(to, std::string::npos);
    return content_;
  }

  OVERLOAD_2(sub_bouncer, SBL_SBR,
             2,
             (std::string (String::*) (unsigned)) (&String::sub),
             (std::string (String::*) (unsigned, unsigned)) (&String::sub)
    );

  OVERLOAD_2(sub_eq_bouncer, SBL_SBR_EQ,
             3,
             (std::string (String::*) (unsigned, const std::string&))
             (&String::sub_eq),
             (std::string (String::*) (unsigned, unsigned, const std::string&))
             (&String::sub_eq)
    );

  void String::initialize(CxxObject::Binder<String>& bind)
  {
    bind(SYMBOL(asPrintable), &as_printable);
    bind(SYMBOL(asString), &as_string);
    bind(SYMBOL(distance), &String::distance);
    bind(SYMBOL(fresh), &String::fresh);
    bind(SYMBOL(LT), &String::lt);
    bind(SYMBOL(PERCENT), &String::format);
    bind(SYMBOL(PLUS), &String::plus);
    bind(SYMBOL(set), &String::set);
    bind(SYMBOL(size), &String::size);
    bind(SYMBOL(split), &String::split);
    bind(SYMBOL(STAR), &String::star);
    bind(SYMBOL(toLower), &String::to_lower);
    bind(SYMBOL(toUpper), &String::to_upper);

    proto->slot_set(SYMBOL(SBL_SBR), new Primitive(sub_bouncer));
    proto->slot_set(SYMBOL(SBL_SBR_EQ), new Primitive(sub_eq_bouncer));
  }

  bool String::string_added = CxxObject::add<String>();
  const std::string String::type_name = "String";
  rObject String::proto;

  std::string String::type_name_get() const
  {
    return type_name;
  }

  rObject
  String::proto_make()
  {
    return new String();
  }

}; // namespace object
