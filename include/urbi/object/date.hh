/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
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

# include <libport/cstdint>
# include <libport/ctime>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Date: public CxxObject
    {

    /*---------------.
    | Construction.  |
    `---------------*/

    public:
      typedef boost::posix_time::ptime value_type;
      typedef boost::posix_time::time_duration duration_type;
      typedef duration_type::hour_type hour_type;
      typedef duration_type::min_type minute_type;
      typedef duration_type::sec_type second_type;
      /// [0, 999999[.
      typedef long microsecond_type;
      typedef uint64_t us_type;

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
#define DATE_MODIFIERS(Name)                    \
      Name ## _type Name ## _get() const;       \
      void Name ## _set(Name ## _type y)        \

    DATE_MODIFIERS(day);
    DATE_MODIFIERS(hour);
    DATE_MODIFIERS(microsecond);
    DATE_MODIFIERS(minute);
    DATE_MODIFIERS(month);
    DATE_MODIFIERS(second);
    DATE_MODIFIERS(us);
    DATE_MODIFIERS(year);
# undef DATE_MODIFIERS


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
      static duration_type local_time_offset();

    /*----------.
    | Details.  |
    `----------*/

    private:
      value_type time_;

      URBI_CXX_OBJECT(Date, CxxObject);
    };
  }
}

# include <urbi/object/date.hxx>

#endif
