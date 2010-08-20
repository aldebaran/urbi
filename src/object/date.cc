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
        time_ = boost::posix_time::time_from_string(args[0]->as<object::String>()->value_get());
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

    rDuration
    Date::operator - (rDate rhs) const
    {
      return new Duration(time_ - rhs->time_);
    }

    Date&
    Date::operator += (const boost::posix_time::time_duration& rhs)
    {
      time_ += rhs;
      return *this;
    }

    rDate
    Date::operator + (const boost::posix_time::time_duration& rhs) const
    {
      rDate res = new Date(time_);
      *res += rhs;
      return res;
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
      return to_simple_string(time_);
    }

    boost::posix_time::time_duration
    Date::as_timestamp() const
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
      bind(SYMBOL(MINUS), &Date::operator -);
      bind(SYMBOL(PLUS), &Date::operator +);
      bind(SYMBOL(asString), &Date::as_string);
      bind(SYMBOL(epoch), &Date::epoch);
      bind(SYMBOL(init), &Date::init);
      bind(SYMBOL(now), &Date::now);
      bind(SYMBOL(timestamp), &Date::as_timestamp);
    }

    URBI_CXX_OBJECT_REGISTER(Date)
      : time_(boost::posix_time::microsec_clock::local_time())
    {}
  }
}
