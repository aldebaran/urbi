/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <boost/algorithm/string.hpp>

#include <libport/escape.hh>
#include <libport/format.hh>
#include <libport/lexical-cast.hh>

#include <object/urbi/regexp.hh>
#include <urbi/object/symbols.hh>

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

    URBI_CXX_OBJECT_INIT(Regexp)
      : re_(".")
    {
      bind(libport::Symbol( "[]" ), &Regexp::operator[]);

      BIND(asPrintable, as_printable);
      BIND(asString,    as_string);
      BIND(init);
      BIND(match);
      BINDG(matches);
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
        FRAISE("invalid regular expression: %s: `%s'",
               "empty expression", libport::escape(r));
      try
      {
        re_ = boost::regex(r);
      }
      catch (const boost::regex_error& e)
      {
        std::string err = e.what();
        // There's a typo in the typical error message.
        boost::replace_all(err, "occur""ed", "occurred");
        // And there should not be an upper case letter.
        if (isupper(err[0]))
          err[0] = tolower(err[0]);
        // If Boost supplies the regexp, we don't need to repeat it.
        if (err.find("occurred while parsing the regular expression:")
            == std::string::npos)
        {
          FRAISE("invalid regular expression: %s: `%s'",
                 err, libport::escape(r));
        }
        else
        {
          // Yet the message is too long.
          boost::erase_all
            (err,
             ".  The error occurred while parsing the regular expression");
          // We use `this' kind of quotes, not 'that' kind.
          boost::replace_first(err, "'", "`");
          // And we don't end sentences with a period.
          if (err[err.size() - 1] == '.')
            err.resize(err.size() - 1);
          FRAISE("invalid regular expression: %s", err);
        }
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
