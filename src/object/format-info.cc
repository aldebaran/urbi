#include <cctype>

#include <libport/lexical-cast.hh>

#include <object/format-info.hh>
#include <object/symbols.hh>

namespace object
{
  FormatInfo::FormatInfo()
    : width_(0)
    , precision_(6)
    , alt_(false)
    , prefix_(0)
    , alignment_(Align::RIGHT)
    , case_(Case::UNDEFINED)
    , group_(0)
    , pad_(' ')
    , spec_('s')
    , pattern_("%s")
  {
    proto_add(proto ? proto : Object::proto);
  }

  FormatInfo::FormatInfo(rFormatInfo model)
    : width_(model->width_)
    , precision_(model->precision_)
    , alt_(model->alt_)
    , prefix_(model->prefix_)
    , alignment_(model->alignment_)
    , case_(model->case_)
    , group_(model->group_)
    , pad_(model->pad_)
    , spec_(model->spec_)
    , pattern_(model->pattern_)
  {
    proto_add(model);
  }

  void
  FormatInfo::init(const std::string& pattern)
  {
    size_t cursor = 0;
    bool piped;

    if (pattern.empty())
      runner::raise_primitive_error("pattern is empty");
    if (pattern[0] != '%')
      runner::raise_primitive_error("format: Pattern \"" + pattern
                                    + "\" doesn't begin with %");
    if (pattern.size() == 1)
      runner::raise_primitive_error("format: Trailing `%'");
    if ((piped = pattern[1] == '|'))
    {
      size_t pos = pattern.find_first_of('|', 2);
      if (pos == pattern.npos)
        runner::raise_primitive_error("format begins with '|' but "
                                     "doesn't end with '|'");
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
// FIXME:
//    if (excludes.npos != excludes.find(current))
//      WARNING;
      switch (current)
      {
        case '-': alignment_ = Align::LEFT; excludes += "-="; break;
        case '=': alignment_ = Align::CENTER; excludes += "-="; break;
        case '+': prefix_ = '+'; excludes + " +"; break;
        case ' ': prefix_ = ' '; excludes + " +"; break;
        case '0': pad_ = '0'; break;
        case '#': alt_ = true; break;
        case '\'': group_ = ' '; break;
      };
    }

    // Parsing width.
    std::string substr_ =
      pattern.substr(cursor,
                     pattern.find_first_not_of("0123456789",
                                               cursor) - cursor);
    if (substr_.size())
    {
      width_ = lexical_cast<size_t>(substr_);
      cursor += substr_.size();
    }

    // Parsing precision.
    if (pattern.size() > cursor && pattern[cursor] == '.' && cursor++)
    {
      substr_ =
        pattern.substr(cursor, pattern.find_first_not_of("0123456789",
                                                         cursor) - cursor);
      if (substr_.size())
      {
        precision_ = lexical_cast<unsigned int>(substr_);
        cursor += substr_.size();
      }
      else
        runner::raise_primitive_error(std::string("format: Unexpected \"")
                                      + pattern[cursor]
                                      + "\", expected width ([1-9][0-9]*).");
    }

    // Parsing spec.
    if ((piped && cursor < pattern_.size() - 1)
        || (!piped && cursor < pattern.size()))
    {
      spec_ = tolower(current = pattern[cursor]);
      substr_ = "sdbxoDX";
      if (substr_.find(spec_) != substr_.npos)
      {
        if (spec_ != 's')
          case_ = (islower(pattern[cursor])) ? Case::LOWER : Case::UPPER;
      }
      else
        runner::raise_primitive_error
          (std::string("format: \"") + spec_
           + "\" is not a valid conversion type character");
    }

    int overflow;
    if (piped)
    {
      if ((overflow = pattern_.size() - 1 - ++cursor) > 0)
        runner::raise_primitive_error("format is between pipes and \""
                                      + pattern_.substr(cursor, overflow)
                                      + "\" are too many characters");
    }
    else
      pattern_ = pattern.substr(0, ++cursor);
  }

  std::string
  FormatInfo::as_string() const
  {
    return pattern_;
  }

  void
  FormatInfo::initialize(CxxObject::Binder<FormatInfo>& bind)
  {
    bind(SYMBOL(init), &FormatInfo::init);
    bind(SYMBOL(asString), &FormatInfo::as_string);
    bind(SYMBOL(width), &FormatInfo::width_get);
    bind(SYMBOL(precision), &FormatInfo::precision_get);
    bind(SYMBOL(alt), &FormatInfo::alt_get);
    bind(SYMBOL(prefix), &FormatInfo::prefix_get);
    bind(SYMBOL(alignment), &FormatInfo::alignment_get);
    bind(SYMBOL(uppercase), &FormatInfo::case_get);
    bind(SYMBOL(group), &FormatInfo::group_get);
    bind(SYMBOL(pad), &FormatInfo::pad_get);
    bind(SYMBOL(spec), &FormatInfo::spec_get);
    bind(SYMBOL(pattern), &FormatInfo::pattern_get);
  }

  rObject
  FormatInfo::proto_make()
  {
    return new FormatInfo();
  }

  URBI_CXX_OBJECT_REGISTER(FormatInfo);
} // namespace object
