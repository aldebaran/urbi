/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
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
#include <object/symbols.hh>

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
      BIND(init);
      BIND(asString, as_string);
      BIND(pattern, pattern_get);

# define DECLARE(Name)                                                  \
      bind(#Name, &FormatInfo::Name ##_get, &FormatInfo::Name ##_set);

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

      if (pattern.empty())
        RAISE("format: pattern is empty");
      if (pattern[0] != '%')
        RAISE("format: pattern \"" + pattern + "\" doesn't begin with %");
      if (pattern.size() == 1)
        RAISE("format: trailing `%'");

      // Cursor inside pattern.
      size_t cursor = 0;
      bool piped;
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
      while ((++cursor < pattern.size())
             && flags.find(current = pattern[cursor]) != flags.npos)
      {
        if (excludes.find(current) != excludes.npos)
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
      {
        std::string substr =
          pattern
          .substr(cursor,
                  pattern.find_first_not_of("0123456789", cursor) - cursor);
        if (!substr.empty())
        {
          width_ = lexical_cast<size_t>(substr);
          cursor += substr.size();
        }
      }

      // Parsing precision.
      if (cursor < pattern.size() && pattern[cursor] == '.')
      {
        ++cursor;
        std::string substr = pattern.substr
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
      if (cursor < pattern_.size() - piped)
      {
        spec_ = tolower(pattern[cursor]);
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

    void
    FormatInfo::alignment_set(Align::position val)
    {
      alignment_ = val;
      consistent_ = false;
    }

    void
    FormatInfo::group_set(std::string v)
    {
      if (!v.empty() && v != " ")
        FRAISE("expected \" \" or \"\": \"%s\"", v);
      group_ = v;
      consistent_ = false;
    }

    void
    FormatInfo::pad_set(std::string v)
    {
      if (v != " " && v != "0")
        FRAISE("expected \" \" or \"0\": \"%s\"", v);
      pad_ = v;
      consistent_ = false;
    }

    void
    FormatInfo::prefix_set(std::string v)
    {
      if (v != " " && v != "+" && v != "")
        FRAISE("expected \"\", \" \" or \"+\": \"%s\"", v);
      prefix_ = v;
      consistent_ = false;
    }

    void
    FormatInfo::spec_set(std::string val_)
    {
      if (val_.size() != 1)
        RAISE("expected one-character long string: " + val_);
      if (!strchr("sdbxoefEDX", val_[0]))
        RAISE("expected one character in \"sdbxoefEDX\": \"" + val_ + "\"");
      spec_ = val_;
      std::transform(spec_.begin(), spec_.end(), spec_.begin(), ::tolower);
      uppercase_ = (spec_ == "s"       ? Case::UNDEFINED
                  : islower(val_[0]) ? Case::LOWER
                  :                    Case::UPPER);
      consistent_ = false;
    }

    void
    FormatInfo::uppercase_set(int v)
    {
      switch (v)
      {
#define CASE(In, Out, Spec)                                     \
        case In: uppercase_ = Case::Out; spec_ = Spec; break
        CASE(-1, LOWER,     "d");
        CASE( 0, UNDEFINED, "s");
        CASE( 1, UPPER,     "D");
#undef CASE
        default:
          FRAISE("expected integer -1, 0 or 1: %s", v);
      }
      consistent_ = false;
    }

    void
    FormatInfo::alt_set(bool v)
    {
      alt_ = v;
      consistent_ = false;
    }

    void
    FormatInfo::precision_set(size_t v)
    {
      precision_ = v;
      consistent_ = false;
    }

    void
    FormatInfo::width_set(size_t v)
    {
      width_ = v;
      consistent_ = false;
    }

  } // namespace object
}
