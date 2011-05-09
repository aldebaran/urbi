/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuvalue/ulist.cc

#include <libport/debug.hh>
#include <libport/escape.hh>

#include <urbi/fwd.hh>
#include <urbi/ulist.hh>
#include <urbi/uvalue.hh>

GD_CATEGORY(Urbi.UValue);

namespace urbi
{

  UList::UList()
    : offset(0)
  {}

  UList::UList(const UList& b)
    : offset(0)
  {
    *this = b;
  }

  UList& UList::operator= (const UList& b)
  {
    if (this == &b)
      return *this;
    clear();
    foreach (UValue* v, b.array)
      array.push_back(new UValue(*v));
    offset = b.offset;
    return *this;
  }

  UList::~UList()
  {
    clear();
  }

  UValue&
  UList::error()
  {
    return UValue::error();
  }



  void UList::clear()
  {
    offset = 0;
    foreach (UValue *v, array)
      delete v;
    array.clear();
  }

  std::ostream&
  UList::print(std::ostream& o) const
  {
    o << '[';
    size_t sz = size();
    for (unsigned i = 0; i < sz; ++i)
      o << (*this)[i]
        << (i != sz - 1 ? ", " : "");
    o << ']';
    return o;
  }

  std::ostream&
  operator<< (std::ostream& o, const UList& t)
  {
    return t.print(o);
  }

} // namespace urbi
