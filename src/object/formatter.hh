/**
 ** \file object/formatter.hh
 ** \brief Definition of the Urbi object formatter.
 */

#ifndef OBJECT_FORMATTER_HH
# define OBJECT_FORMATTER_HH

# include <libport/attributes.hh>

# include <object/fwd.hh>
# include <object/cxx-object.hh>
# include <object/cxx-primitive.hh>

namespace object
{

  class Formatter: public CxxObject
  {
  public:
    Formatter();
    Formatter(rFormatter model);

    void init(const std::string& format);

    /// Regular C++ signature, all the arguments are provided.
    std::string operator%(const objects_type& args) const;

    /// Urbi signature that accepts Lists (and bounces to the function
    /// above), otherwise wraps \a arg in a List and then bounces.
    std::string operator%(const rObject& arg) const;

  private:
    /// The FormatInfos and Strings.
    ATTRIBUTE_R(rList, data);

    URBI_CXX_OBJECT(Formatter);
  };

} // namespace object

#endif // !OBJECT_FORMATTER_HH
