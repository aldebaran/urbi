/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_DATE_HH
# define OBJECT_DATE_HH

# include <boost/date_time/gregorian/gregorian.hpp>
# include <boost/date_time/posix_time/posix_time.hpp>

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
      typedef boost::posix_time::ptime value_type;
      typedef boost::posix_time::time_duration duration_type;

      Date();
      Date(rDate model);
      Date(const value_type& time);
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
      Date& operator += (const duration_type& rhs);
      rDate operator + (const duration_type& rhs) const;

      Date& operator -= (const duration_type& rhs);
      rDate operator - (const duration_type& rhs) const;

      rDuration operator - (rDate rhs) const;


    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      value_type as_boost() const;
      virtual std::string as_string() const;
      duration_type as_float() const;

    /*--------.
    | Dates.  |
    `--------*/

    public:
      static rDate now();
      static value_type epoch();

    /*----------.
    | Details.  |
    `----------*/

    private:
      value_type time_;

      URBI_CXX_OBJECT_(Date);
    };
  }
}

# include <urbi/object/date.hxx>

#endif
