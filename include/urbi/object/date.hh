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

# include <libport/ctime>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class Date: public CxxObject
    {

    /*---------------.
    | Construction.  |
    `---------------*/

    public:
      typedef time_t value_type;
      Date();
      Date(rDate model);
      Date(value_type time);
      void init(const objects_type& args);

    /*--------------.
    | Comparisons.  |
    `--------------*/

    public:
      using Object::operator <;
      bool operator ==(rDate rhs) const;
      bool operator <(rDate rhs) const;

    /*-------------.
    | Operations.  |
    `-------------*/

    public:
      rDuration operator - (rDate rhs) const;
      Date& operator += (rDuration rhs);
      rDate operator + (rDuration rhs) const;

    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      std::string as_string() const;

    /*--------.
    | Dates.  |
    `--------*/

    public:
      static rDate now();
      static rDate epoch();

    /*----------.
    | Details.  |
    `----------*/

    private:
      value_type time_;

      URBI_CXX_OBJECT_(Date);
    };
  }
}

#endif
