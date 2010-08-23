/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <boost/date_time/local_time/local_time.hpp>

#include <object/symbols.hh>
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

    void
    Date::init(const objects_type& args)
    {
      check_arg_count(args.size(), 0, 1);

      if (args.empty())
        time_ = boost::posix_time::microsec_clock::local_time();
      else
      {
        type_check(args[0], String::proto, 1);
        time_ = boost::posix_time::time_from_string
          (args[0]->as<object::String>()->value_get());
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

    OVERLOAD_TYPE_3(
      MINUS_overload, 1, 1,
      Date, (rDuration (Date::*)(rDate) const)     &Date::operator-,
      Duration, (rDate (Date::*)(const Date::duration_type&) const) &Date::operator-,
      Float, (rDate (Date::*)(const Date::duration_type&) const) &Date::operator-)

    static rObject MINUS(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 0, 1);
      static rPrimitive actual = make_primitive(MINUS_overload);
      return (*actual)(args);
    }

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
      return libport::format("%04s-%02s-%02s %02s:%02s:%02s",
                             time_.date().year(),
                             // Otherwise we get "Jan", "Feb", etc.
                             int(time_.date().month()),
                             time_.date().day(),
                             time_.time_of_day().hours(),
                             time_.time_of_day().minutes(),
                             time_.time_of_day().seconds());
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
      // FIXME: Crappy way to find the local time offset, but AFAICT,
      // Boost provides no way to get the current timezone :-(
      static boost::posix_time::time_duration diff = boost::posix_time::seconds
        ((boost::posix_time::microsec_clock::local_time()
          - boost::posix_time::microsec_clock::universal_time()).total_seconds());
      return abs + diff;
    }


    /*-----------------.
    | Binding system.  |
    `-----------------*/

    void
    Date::initialize(CxxObject::Binder<Date>& bind)
    {
      bind(SYMBOL(EQ_EQ), &Date::operator ==);
      bind(SYMBOL(LT), (bool (Date::*)(rDate rhs) const)&Date::operator <);
      bind(SYMBOL(MINUS), MINUS);
      bind(SYMBOL(PLUS), &Date::operator +);
      bind(SYMBOL(asFloat), &Date::as_float);
      bind(SYMBOL(asString), &Date::as_string);
      bind(SYMBOL(epoch), &Date::epoch);
      bind(SYMBOL(init), &Date::init);
      bind(SYMBOL(now), &Date::now);
    }

    URBI_CXX_OBJECT_REGISTER(Date)
      : time_(boost::posix_time::microsec_clock::local_time())
    {}
  }
}
