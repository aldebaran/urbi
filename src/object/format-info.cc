#include <cctype>

#include <libport/lexical-cast.hh>

#include <object/format-info.hh>
#include <object/global.hh>
#include <object/symbols.hh>

namespace object
{
  FormatInfo::FormatInfo()
    : alignment_(Align::RIGHT)
    , alt_(false)
    , case_(Case::UNDEFINED)
    , group_("")
    , pad_(" ")
    , pattern_("%s")
    , precision_(6)
    , prefix_("")
    , spec_("s")
    , width_(0)
    , consistent_(true)
  {
    proto_add(proto ? proto : Object::proto);
  }

  FormatInfo::FormatInfo(rFormatInfo model)
    : alignment_(model->alignment_)
    , alt_(model->alt_)
    , case_(model->case_)
    , group_(model->group_)
    , pad_(model->pad_)
    , pattern_(model->pattern_)
    , precision_(model->precision_)
    , prefix_(model->prefix_)
    , spec_(model->spec_)
    , width_(model->width_)
    , consistent_(model->consistent_)
  {
    proto_add(model);
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
        RAISE(std::string("format: '") + current
              + "' conflics whese one of these flags: \"" + excludes + "\".");
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
      substr =
        pattern.substr(cursor,
                       pattern.find_first_not_of("0123456789", cursor) - cursor);
      if (substr.empty())
        RAISE(std::string("format: unexpected \"")
              + pattern[cursor]
              + "\", expected width ([1-9][0-9]*).");
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
        RAISE(std::string("format: \"") + spec_
              + "\" is not a valid conversion type character");
      else if (spec_ != "s")
        case_ = (islower(pattern[cursor])) ? Case::LOWER : Case::UPPER;
    }

    int overflow;
    if (!piped)
      pattern_ = pattern.substr(0, ++cursor);
    else if (0 < (overflow = pattern_.size() - 1 - ++cursor))
      RAISE("format: format is between pipes and \""
            + pattern_.substr(cursor, overflow)
            + "\" are too many characters");
  }

  std::string
  FormatInfo::as_string(const objects_type& args)
  {
    if (args.size() > 1)
      runner::raise_arity_error(args.size(), 0, 1);
    if (args.size() == 0)
      return pattern_get();

    rFormatInfo finfo = type_check<FormatInfo>(args[0], 0u);
    return global_format(finfo, pattern_get(), finfo);
  }

  const std::string&
  FormatInfo::pattern_get()
  {
    if (!consistent_)
      compute_pattern();
    return pattern_;
  }

  void
  FormatInfo::compute_pattern()
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
      + (case_ == Case::UPPER ? char(toupper(spec_[0])): spec_[0])
      + '|';
    consistent_ = true;
  }

  rObject
  FormatInfo::update_hook(const std::string& slot, rObject val)
  {
    if (slot == "width")
      width_ = type_check<Float>(val, 0u)->to_unsigned_int();
    else if (slot == "precision")
      precision_ = type_check<Float>(val, 0u)->to_unsigned_int();
    else if (slot == "alt")
      alt_ = val->as_bool();
    else if (slot == "prefix")
    {
      std::string val_(type_check<String>(val, 0u)->value_get());
      if (val_ != " " && val_ != "+" && val_ != "")
        RAISE("expected \"\", \" \" or \"+\", got \"" + val_ + "\"");
      prefix_ = val_;
    }
    else if (slot == "alignment")
    {
      int val_(type_check<Float>(val, 0u)->to_int());
      if (val > 1 || val < -1)
        RAISE("expected integer -1, 0 or 1, got " + string_cast(val_));
      alignment_ = val_ ? Align::CENTER
        : (val_ == 1 ? Align::RIGHT : Align::LEFT);
    }
    else if (slot == "uppercase")
    {
      int val_(type_check<Float>(val, 0u)->to_int());
      if (val > 1 || val < -1)
        RAISE("expected integer -1, 0 or 1, got " + string_cast(val_));
      case_ = val_ ? Case::UNDEFINED
        : (val_ == 1 ? Case::UPPER : Case::LOWER);
      if (case_ == Case::UNDEFINED)
        spec_ = "s";
      else if (spec_ == "s")
        spec_ = (case_ == Case::UPPER ? "D" : "d");
    }
    else if (slot == "group")
    {
      std::string val_(type_check<String>(val, 0u)->value_get());
      if (val_ != " " && val_ != "")
        RAISE("expected \" \" or \"\", got \"" + val_ + "\"");
      group_ = val_;
    }
    else if (slot == "pad")
    {
      std::string val_(type_check<String>(val, 0u)->value_get());
      if (val_ != " " && val_ != "0")
        RAISE("expected \" \" or \"0\", got \"" + val_ + "\"");
      pad_ = val_;
    }
    else if (slot == "spec")
    {
      std::string val_(type_check<String>(val, 0u)->value_get());
      if (val_.size() != 1)
        RAISE("expected one character long string, got " + val_);
      if (!strchr("sdbxoefEDX", val_[0]))
        RAISE("expected one character in \"sdbxoefEDX\", got \"" + val_ + "\"");
      spec_ = type_check<String>(val, 0u)->to_lower();
      case_ = (spec_ == "s") ? Case::UNDEFINED
        : (islower(val_[0]) ? Case::LOWER : Case::UPPER);
    }
    else
      return 0;
    consistent_ = false;
    return 0;
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
    bind.proto()->property_set(SYMBOL(width), SYMBOL(updateHook), make_primitive(&FormatInfo::update_hook));
    bind.proto()->property_set(SYMBOL(precision), SYMBOL(updateHook), make_primitive(&FormatInfo::update_hook));
    bind.proto()->property_set(SYMBOL(alt), SYMBOL(updateHook), make_primitive(&FormatInfo::update_hook));
    bind.proto()->property_set(SYMBOL(prefix), SYMBOL(updateHook), make_primitive(&FormatInfo::update_hook));
    bind.proto()->property_set(SYMBOL(alignment), SYMBOL(updateHook), make_primitive(&FormatInfo::update_hook));
    bind.proto()->property_set(SYMBOL(uppercase), SYMBOL(updateHook), make_primitive(&FormatInfo::update_hook));
    bind.proto()->property_set(SYMBOL(group), SYMBOL(updateHook), make_primitive(&FormatInfo::update_hook));
    bind.proto()->property_set(SYMBOL(pad), SYMBOL(updateHook), make_primitive(&FormatInfo::update_hook));
    bind.proto()->property_set(SYMBOL(spec), SYMBOL(updateHook), make_primitive(&FormatInfo::update_hook));
  }

  rObject
  FormatInfo::proto_make()
  {
    return new FormatInfo();
  }

  URBI_CXX_OBJECT_REGISTER(FormatInfo);
} // namespace object
