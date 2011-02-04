/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
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
    bool res = m && m->type == MESSAGE_DATA;
    if (res)
      val = *m->value;
    delete m;
    return res;
  }

  template <>
  inline
  int
  getValue<ufloat>(UMessage* m, ufloat& val)
  {
    bool res = m && m->type == MESSAGE_DATA && m->value->type == DATA_DOUBLE;
    if (res)
      val = m->value->val;
    delete m;
    return res;
  }

  template <>
  inline
  int
  getValue<std::string>(UMessage* m, std::string& val)
  {
    bool res = m && m->type == MESSAGE_DATA && m->value->type == DATA_STRING;
    if (res)
      val = *m->value->stringValue;
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
