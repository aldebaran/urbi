#ifndef OBJECT_REGEXP_HH
# define OBJECT_REGEXP_HH

# include <boost/regex.hpp>

# include <object/cxx-object.hh>

namespace object
{
  class Regexp
    : public CxxObject
  {
  public:
    typedef Regexp self_type;
    Regexp(const std::string& rg);
    Regexp(rRegexp model);

  /*-------------.
  | Urbi methods |
  `-------------*/
  public:
    std::string as_string() const;
    void init(const std::string& rg);
    bool match(const std::string& str) const;

  private:
    boost::regex re_;
    URBI_CXX_OBJECT(Regexp);
  };
}

#endif
