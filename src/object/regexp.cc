#include <libport/format.hh>
#include <libport/escape.hh>

#include <object/regexp.hh>
#include <object/symbols.hh>

namespace object
{
  Regexp::Regexp(const std::string& rg)
    : rg_(rg)
  {
    proto_add(proto ? proto : Object::proto);
  }

  Regexp::Regexp(rRegexp model)
    : rg_(model->rg_)
  {
    proto_add(model);
  }

  std::string
  Regexp::as_string() const
  {
    return libport::format("Regexp(\"%s\")", libport::escape(rg_));
  }

  void
  Regexp::init(const std::string& rg)
  {
    try
    {
      rg_ = boost::regex(rg);
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
    return regex_search(str, rg_);
  }

  void
  Regexp::initialize(CxxObject::Binder<Regexp>& bind)
  {
    bind(SYMBOL(asString), &Regexp::as_string);
    bind(SYMBOL(init), &Regexp::init);
    bind(SYMBOL(match), &Regexp::match);
  }

  rObject
  Regexp::proto_make()
  {
    return new Regexp(".");
  }

  URBI_CXX_OBJECT_REGISTER(Regexp);
}
