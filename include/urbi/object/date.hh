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

#define DATE_MODIFIERS(Name)          \
  Name ## _type Name ## _get() const; \
  void Name ## _set(Name ## _type y); \

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
      typedef boost::posix_time::time_duration::hour_type hour_type;
      typedef boost::posix_time::time_duration::min_type minute_type;
      typedef boost::posix_time::time_duration::sec_type second_type;

      typedef boost::gregorian::date date_type;
      typedef boost::gregorian::greg_year year_type;
      typedef boost::gregorian::greg_month month_type;
      typedef boost::gregorian::greg_day day_type;

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

    /*----------------.
    | Modifications.  |
    `----------------*/

    public:
      DATE_MODIFIERS(year)
      DATE_MODIFIERS(month)
      DATE_MODIFIERS(day)
      DATE_MODIFIERS(hour)
      DATE_MODIFIERS(minute)
      DATE_MODIFIERS(second)

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

      URBI_CXX_OBJECT(Date);
    };
  }
}

# undef DATE_MODIFIERS

# include <urbi/object/date.hxx>

#endif
