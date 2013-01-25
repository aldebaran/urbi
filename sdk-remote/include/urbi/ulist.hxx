/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/ulist.hxx

#include <libport/preproc.hh>
#include <boost/lexical_cast.hpp>
#include <urbi/uvalue.hh>
#include <libport/cassert>

namespace urbi
{

  /*--------.
  | UList.  |
  `--------*/

# define ULIST_NTH(Const)                       \
  inline                                        \
  Const UValue&                                 \
  UList::operator[](size_t i) Const             \
  {                                             \
    i += offset;                                \
    if (i < size())                             \
      return *array[i];                         \
    else                                        \
      return error();                           \
  }

  ULIST_NTH(__)
  ULIST_NTH(const)

# undef ULIST_NTH

  inline
  UList::iterator
  UList::begin()
  {
    return array.begin();
  }

  inline
  UList::iterator
  UList::end()
  {
    return array.end();
  }

  inline
  UList::const_iterator
  UList::begin() const
  {
    return array.begin();
  }

  inline
  UList::const_iterator
  UList::end() const
  {
    return array.end();
  }

  inline
  size_t
  UList::size() const
  {
    return array.size();
  }

  inline
  void
  UList::setOffset(size_t n)
  {
    offset = n;
  }

  template<typename T>
  inline
  UList&
  UList::push_back(const T& v)
  {
    array.push_back(new UValue(v));
    return *this;
  }

  inline
  UValue&
  UList::front()
  {
   return *array.front();
  }

  inline
  void
  UList::pop_back()
  {
    array.pop_back();
  }

} // namespace urbi
