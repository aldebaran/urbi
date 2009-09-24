/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_DATE_HH
# define OBJECT_DATE_HH

# include <time.h>

# include <object/cxx-object.hh>

namespace object
{
  class Date: public CxxObject
  {

  /*-------------.
  | Construction |
  `-------------*/

  public:
    Date();
    Date(rDate model);
    Date(time_t time);
    void init(const objects_type& args);

  private:
    void split(time_t time);

  /*------------.
  | Comparisons |
  `------------*/

  public:
    using Object::operator <;
    bool operator ==(rDate rhs) const;
    bool operator <(rDate rhs) const;

  /*------------.
  | Conversions |
  `------------*/

  public:
    std::string as_string();
    std::string as_printable();

  /*--------.
  | Details |
  `--------*/

  private:
    unsigned _stamp;
    unsigned _year;
    unsigned _month;
    unsigned _day;
    unsigned _hour;
    unsigned _min;
    unsigned _sec;

    URBI_CXX_OBJECT(Date);
  };
}

#endif
