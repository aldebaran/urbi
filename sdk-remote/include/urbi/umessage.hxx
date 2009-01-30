/// \file urbi/umessage.hxx

namespace urbi
{

  inline
  UMessage::operator urbi::UValue& ()
  {
    return *value;
  }

  inline
  std::ostream&
  operator<<(std::ostream& o, const UMessage& m)
  {
    return m.print(o);
  };

} // namespace urbi
