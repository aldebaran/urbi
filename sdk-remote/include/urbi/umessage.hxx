/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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
      val = (T) *m->value;
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
