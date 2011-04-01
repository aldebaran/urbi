/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/escape.hh>
#include <libport/format.hh>
#include <libport/lexical-cast.hh>

#include <object/urbi/regexp.hh>
#include <object/urbi/symbols.hh>

namespace urbi
{
  namespace object
  {

    /*-----------------.
    | Implementation.  |
    `-----------------*/

    Regexp::Regexp(const std::string& r)
      : re_(r)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Regexp::Regexp(rRegexp model)
      : re_(model->re_)
    {
      proto_add(model);
    }

    URBI_CXX_OBJECT_REGISTER_INIT(Regexp)
      : re_(".")
    {
      bind(libport::Symbol( "[]" ), &Regexp::operator[]);

      BIND(asPrintable, as_printable);
      BIND(asString,    as_string);
      BIND(init);
      BIND(match);
      BIND(matches);
    }

    std::string
    Regexp::as_printable() const
    {
      return libport::format("Regexp(\"%s\")", libport::escape(re_));
    }

    std::string
    Regexp::as_string() const
    {
      return string_cast(re_);
    }

    void
    Regexp::init(const std::string& r)
    {
      // Depending on the version of Boost.Regex, "" might not be valid.
      // Make it always invalid.
      if (r.empty())
        FRAISE("invalid regular expression `%s': %s",
               libport::escape(r), "Empty expression");
      try
      {
        re_ = boost::regex(r);
      }
      catch (const boost::regex_error& e)
      {
        FRAISE("invalid regular expression `%s': %s",
               libport::escape(r), e.what());
      }
    }

    bool
    Regexp::match(const std::string& str)
    {
      boost::match_results<std::string::const_iterator> groups;
      bool res = regex_search(str, groups, re_);
      matches_.clear();
      if (res)
        for (unsigned i = 0; i < groups.size(); ++i)
          matches_ << std::string(groups[i].first, groups[i].second);
      return res;
    }

    std::string
    Regexp::operator[] (unsigned idx)
    {
      if (idx >= matches_.size())
        FRAISE("out of bound index: %s", idx);
      return matches_[idx];
    }

    Regexp::matches_type
    Regexp::matches() const
    {
      return matches_;
    }
  }
}
