/**
 ** \file object/string-class.cc
 ** \brief Creation of the URBI object string.
 */

#include <boost/algorithm/string.hpp>
#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/multi_array.hpp>

#include <libport/escape.hh>
#include <libport/lexical-cast.hh>

#include <object/float.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/string.hh>
#include <object/symbols.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>

namespace object
{
  /*--------------------.
  | String primitives.  |
  `--------------------*/

  String::String()
    : content_()
  {
    proto_add(proto ? proto : Object::proto);
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

  std::string String::plus (rObject rhs)
  {
    rObject str = rhs->call(SYMBOL(asString));
    type_check<String>(str);
    return content_ + str->as<String>()->value_get();
  }

  String::size_type
  String::size()
  {
    return content_.size();
  }

  float
  String::as_float ()
  {
    try
    {
      return boost::lexical_cast<float>(content_);
    }
    catch (const boost::bad_lexical_cast&)
    {
      boost::format fmt("unable to convert to float: %s");
      RAISE(str(fmt % as_printable()));
    }
  }

  std::string
  String::as_printable ()
  {
    return '"' + string_cast(libport::escape(content_, '"')) + '"';
  }

  std::string
  String::as_string ()
  {
    return content_;
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

  static size_t
  find_first(const std::vector<std::string>& seps,
             const std::string& str,
             size_t start,
             std::string& delim)
  {
    size_t res = std::string::npos;
    delim = "";

    foreach (const std::string& sep, seps)
    {
      size_t pos = str.find(sep, start);
      if (pos != std::string::npos && (pos < res || res == std::string::npos))
      {
        res = pos;
        delim = sep;
      }
    }
    return res;
  }

  std::vector<std::string>
  String::split(const std::vector<std::string>& sep, int limit,
                bool keep_delim, bool keep_empty)
  {
    std::vector<std::string> res;
    size_t start = 0;
    size_t end;
    std::string delim;

    // Special case: splitting on an empty string returns the individual
    // characters.
    if (libport::has(sep, ""))
    {
      for (std::string::size_type pos = 0; pos != content_.size(); pos++)
	res.push_back(content_.substr(pos, 1));
      return res;
    }

    for (end = find_first(sep, content_, start, delim);
         end != std::string::npos && limit;
         end = find_first(sep, content_, start, delim), --limit)
    {
      std::string sub = content_.substr(start, end - start);
      if (keep_empty || sub != "")
        res << sub;
      if (keep_delim)
        res << delim;
      start = end + delim.length();
    }

    if (start < content_.size() || keep_empty)
      res << content_.substr(start);

    return res;
  }

  std::vector<std::string>
  String::split(const std::string& sep, int limit,
                bool keep_delim, bool keep_empty)
  {
    std::vector<std::string> seps;
    seps << sep;
    return split(seps, limit, keep_delim, keep_empty);
  }

  OVERLOAD_TYPE(split_overload, 4, 1,
                String,
                (std::vector<std::string>
                 (String::*)(const std::string&, int, bool, bool))
                  &String::split,
                List,
                (std::vector<std::string>
                 (String::*)(const std::vector<std::string>&, int, bool, bool))
                 &String::split)

  static rObject split_bouncer(const objects_type& _args)
  {
    objects_type args = _args;
    static rPrimitive actual = make_primitive(split_overload);
    check_arg_count(args.size() - 1, 0, 4);
    switch (args.size())
    {
      case 1:
      {
	static std::vector<std::string> seps =
	  boost::assign::list_of(" ")("\t")("\r")("\n");
        args.push_back(to_urbi(seps));
        args.push_back(new Float(-1));
        args.push_back(object::false_class);
        args.push_back(object::false_class);
        break;
      }
      case 2:
        args.push_back(new Float(-1));
      case 3:
        args.push_back(object::false_class);
      case 4:
        args.push_back(object::true_class);
      default:
        break;
    }
    if (args[2] == object::nil_class)
      args[2] = new Float(-1);
    return (*actual)(args);
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

  template <typename It>
  static std::string
  str_format(const std::string& fmt, It begin, It end)
  {
    std::string res;
    const char* next;
    const char* str = fmt.c_str();

    while ((next = strchr(str, '%')))
    {
      res += std::string(str, next - str);
      str = next + 2;
      const char format = next[1];
      if (format == 0)
	RAISE("Trailing `%'");
      switch (format)
      {
        case '%':
          res += '%';
          break;
        case 's':
        {
          if (begin == end)
	    RAISE("Too few arguments for format");
          res += from_urbi<std::string>((*begin)->call(SYMBOL(asString)));
          begin++;
          break;
        }
        default:
	  RAISE(std::string("Unrecognized format: `%") + format + "'");
          break;
      }
    }
    if (begin != end)
      RAISE("Too many arguments for format");
    res += str;
    return res;
  }

  std::string String::format(rObject value)
  {
    rList l = value->as<List>();
    if (l)
      return str_format(content_, l->value_get().begin(), l->value_get().end());
    else
      return str_format(content_, &value, &value + 1);
  }

  void String::check_bounds(unsigned int from, unsigned int to)
  {
    if (from >= content_.length())
      RAISE("invalid index: " + string_cast(from));
    if (to >  content_.length())
      RAISE("invalid index: " + string_cast(to));
    if (from > to)
      RAISE("range starting after its end does not make sense: "
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
    check_bounds(from, to);
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
    check_bounds(from, to);
    content_ = content_.substr(0, from)
      + v
      + content_.substr(to, std::string::npos);
    return v;
  }

  std::string String::fromAscii(rObject, int code)
  {
    std::string res;
    res += code;
    return res;
  }

  OVERLOAD_2(sub_bouncer,
             2,
             (std::string (String::*) (unsigned)) (&String::sub),
             (std::string (String::*) (unsigned, unsigned)) (&String::sub)
    );

  OVERLOAD_2(sub_eq_bouncer,
             3,
             (std::string (String::*) (unsigned, const std::string&))
             (&String::sub_eq),
             (std::string (String::*) (unsigned, unsigned, const std::string&))
             (&String::sub_eq)
    );

  URBI_CXX_OBJECT_REGISTER(String);

  void String::initialize(CxxObject::Binder<String>& bind)
  {
    bind(SYMBOL(asFloat), &String::as_float);
    bind(SYMBOL(asPrintable), &String::as_printable);
    bind(SYMBOL(asString), &String::as_string);
    bind(SYMBOL(distance), &String::distance);
    bind(SYMBOL(fresh), &String::fresh);
    bind(SYMBOL(fromAscii), &String::fromAscii);
    bind(SYMBOL(LT), &String::lt);
    bind(SYMBOL(PERCENT), &String::format);
    bind(SYMBOL(PLUS), &String::plus);
    bind(SYMBOL(set), &String::set);
    bind(SYMBOL(size), &String::size);
    bind(SYMBOL(split), split_bouncer);
    bind(SYMBOL(STAR), &String::star);
    bind(SYMBOL(toLower), &String::to_lower);
    bind(SYMBOL(toUpper), &String::to_upper);

    proto->slot_set(SYMBOL(SBL_SBR), new Primitive(sub_bouncer));
    proto->slot_set(SYMBOL(SBL_SBR_EQ), new Primitive(sub_eq_bouncer));
  }

  rObject
  String::proto_make()
  {
    return new String();
  }

}; // namespace object
