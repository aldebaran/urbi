/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/string.cc
 ** \brief Creation of the Urbi object string.
 */

#include <cctype>

#include <boost/algorithm/string.hpp>
#include <boost/assign.hpp>
#include <libport/format.hh>

#include <libport/damerau-levenshtein-distance.hh>
#include <libport/escape.hh>
#include <libport/lexical-cast.hh>

#include <urbi/object/float.hh>
#if !defined COMPILATION_MODE_SPACE
# include <object/format-info.hh>
#endif
#include <urbi/object/global.hh>
#include <urbi/object/hash.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <urbi/object/symbols.hh>
#include <urbi/runner/raise.hh>
#include <runner/runner.hh>

namespace urbi
{
  namespace object
  {
    /*--------------------.
    | String primitives.  |
    `--------------------*/

    String::String()
      : content_()
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    String::String(rString model)
      : content_(model->content_)
    {
      proto_add(proto);
    }

    String::String(const value_type& v)
      : content_(v)
    {
      // Initialization hack: bootstraping String - and even Primitive
      // before that - instantiates Strings.
      if (!proto)
        proto = new String(FirstPrototypeFlag());
      proto_add(proto);
    }

    // FIXME: kill this when overloading is fully supported
    typedef std::vector<std::string> strings_type;
    OVERLOAD_TYPE(
      split_overload, 4, 1,
      String,
      (strings_type (String::*)(const std::string&, int, bool, bool) const)
      &String::split,
      List,
      (strings_type (String::*)(const strings_type&, int, bool, bool) const)
      &String::split)

    // FIXME: kill this when overloading is fully supported
    static rObject string_split_bouncer(const objects_type& _args)
    {
      objects_type args = _args;
      static rPrimitive actual = new Primitive(split_overload);
      check_arg_count(args.size() - 1, 0, 4);
      switch (args.size())
      {
        case 1:
        {
          static strings_type seps =
            boost::assign::list_of(" ")("\t")("\r")("\n");
          args << to_urbi(seps)
               << new Float(-1)
               << object::false_class
               << object::false_class;
          break;
        }
        case 2:
          args << new Float(-1);
        case 3:
          args << object::false_class;
        case 4:
          args << object::true_class;
        default:
          break;
      }
      if (args[2] == object::nil_class)
        args[2] = new Float(-1);
      return (*actual)(args);
    }

    OVERLOAD_2
    (string_sub_bouncer, 2,
     (std::string (String::*) (unsigned) const) (&String::sub),
     (std::string (String::*) (unsigned, unsigned) const) (&String::sub)
      );

    OVERLOAD_2
    (string_sub_eq_bouncer, 3,
     (std::string (String::*) (unsigned, const std::string&))
     (&String::sub_eq),
     (std::string (String::*) (unsigned, unsigned, const std::string&))
     (&String::sub_eq)
      );

    URBI_CXX_OBJECT_INIT(String)
    {
      bind_variadic(SYMBOL(split), string_split_bouncer);

      bind(SYMBOL(EQ_EQ),
           static_cast<bool (self_type::*)(const rObject&) const>
           (&self_type::operator==));

#define DECLARE(Name, Function)                 \
      bind(SYMBOL_(Name), &String::Function)

      DECLARE(LT_EQ       , operator<=);
      DECLARE(PLUS        , plus);
      DECLARE(STAR        , star);
      DECLARE(asBool      , as_bool);
      DECLARE(asFloat     , as_float);
      DECLARE(asPrintable , as_printable);
      DECLARE(asString    , as_string);
      DECLARE(distance    , distance);
      DECLARE(empty       , empty);
#if !defined COMPILATION_MODE_SPACE
      DECLARE(format      , format);
#endif
      DECLARE(fresh       , fresh);
      DECLARE(fromAscii   , fromAscii);
      DECLARE(hash        , hash);
      DECLARE(isAlnum     , is_alnum);
      DECLARE(isAlpha     , is_alpha);
      DECLARE(isCntrl     , is_cntrl);
      DECLARE(isDigit     , is_digit);
      DECLARE(isGraph     , is_graph);
      DECLARE(isLower     , is_lower);
      DECLARE(isPrint     , is_print);
      DECLARE(isPunct     , is_punct);
      DECLARE(isSpace     , is_space);
      DECLARE(isUpper     , is_upper);
      DECLARE(isXdigit    , is_xdigit);
      DECLARE(join        , join);
      DECLARE(replace     , replace);
      DECLARE(set         , set);
      DECLARE(size        , size);
      DECLARE(toAscii     , toAscii);
      DECLARE(toLower     , to_lower);
      DECLARE(toUpper     , to_upper);

#undef DECLARE

      setSlot(SYMBOL(SBL_SBR), new Primitive(string_sub_bouncer));
      setSlot(SYMBOL(SBL_SBR_EQ), new Primitive(string_sub_eq_bouncer));
    }

    const String::value_type& String::value_get() const
    {
      return content_;
    }

    String::value_type& String::value_get()
    {
      return content_;
    }

    bool
    String::operator<=(const value_type& rhs) const
    {
      return value_get() <= rhs;
    }



    String::size_type
    String::distance(const std::string& other) const
    {
      return libport::damerau_levenshtein_distance(value_get(), other);
    }

    std::string
    String::plus(rObject rhs) const
    {
      return content_ + rhs->as_string();
    }

    String::size_type
    String::size() const
    {
      return content_.size();
    }

    bool
    String::empty() const
    {
      return content_.empty();
    }

