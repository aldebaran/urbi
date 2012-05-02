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

#include <libport/cctype>

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
    OVERLOAD_TYPE(
      split_overload, 4, 1,
      String, (String::split_string_type) &String::split,
      List,   (String::split_list_type)   &String::split)

    // FIXME: kill this when overloading is fully supported
    static rObject string_split_bouncer(const objects_type& _args)
    {
      objects_type args = _args;
      static rPrimitive actual = new Primitive(split_overload);
      check_arg_count(args, 0, 4);
      switch (args.size())
      {
        case 1:
        {
          static String::strings_type seps =
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
     (String::value_type (String::*) (ufloat) const) (&String::sub),
     (String::value_type (String::*) (ufloat, ufloat) const) (&String::sub)
      );

    OVERLOAD_2
    (string_sub_eq_bouncer, 3,
     (String::value_type (String::*) (ufloat, const String::value_type&))
     (&String::sub_eq),
     (String::value_type (String::*) (ufloat, ufloat, const String::value_type&))
     (&String::sub_eq)
      );

    URBI_CXX_OBJECT_INIT(String)
      : content_()
    {
      proto = this;
      Ward w(this);
      bind_variadic(SYMBOL(split), string_split_bouncer);

      BIND(EQ_EQ, operator==, bool, (const rObject&) const);
      BIND(LT_EQ , operator<=);
      BIND(PLUS , plus);
      BIND(STAR , star);
      BIND(asBool , as_bool);
      BIND(asFloat , as_float);
      BIND(asPrintable, as_printable);
      BIND(asString);
      BIND(distance);
      BINDG(empty);
#if !defined COMPILATION_MODE_SPACE
      BIND(format);
#endif
      BIND(fresh);
      BIND(fromAscii);
      BIND(hash);
      BIND(isAlnum, is_alnum);
      BIND(isAlpha, is_alpha);
      BIND(isAscii, is_ascii);
      BIND(isBlank, is_blank);
      BIND(isCntrl, is_cntrl);
      BIND(isDigit, is_digit);
      BIND(isGraph, is_graph);
      BIND(isLower, is_lower);
      BIND(isPrint, is_print);
      BIND(isPunct, is_punct);
      BIND(isSpace, is_space);
      BIND(isUpper, is_upper);
      BIND(isXdigit, is_xdigit);
      BIND(join);
      BIND(replace);
      BIND(set);
      BINDG(size);
      BIND(toAscii);
      BIND(toLower, to_lower);
      BIND(toUpper, to_upper);

      BIND(find);
      BIND(find, find_default);
      BIND(rfind);
      BIND(rfind, rfind_default);

      setSlot(SYMBOL(SBL_SBR), new Primitive(string_sub_bouncer));
      setSlot(SYMBOL(SBL_SBR_EQ), new Primitive(string_sub_eq_bouncer));
    }

    String::size_type
    String::index(ufloat idx, bool large) const
    {
      int i = Float::to_int_type(idx, "invalid index: %s");
      // When working on end, prefer size() to 0.
      if (i < 0 || (large && i == 0))
        i += content_.size();
      if (i < 0
          || ((large ? 1 : 0) + content_.size()) < static_cast<size_type>(i))
        FRAISE("invalid index: %s", idx);
      return static_cast<size_type>(i);
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
    String::distance(const value_type& other) const
    {
      return libport::damerau_levenshtein_distance(value_get(), other);
    }

    String::value_type
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
        return libport::as_ufloat(content_);
      }
      catch (const boost::bad_lexical_cast&)
      {
        FRAISE("invalid number: %s", as_printable());
      }
    }

    String::value_type
    String::as_printable() const
    {
      // See https://svn.boost.org/trac/boost/ticket/6264, we cannot
      // use string_cast here since Boost 1.48.
      return libport::format("\"%s\"", libport::escape(content_, '"'));
    }

#if !defined COMPILATION_MODE_SPACE
    String::value_type
    String::format(rFormatInfo finfo) const
    {
      value_type res(!finfo->uppercase_get() ? content_
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
                   value_type(padsize, finfo->pad_get()[0]));
      }
      return res;
    }
