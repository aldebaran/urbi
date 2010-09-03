/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_REGEXP_HH
# define OBJECT_REGEXP_HH

# include <boost/regex.hpp>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class Regexp
      : public CxxObject
    {
    public:
      typedef Regexp self_type;
      Regexp(const std::string& rg);
      Regexp(rRegexp model);

      /*---------------.
      | Urbi methods.  |
      `---------------*/
    public:
      /// "Regexp(\"^a*\")"
      std::string as_printable() const;
      /// "^a*"
      virtual std::string as_string() const;
      void init(const std::string& rg);
      bool match(const std::string& str);
      std::string operator[] (unsigned idx);
      typedef std::vector<std::string> matches_type;
      matches_type matches() const;

    private:
      boost::regex re_;
      matches_type matches_;
      URBI_CXX_OBJECT_(Regexp);
    };
  }
}

#endif