    bool
    String::as_bool() const
    {
      return !empty();
    }

    libport::ufloat
    String::as_float() const
    {
      try
      {
        return boost::lexical_cast<libport::ufloat>(content_);
      }
      catch (const boost::bad_lexical_cast&)
      {
        FRAISE("cannot convert to float: %s", as_printable());
      }
    }

    std::string
    String::as_printable() const
    {
      // See https://svn.boost.org/trac/boost/ticket/6264, we cannot
      // use string_cast here since Boost 1.48.
      return libport::format("\"%s\"", libport::escape(content_, '"'));
    }

#if !defined COMPILATION_MODE_SPACE
    std::string
    String::format(rFormatInfo finfo) const
    {
      std::string res(!finfo->uppercase_get() ? content_
                      : finfo->uppercase_get() > 0 ? to_upper()
                      : to_lower());

      // Number of padding chars to add.
      size_t size = res.size();
      int padsize = finfo->width_get() - size;
      if (0 < padsize)
      {
        // Where to insert the padding.
        size_t pos =
          (finfo->alignment_get() == FormatInfo::Align::LEFT ? size
           : finfo->alignment_get() == FormatInfo::Align::RIGHT ? 0
           : (size + 1) / 2);
        res.insert(pos,
                   std::string(padsize, finfo->pad_get()[0]));
      }
      return res;
    }
#endif

    std::string
    String::as_string() const
    {
      return content_;
    }

    const std::string&
    String::set(const std::string& rhs)
    {
      return content_ = rhs;
    }

    std::string
    String::fresh() const
    {
      return libport::Symbol::fresh_string(value_get());
    }

    std::string
    String::replace(const std::string& from, const std::string& to) const
    {
      return boost::replace_all_copy(value_get(), from, to);
    }


    String::value_type
    String::join(const objects_type& os,
                 const std::string& prefix, const std::string& suffix) const
    {
      value_type res = prefix;
      bool first = true;
      foreach (const rObject& o, os)
      {
        if (!first)
          res += content_;
        first = false;
        res += o->as_string();
      }
      res += suffix;
      return res;
    }


    typedef std::vector<std::string> strings_type;

    static
    size_t
    find_first(const strings_type& seps,
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

    strings_type
    String::split(const strings_type& sep, int limit,
                  bool keep_delim, bool keep_empty) const
    {
      strings_type res;

      // Special case: splitting on an empty string returns the individual
      // characters.
      if (libport::has(sep, ""))
      {
        for (std::string::size_type pos = 0; pos != content_.size(); ++pos)
          res.push_back(content_.substr(pos, 1));
        return res;
      }

      size_t start = 0;
      std::string delim;
      for (size_t end = find_first(sep, content_, start, delim);
           end != std::string::npos && limit;
           end = find_first(sep, content_, start, delim), --limit)
      {
        std::string sub = content_.substr(start, end - start);
        if (keep_empty || !sub.empty())
          res << sub;
        if (keep_delim)
          res << delim;
        start = end + delim.length();
      }

      if (start < content_.size() || keep_empty)
        res << content_.substr(start);

      return res;
    }

    strings_type
    String::split(const std::string& sep, int limit,
                  bool keep_delim, bool keep_empty) const
    {
      strings_type seps;
      seps << sep;
      return split(seps, limit, keep_delim, keep_empty);
    }

    std::string
    String::star(size_type times) const
    {
      std::string res;
      res.reserve(times * size());
      for (size_type i = 0; i < times; i++)
        res += value_get();
      return res;
    }

    void String::check_bounds(size_type from, size_type to) const
    {
      if (from >= content_.length())
        FRAISE("invalid index: %s", from);
      if (to >  content_.length())
        FRAISE("invalid index: %s", to);
      if (from > to)
        FRAISE("range starting after its end does not make sense: %s, %s",
               from, to);
    }

    std::string String::sub(size_type idx) const
    {
      return sub(idx, idx + 1);
    }

    std::string String::sub_eq(size_type idx, const std::string& v)
    {
      return sub_eq(idx, idx + 1, v);
    }

    std::string String::sub(size_type from, size_type to) const
    {
      check_bounds(from, to);
      return content_.substr(from, to - from);
    }

    std::string
    String::sub_eq(size_type from, size_type to, const std::string& v)
    {
      check_bounds(from, to);
      content_ = (content_.substr(0, from)
                  + v
                  + content_.substr(to, std::string::npos));
      return v;
    }

    std::string String::to_lower() const
    {
      return boost::to_lower_copy(value_get());
    }

    std::string String::to_upper() const
    {
      return boost::to_upper_copy(value_get());
    }

    std::string String::fromAscii(rObject, unsigned char code)
    {
      std::string res;
      res += code;
      return res;
    }

#define IS(Spec)                                \
    bool String::is_ ## Spec() const            \
    {                                           \
      foreach (char c, content_)                \
        if (!is ## Spec(c))                     \
          return false;                         \
      return true;                              \
    }

    IS(alnum)
    IS(alpha)
    IS(cntrl)
    IS(digit)
    IS(graph)
    IS(lower)
    IS(print)
    IS(punct)
    IS(space)
    IS(upper)
    IS(xdigit)
#undef IS

    unsigned char String::toAscii() const
    {
      check_bounds(0, 1);
      return value_get()[0];
    }

    rHash
    String::hash() const
    {
      return new Hash(boost::hash_value(value_get()));
    }

  } // namespace object
}