#endif

    String::value_type
    String::as_string() const
    {
      return content_;
    }

    rString
    String::asString()
    {
      return this;
    }

    const String::value_type&
    String::set(const value_type& rhs)
    {
      return content_ = rhs;
    }

    String::value_type
    String::fresh() const
    {
      return libport::fresh_string(value_get());
    }

    long
    String::find(const value_type& pattern, long pos) const
    {
      size_t res = value_get()
        .find(pattern, pos == -1 ? value_type::npos : size_t(pos));
      return res == value_type::npos ? -1 : long(res);
    }

    long
    String::rfind(const value_type& pattern, long pos) const
    {
      size_t res = value_get()
        .rfind(pattern, pos == -1 ? value_type::npos : size_t(pos));
      return res == value_type::npos ? -1 : long(res);
    }

    long
    String::find_default(const value_type& pattern) const
    {
      return find(pattern);
    }

    long
    String::rfind_default(const value_type& pattern) const
    {
      return rfind(pattern);
    }

    String::value_type
    String::replace(const value_type& from, const value_type& to) const
    {
      return boost::replace_all_copy(value_get(), from, to);
    }


    String::value_type
    String::join(const objects_type& os,
                 const value_type& prefix, const value_type& suffix) const
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

    static
    size_t
    find_first(const String::strings_type& seps,
               const String::value_type& str,
               size_t start,
               String::value_type& delim)
    {
      typedef String::value_type value_type;
      size_t res = value_type::npos;
      delim = "";

      foreach (const value_type& sep, seps)
      {
        size_t pos = str.find(sep, start);
        if (pos != value_type::npos && (pos < res || res == value_type::npos))
        {
          res = pos;
          delim = sep;
        }
      }
      return res;
    }

    String::strings_type
    String::split(const strings_type& sep, int limit,
                  bool keep_delim, bool keep_empty) const
    {
      strings_type res;

      // Special case: splitting on an empty string returns the individual
      // characters.
      if (libport::has(sep, ""))
      {
        foreach (char c, content_)
          res << value_type(1, c);
        return res;
      }
      else
      {
        size_t start = 0;
        value_type delim;
        for (size_t end = find_first(sep, content_, start, delim);
             end != value_type::npos && limit;
             end = find_first(sep, content_, start, delim), --limit)
        {
          value_type sub = content_.substr(start, end - start);
          if (keep_empty || !sub.empty())
            res << sub;
          if (keep_delim)
            res << delim;
          start = end + delim.length();
        }

        if (start < size() || keep_empty)
          res << content_.substr(start);
      }
      return res;
    }

    String::strings_type
    String::split(const value_type& sep, int limit,
                  bool keep_delim, bool keep_empty) const
    {
      strings_type seps;
      seps << sep;
      return split(seps, limit, keep_delim, keep_empty);
    }

    String::value_type
    String::star(size_type times) const
    {
      value_type res;
      res.reserve(times * size());
      for (size_type i = 0; i < times; i++)
        res += value_get();
      return res;
    }

    void String::check_bounds(ufloat f, ufloat t) const
    {
      size_t from = index(f);
      size_t to = index(t, true);
      if (size() <= from)
        FRAISE("invalid index: %s", from);
      if (size() < to)
        FRAISE("invalid index: %s", to);
      if (to < from)
        FRAISE("range starting after its end: %s, %s", from, to);
    }

    String::value_type String::sub(ufloat idx) const
    {
      return sub(idx, idx + 1);
    }

    String::value_type String::sub_eq(ufloat idx, const value_type& v)
    {
      return sub_eq(idx, idx + 1, v);
    }

    String::value_type String::sub(ufloat from, ufloat to) const
    {
      check_bounds(from, to);
      return content_.substr(index(from), index(to, true) - index(from));
    }

    String::value_type
    String::sub_eq(ufloat from, ufloat to, const value_type& v)
    {
      check_bounds(from, to);
      content_ = (content_.substr(0, index(from))
                  + v
                  + content_.substr(index(to, true), value_type::npos));
      return v;
    }

    String::value_type String::to_lower() const
    {
      return boost::to_lower_copy(value_get());
    }

    String::value_type String::to_upper() const
    {
      return boost::to_upper_copy(value_get());
    }

    String::value_type String::fromAscii(rObject, unsigned char code)
    {
      value_type res;
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
    IS(ascii)
    IS(blank)
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
