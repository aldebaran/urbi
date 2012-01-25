/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <boost/date_time/local_time/local_time.hpp>

#include <urbi/object/symbols.hh>
#include <urbi/object/date.hh>
#include <urbi/object/duration.hh>
#include <urbi/object/global.hh>

namespace urbi
{
  namespace object
  {
    /*---------------.
    | Construction.  |
    `---------------*/

    Date::Date()
      : time_(boost::posix_time::microsec_clock::local_time())
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Date::Date(const value_type& t)
      : time_(t)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Date::Date(rDate time)
      : time_(time->time_)
    {
      CAPTURE_GLOBAL(Orderable);
      proto_add(Orderable);
      proto_add(proto);
    }

    // FIXME: kill this when overloading is fully supported
    OVERLOAD_TYPE_3(
      MINUS_overload, 1, 1,
      Date,
      (rDuration (Date::*)(rDate) const)                      &Date::operator-,
      Duration,
      (rDate     (Date::*)(const Date::duration_type&) const) &Date::operator-,
      Float,
      (rDate     (Date::*)(const Date::duration_type&) const) &Date::operator-)

    // FIXME: kill this when overloading is fully supported
    static rObject MINUS(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 0, 1);
      static rPrimitive actual = new Primitive(MINUS_overload);
      return (*actual)(args);
    }

