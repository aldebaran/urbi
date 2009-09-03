/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <libport/format.hh>
#include <libport/escape.hh>

#include <object/regexp.hh>
#include <object/symbols.hh>

namespace object
{
  Regexp::Regexp(const std::string& rg)
    : re_(rg)
  {
    proto_add(proto ? proto : Object::proto);
  }

  Regexp::Regexp(rRegexp model)
    : re_(model->re_)
  {
    proto_add(model);
  }

  std::string
  Regexp::as_string() const
  {
    return libport::format("Regexp(\"%s\")", libport::escape(re_));
  }

  void
  Regexp::init(const std::string& rg)
  {
    // Depending on the version of Boost.Regex, "" might not be valid.
    // Make it always invalid.
    if (rg.empty())
      RAISE(libport::format("Invalid regular expression \"%s\": %s",
                            libport::escape(rg), "Empty expression"));
    try
    {
      re_ = boost::regex(rg);
    }
    catch (const boost::regex_error& e)
    {
      RAISE(libport::format("Invalid regular expression \"%s\": %s",
                            libport::escape(rg), e.what()));
    }
  }

  bool
  Regexp::match(const std::string& str) const
  {
    return regex_search(str, re_);
  }

  void
  Regexp::initialize(CxxObject::Binder<Regexp>& bind)
  {
#define DECLARE(Urbi, Cxx)                      \
    bind(SYMBOL(Urbi), &Regexp::Cxx)

    DECLARE(asString, as_string);
    DECLARE(init, init);
    DECLARE(match, match);
#undef DECLARE
  }

  rObject
  Regexp::proto_make()
  {
    return new Regexp(".");
  }

  URBI_CXX_OBJECT_REGISTER(Regexp);
}
