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
      , rank_(0)
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
      , rank_(model->rank_)
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
      , rank_(0)
      , spec_("s")
      , uppercase_(Case::UNDEFINED)
      , width_(0)
    {
      BIND(init);
      BIND(asString, as_string);
      BINDG(pattern, pattern_get);

# define DECLARE(Name)                                                  \
      bind(#Name, &FormatInfo::Name ##_get, &FormatInfo::Name ##_set);

      DECLARE(alignment);
      DECLARE(alt);
      DECLARE(group);
      DECLARE(pad);
      DECLARE(precision);
      DECLARE(prefix);
      DECLARE(rank);
      DECLARE(spec);
      DECLARE(uppercase);
      DECLARE(width);

# undef DECLARE
    }

    void
    FormatInfo::init(const std::string& pattern)
    {
      init_(pattern, true);
    }

    void
    FormatInfo::init_(const std::string& pattern, bool check_end)
    {
      if (pattern.empty())
        RAISE("format: empty pattern");
      if (pattern[0] != '%')
        FRAISE("format: pattern does not begin with %%: %s", pattern);
      if (pattern.size() == 1)
        RAISE("format: trailing `%'");

      const char* digits = "0123456789";

      // Cursor inside pattern.
      size_t cursor = 1;

      // Whether between `|'.
      bool piped = pattern[cursor] == '|';
      if (piped)
        ++cursor;

      // Whether a simple positional request: %<rank>%, in which case,
      // no flags admitted.
      bool percented = false;

      // Whether there is a positional argument: <NUM>$, or if
      // it's a %<rank>% (but then, no pipe accepted).
      {
        // First non digit.
        size_t sep = pattern.find_first_not_of(digits, cursor);
        // If there are digits, and then a $, then we have a
        // positional argument.
        if (sep != cursor
            && sep < pattern.size()
            && (pattern[sep] == '$'
                || (!piped && pattern[sep] == '%')))
        {
          percented = pattern[sep] == '%';
          rank_ = lexical_cast<size_t>(pattern.substr(cursor, sep - cursor));
          if (!rank_)
            FRAISE("format: invalid positional argument: %s",
                   pattern.substr(cursor, sep - cursor));
          cursor = sep + 1;
        }
        else
          rank_ = 0;
      }

      // Parsing flags.
      if (!percented)
      {
        std::string flags("-=+#0 '");
        std::string excludes;
        char current;
        for (;
             cursor < pattern.size()
               && libport::has(flags, current = pattern[cursor]);
             ++cursor)
          if (libport::has(excludes, current))
            FRAISE("format: '%s' conflicts with one of these flags: \"%s\"",
                   current, excludes);
          else
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
      if (!percented)
        if (size_t w = pattern.find_first_not_of(digits, cursor) - cursor)
        {
          width_ = lexical_cast<size_t>(pattern.substr(cursor, w));
          cursor += w;
        }

      // Parsing precision.
      if (!percented
          && cursor < pattern.size() && pattern[cursor] == '.')
      {
        ++cursor;
        if (size_t w = pattern.find_first_not_of(digits, cursor) - cursor)
        {
          precision_ = lexical_cast<size_t>(pattern.substr(cursor, w));
          cursor += w;
        }
        else
          FRAISE("format: invalid width after `.': %s", pattern[cursor]);
      }

      // Parsing spec.  Optional if piped.
      if (!percented
          && cursor < pattern.size()
          && (!piped || pattern[cursor] != '|'))
      {
        spec_ = tolower(pattern[cursor]);
        if (!strchr("sdbxoef", spec_[0]))
          FRAISE("format: invalid conversion type character: %s", spec_);
        else if (spec_ != "s")
          uppercase_ = (islower(pattern[cursor])) ? Case::LOWER : Case::UPPER;
        ++cursor;
      }

      if (piped)
      {
        if (cursor < pattern.size())
          ++cursor;
        else {
          RAISE("format: missing closing '|'");
        }
      }
      if (check_end && cursor < pattern.size())
        FRAISE("format: spurious characters after format: %s",
               pattern.substr(cursor));

      pattern_ = pattern.substr(0, cursor);
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
      {
        pattern_ = compute_pattern();
        consistent_ = true;
      }
      return pattern_;
    }

    std::string
    FormatInfo::compute_pattern(bool for_float) const
    {
      // To format floats, we rely on Boost.Format.
      size_t rank = rank_;
      char spec = spec_[0];
      if (for_float)
      {
        rank = 0;
        if (spec == 's' || spec == 'd') spec = 'g';
        if (spec == 'D') spec = 'G';
      }

      return std::string("%|")
        + (rank == 0 ? "" : (string_cast(rank) + "$"))
        + ((alignment_ == Align::RIGHT) ? ""
           : (alignment_ == Align::LEFT) ? "-"
           : "=")
        + (alt_ ? "#" : "")
        + (group_ == "" ? "" : "'")
        + (pad_ == " " ? "" : "0")
        + prefix_
        + (width_ == 0 ? "" : string_cast(width_))
        + (precision_ == 6 ? "" : "." + string_cast(precision_))
        + (uppercase_ == Case::UPPER ? char(toupper(spec)) : spec)
        + '|';
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
    FormatInfo::rank_set(size_t v)
    {
      rank_ = v;
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