    URBI_CXX_OBJECT_INIT(Date)
      : time_(boost::posix_time::microsec_clock::local_time())
    {
      bind_variadic(SYMBOL(MINUS), MINUS);

#define DECLARE(Unit)                                         \
      bind(libport::Symbol(#Unit),       &Date::Unit ## _get, \
           libport::Symbol(#Unit "Set"), &Date::Unit ## _set)

      DECLARE(day);
      DECLARE(hour);
      DECLARE(microsecond);
      DECLARE(minute);
      DECLARE(month);
      DECLARE(second);
      DECLARE(us);
      DECLARE(year);

#undef DECLARE

      bind(SYMBOL(LT), (bool (Date::*)(rDate rhs) const)&Date::operator <);

#define DECLARE(Name, Cxx)           \
      bind(SYMBOL_(Name), &Date::Cxx)

      DECLARE(EQ_EQ,    operator ==);
      DECLARE(PLUS,     operator +);
      DECLARE(asFloat,  as_float);
      DECLARE(asString, as_string);
      DECLARE(epoch,    epoch);
      DECLARE(now,      now);

#undef DECLARE

      bind_variadic(SYMBOL(init), &Date::init);
    }

    void
    Date::init(const objects_type& args)
    {
      check_arg_count(args.size(), 0, 1);

      if (args.empty())
        time_ = boost::posix_time::microsec_clock::local_time();
      else
      {
        const std::string& s = from_urbi<std::string>(args[0]);
        try
        {
          time_ = boost::posix_time::time_from_string(s);
        }
        catch (const boost::bad_lexical_cast&)
        {
          FRAISE("cannot convert to date: %s", s);
        }
      }
    }

    /*-------------.
    | Comparison.  |
    `-------------*/

    bool
    Date::operator ==(rDate rhs) const
    {
      return time_ == rhs->time_;
    }

    bool
    Date::operator <(rDate rhs) const
    {
      return time_ < rhs->time_;
    }

    /*-------------.
    | Operations.  |
    `-------------*/

    Date&
    Date::operator += (const duration_type& rhs)
    {
      time_ += rhs;
      return *this;
    }

    rDate
    Date::operator + (const duration_type& rhs) const
    {
      rDate res = new Date(time_);
      *res += rhs;
      return res;
    }

    Date&
    Date::operator -= (const duration_type& rhs)
    {
      time_ -= rhs;
      return *this;
    }

    rDate
    Date::operator - (const duration_type& rhs) const
    {
      rDate res = new Date(time_);
      *res -= rhs;
      return res;
    }

    rDuration
    Date::operator - (rDate rhs) const
    {
      return new Duration(time_ - rhs->time_);
    }


    /*----------------.
    | Modifications.  |
    `----------------*/

#define TIME_GETTER(Unit)                        \
  Date::Unit ## _type Date::Unit ## _get() const \
  {                                              \
    return time_.time_of_day().Unit ## s();      \
  }                                              \

#define TIME_SETTER(Unit)                               \
  void Date::Unit ## _set(Date::Unit ## _type value)    \
  {                                                     \
    Date::duration_type td = time_.time_of_day();       \
    td -= boost::posix_time::Unit ## s(td.Unit ## s()); \
    td += boost::posix_time::Unit ## s(value);          \
    time_ = Date::value_type(time_.date(), td);         \
  }                                                     \

#define TIME_MODIFIERS(Unit) \
  TIME_GETTER(Unit)          \
  TIME_SETTER(Unit)          \

  TIME_MODIFIERS(hour)
  TIME_MODIFIERS(minute)
  TIME_MODIFIERS(second)

#undef TIME_GETTER
#undef TIME_SETTER
#undef TIME_MODIFIERS

#define MILLION 1000000L

  Date::us_type Date::us_get() const
  {
    return time_.time_of_day().total_microseconds();
  }

  void Date::us_set(Date::us_type value)
  {
    Date::duration_type td = time_.time_of_day();
    td -= boost::posix_time::microseconds(td.total_microseconds());
    td += boost::posix_time::microseconds(value);
    time_ = Date::value_type(time_.date(), td);
  }

  Date::microsecond_type Date::microsecond_get() const
  {
    return us_get() % MILLION;
  }

  void Date::microsecond_set(Date::microsecond_type value)
  {
    Date::duration_type td = time_.time_of_day();
    td -= boost::posix_time::microseconds(td.total_microseconds() % MILLION);
    td += boost::posix_time::microseconds(value);
    time_ = Date::value_type(time_.date(), td);
  }


#define DATE_GETTER(Unit)                        \
  Date::Unit ## _type Date::Unit ## _get() const \
  {                                              \
    return time_.date().Unit();                  \
  }                                              \

#define DATE_SETTER(Unit, Y, M, D)                            \
  void Date::Unit ## _set(Date::Unit ## _type Unit ## _given) \
  {                                                           \
    try                                                       \
    {                                                         \
      time_ = Date::value_type(Date::date_type(Y, M, D),      \
                               time_.time_of_day());          \
    }                                                         \
    catch(std::exception& e)                                  \
    {                                                         \
      runner::raise_primitive_error(e.what());                \
    }                                                         \
  }                                                           \

#define DATE_MODIFIERS(Unit, Y, M, D) \
  DATE_GETTER(Unit)                   \
  DATE_SETTER(Unit, Y, M, D)          \

  DATE_MODIFIERS(year,  year_given, time_.date().month(), time_.date().day())
  DATE_MODIFIERS(month, time_.date().year(), month_given, time_.date().day())
  DATE_MODIFIERS(day,   time_.date().year(), time_.date().month(), day_given)

#undef DATE_GETTER
#undef DATE_SETTER
#undef DATE_MODIFIERS

    /*--------------.
    | Conversions.  |
    `--------------*/

    Date::value_type
    Date::as_boost() const
    {
      return time_;
    }

    std::string
    Date::as_string() const
    {
      Date::duration_type td = time_.time_of_day();
      return
        libport::format("%04s-%02s-%02s %02s:%02s:%02s.%06s",
                        time_.date().year(),
                        // Otherwise we get "Jan", "Feb", etc.
                        int(time_.date().month()),
                        time_.date().day(),
                        td.hours(),
                        td.minutes(),
                        td.seconds(),
                        td.total_microseconds() % MILLION);
    }

    Date::duration_type
    Date::as_float() const
    {
      return time_ - epoch();
    }

    /*--------.
    | Dates.  |
    `--------*/

    rDate
    Date::now()
    {
      return new Date();
    }

    Date::value_type
    Date::epoch()
    {
      value_type abs(boost::gregorian::date(1970, 1, 1));
      return abs + local_time_offset();
    }

    Date::duration_type
    Date::local_time_offset()
    {
      // FIXME: Crappy way to find the local time offset, but AFAICT,
      // Boost provides no way to get the current timezone :-(
      using namespace boost::posix_time;
      static Date::duration_type offset =
        seconds((microsec_clock::local_time()
                 - microsec_clock::universal_time()).total_seconds());
      return offset;
    }
  }
}
