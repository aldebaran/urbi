/// \file urbi/umessage.hxx

#include <urbi/uvalue.hh>

namespace urbi
{

  template <typename T>
  inline
  int
  getValue(UMessage* m, T& val)
  {
    int res = 0;
    if ((res = (m && m->type == MESSAGE_DATA)))
      val = *m->value;
    delete m;
    return res;
  }

  template <>
  inline
  int
  getValue<double>(UMessage* m, double& val)
  {
    int res = 0;
    if ((res = (m && m->type == MESSAGE_DATA && m->value->type == DATA_DOUBLE)))
      val = (double) *m->value;
    delete m;
    return res;
  }

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
