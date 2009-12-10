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
      : time_(0)
    {
      proto_add(proto ? proto : Object::proto);
    }

    Date::Date(time_t t)
      : time_(t)
    {
      proto_add(proto ? proto : Object::proto);
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
      time_ = args.empty() ? 0 : from_urbi<unsigned>(args[0]);
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
      time_ += rhs->seconds();
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

    /*--------.
    | Dates.  |
    `--------*/

    rDate
    Date::now ()
    {
      return new Date (time(NULL));
    }

    rDate
    Date::epoch ()
    {
      return new Date (0);
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
    }

    URBI_CXX_OBJECT_REGISTER(Date)
      : time_(0)
    {}
  }
}
