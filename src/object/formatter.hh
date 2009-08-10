/**
 ** \file object/formatter.hh
 ** \brief Definition of the Urbi object formatter.
 */

#ifndef OBJECT_FORMATTER_HH
# define OBJECT_FORMATTER_HH

# include <libport/attributes.hh>

# include <object/fwd.hh>
# include <object/cxx-object.hh>

namespace object
{

  class Formatter: public CxxObject
  {
  public:
    Formatter();
    Formatter(rFormatter model);

    void init(const std::string& format);

  private:
    ATTRIBUTE_R(rList, data);

    URBI_CXX_OBJECT(Formatter);
  };

} // namespace object

#endif // !OBJECT_FORMATTER_HH
