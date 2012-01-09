/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <cctype>

#include <libport/lexical-cast.hh>

#include <object/format-info.hh>
#include <urbi/object/global.hh>
#include <urbi/object/symbols.hh>

namespace urbi
{
  namespace object
  {
    FormatInfo::FormatInfo()
      : alignment_(Align::RIGHT)
      , alt_(false)
      , consistent_(true)
      , group_("")
      , pad_(" ")
      , pattern_("%s")
      , precision_(6)
      , prefix_("")
      , spec_("s")
      , uppercase_(Case::UNDEFINED)
      , width_(0)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    FormatInfo::FormatInfo(rFormatInfo model)
      : alignment_(model->alignment_)
      , alt_(model->alt_)
      , consistent_(model->consistent_)
      , group_(model->group_)
      , pad_(model->pad_)
      , pattern_(model->pattern_)
      , precision_(model->precision_)
      , prefix_(model->prefix_)
      , spec_(model->spec_)
      , uppercase_(model->uppercase_)
      , width_(model->width_)
    {
      proto_add(model);
    }

    URBI_CXX_OBJECT_INIT(FormatInfo)
      : alignment_(Align::RIGHT)
      , alt_(false)
      , consistent_(true)
      , group_("")
      , pad_(" ")
      , pattern_("%s")
      , precision_(6)
      , prefix_("")
      , spec_("s")
      , uppercase_(Case::UNDEFINED)
      , width_(0)
    {
      bind(SYMBOL(init),     &FormatInfo::init);
      bind(SYMBOL(asString), &FormatInfo::as_string);
      bind(SYMBOL(pattern),  &FormatInfo::pattern_get);

# define DECLARE(Name)                                  \
      bind(SYMBOL_(Name), &FormatInfo::Name ##_get);    \
      property_set(SYMBOL_(Name),                       \
                   SYMBOL(updateHook),                  \
                   primitive(&FormatInfo::update_hook))

      DECLARE(alignment);
      DECLARE(alt);
      DECLARE(group);
      DECLARE(pad);
      DECLARE(precision);
      DECLARE(prefix);
      DECLARE(spec);
      DECLARE(uppercase);
      DECLARE(width);

# undef DECLARE
    }

    void
    FormatInfo::init(const std::string& pattern)
    {
      size_t cursor = 0;
      bool piped;

      if (pattern.empty())
        RAISE("format: pattern is empty");
      if (pattern[0] != '%')
        RAISE("format: pattern \"" + pattern + "\" doesn't begin with %");
      if (pattern.size() == 1)
        RAISE("format: trailing `%'");
      if ((piped = pattern[1] == '|'))
      {
        size_t pos = pattern.find_first_of('|', 2);
        if (pos == pattern.npos)
          RAISE("format: format begins with '|' but "
                "does not end with '|'");
        pattern_ = pattern.substr(0, pos + 1);
        cursor++;
      }

      // Parsing flags.
      std::string flags("-=+#0 '");
      std::string excludes("");
      char current;
      while ((pattern.size() > ++cursor)
             && flags.npos != flags.find(current = pattern[cursor]))
      {
        if (excludes.npos != excludes.find(current))
          FRAISE("format: '%s' conflicts with one of these flags: \"%s\".",
                 current, excludes);
        switch (current)
        {
          case '-': alignment_ = Align::LEFT;   excludes += "-="; break;
          case '=': alignment_ = Align::CENTER; excludes += "-="; break;
          case '+': prefix_ = "+";              excludes += " +"; break;
          case ' ': prefix_ = " ";              excludes += " +"; break;
          case '0': pad_ = "0"; break;
          case '#': alt_ = true; break;
          case '\'': group_ = " "; break;
        };
      }

      // Parsing width.
      std::string substr =
        pattern.substr(cursor,
                       pattern.find_first_not_of("0123456789", cursor) - cursor);
      if (!substr.empty())
      {
        width_ = lexical_cast<size_t>(substr);
        cursor += substr.size();
      }

      // Parsing precision.
      if (cursor < pattern.size() && pattern[cursor] == '.' && cursor++)
      {
        substr = pattern.substr
          (cursor, pattern.find_first_not_of("0123456789", cursor) - cursor);
        if (substr.empty())
          FRAISE("format: unexpected \"%s\", expected width ([1-9][0-9]*).",
                 pattern[cursor]);
        else
        {
          precision_ = lexical_cast<unsigned int>(substr);
          cursor += substr.size();
        }
      }

      // Parsing spec.
      if ((piped && cursor < pattern_.size() - 1)
          || (!piped && cursor < pattern.size()))
      {
        spec_ = tolower(current = pattern[cursor]);
        if (!strchr("sdbxoefEDX", spec_[0]))
          FRAISE("format: \"%s\" is not a valid conversion type character",
                 spec_);
        else if (spec_ != "s")
          uppercase_ = (islower(pattern[cursor])) ? Case::LOWER : Case::UPPER;
      }


      if (piped)
      {
        int overflow = pattern_.size() - 1 - ++cursor;
        if (0 < overflow)
          FRAISE("format: too many characters between pipes and \"%s\"",
                 pattern_.substr(cursor, overflow));
      }
      else
        pattern_ = pattern.substr(0, ++cursor);
    }

    std::string
    FormatInfo::as_string() const
    {
      return pattern_get();
    }

    const std::string&
    FormatInfo::pattern_get() const
    {
      if (!consistent_)
        compute_pattern();
      return pattern_;
    }

    void
    FormatInfo::compute_pattern() const
    {
      pattern_ = std::string("%|")
        + (alignment_ == Align::RIGHT ? ""
           : (alignment_ == Align::LEFT) ? "-" : "=")
        + (alt_ ? "#" : "")
        + (group_ == "" ? "" : "'")
        + (pad_ == " " ? "" : "0")
        + prefix_
        + (width_ == 0 ? "" : string_cast(width_))
        + (precision_ == 6 ? "" : "." + string_cast(precision_))
        + (uppercase_ == Case::UPPER ? char(toupper(spec_[0])): spec_[0])
        + '|';
      consistent_ = true;
    }

    rObject
    FormatInfo::update_hook(const std::string& slot, rObject val)
    {
      if (slot == "alignment")
        switch (Float::int_type v = from_urbi<rFloat>(val)->to_int_type())
        {
#define CASE(In, Out)                                   \
          case In: alignment_ = Align::Out; break
          CASE(-1, LEFT);
          CASE( 0, CENTER);
          CASE( 1, RIGHT);
#undef CASE
          default:
            FRAISE("expected integer -1, 0 or 1, got %s", v);
        }
      else if (slot == "alt")
        alt_ = val->as_bool();
      else if (slot == "group")
      {
        std::string v = from_urbi<rString>(val)->value_get();
        if (!v.empty() && v != " ")
          FRAISE("expected \" \" or \"\", got \"%s\"", v);
        group_ = v;
      }
      else if (slot == "pad")
      {
        std::string v = from_urbi<rString>(val)->value_get();
        if (v != " " && v != "0")
          FRAISE("expected \" \" or \"0\", got \"%s\"", v);
        pad_ = v;
      }
      else if (slot == "precision")
        precision_ = from_urbi<rFloat>(val)->to_unsigned_type();
      else if (slot == "prefix")
      {
        std::string v = from_urbi<rString>(val)->value_get();
        if (v != " " && v != "+" && v != "")
          FRAISE("expected \"\", \" \" or \"+\", got \"%s\"", v);
        prefix_ = v;
      }
      else if (slot == "spec")
      {
        std::string val_(from_urbi<rString>(val)->value_get());
        if (val_.size() != 1)
          RAISE("expected one-character long string, got " + val_);
        if (!strchr("sdbxoefEDX", val_[0]))
          RAISE("expected one character in \"sdbxoefEDX\", got \"" + val_ + "\"");
        spec_ = from_urbi<rString>(val)->to_lower();
        uppercase_ = (spec_ == "s"       ? Case::UNDEFINED
                      : islower(val_[0]) ? Case::LOWER
                      :                    Case::UPPER);
      }
      else if (slot == "uppercase")
        switch (Float::int_type v = from_urbi<rFloat>(val)->to_int_type())
        {
#define CASE(In, Out, Spec)                                     \
          case In: uppercase_ = Case::Out; spec_ = Spec; break
          CASE(-1, LOWER,     "d");
          CASE( 0, UNDEFINED, "s");
          CASE( 1, UPPER,     "D");
#undef CASE
          default:
            FRAISE("expected integer -1, 0 or 1, got %s", v);
        }
      else if (slot == "width")
        width_ = from_urbi<rFloat>(val)->to_unsigned_type();
      else
        return 0;
      consistent_ = false;
      return 0;
    }
  } // namespace object
}
