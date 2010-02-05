/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <object/symbols.hh>
#include <urbi/object/date.hh>
#include <urbi/object/duration.hh>
#include <urbi/object/global.hh>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace urbi
{
  namespace object
  {
    /*---------------.
    | Construction.  |
    `---------------*/

    Date::Date()
      : time_(0)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Date::Date(value_type t)
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
      {
        time_ = 0;
        return;
      }

      if (rString str = args[0]->as<object::String>())
      {
        boost::posix_time::ptime t
          (boost::posix_time::time_from_string(str->value_get()));
        tm timeinfo = boost::posix_time::to_tm(t);
        time_ = mktime (&timeinfo);

        return;
      }

      time_ = from_urbi<unsigned>(args[0]);
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

    rDuration
    Date::operator - (rDate rhs) const
    {
      return new Duration(time_ - rhs->time_);
    }

    Date&
    Date::operator += (rDuration rhs)
    {
      time_ += static_cast<value_type>(rhs->seconds());
      return *this;
    }

    rDate
    Date::operator + (rDuration rhs) const
    {
      rDate res = new Date(time_);
      *res += rhs;
      return res;
    }

    /*--------------.
    | Conversions.  |
    `--------------*/

    std::string
    Date::as_string() const
    {
      struct tm date;
      localtime_r(&time_, &date);
      return libport::format("%04s-%02s-%02s %02s:%02s:%02s",
                             1900 + date.tm_year,
                             date.tm_mon + 1,
                             date.tm_mday,
                             date.tm_hour,
                             date.tm_min,
                             date.tm_sec);
    }

    libport::ufloat
    Date::asFloat() const
    {
      return time_;
    }

    libport::ufloat
    Date::timestamp() const
    {
      return asFloat ();
    }

    /*--------.
    | Dates.  |
    `--------*/

    rDate
    Date::now ()
    {
      return new Date(time(0));
    }

    rDate
    Date::epoch ()
    {
      return new Date(0);
    }


    /*-----------------.
    | Binding system.  |
    `-----------------*/

    void
    Date::initialize(CxxObject::Binder<Date>& bind)
    {
      bind(SYMBOL(EQ_EQ), &Date::operator ==);
      bind(SYMBOL(LT), (bool (Date::*)(rDate rhs) const)&Date::operator <);
      bind(SYMBOL(MINUS), &Date::operator -);
      bind(SYMBOL(PLUS), &Date::operator +);
      bind(SYMBOL(asFloat), &Date::asFloat);
      bind(SYMBOL(asString), &Date::as_string);
      bind(SYMBOL(epoch), &Date::epoch);
      bind(SYMBOL(init), &Date::init);
      bind(SYMBOL(now), &Date::now);
      bind(SYMBOL(timestamp), &Date::timestamp);
    }

    URBI_CXX_OBJECT_REGISTER(Date)
      : time_(0)
    {}
  }
}
