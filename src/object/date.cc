#include <object/date.hh>
#include <object/global.hh>

namespace object
{
  /*-------------.
  | Construction |
  `-------------*/

  Date::Date()
  {
    proto_add(proto ? proto : Object::proto);
    time_t time;
    ::time(&time);
    split(time);
  }

  Date::Date(time_t time)
  {
    proto_add(proto ? proto : Object::proto);
    split(time);
  }

  Date::Date(rDate time)
    : _stamp(time)
    , _year(time->_year)
    , _month(time->_month)
    , _day(time->_day)
    , _hour(time->_hour)
    , _min(time->_min)
    , _sec(time->_sec)
  {
    CAPTURE_GLOBAL(Orderable);
    proto_add(Orderable);
    proto_add(proto);
  }

  void
  Date::init(const objects_type& args)
  {
    check_arg_count(args.size(), 0, 1);

    if (!args.empty())
      split(from_urbi<unsigned>(args[0]));
    else
    {
      time_t time;
      ::time(&time);
      split(time);
    }
  }

  void
  Date::split(time_t time)
  {
    struct tm date;
    localtime_r(&time, &date);
    _stamp = time;
    _year = date.tm_year + 1900;
    _month = date.tm_mon;
    _day = date.tm_mday;
    _hour = date.tm_hour;
    _min = date.tm_min;
    _sec = date.tm_sec;
  }

  /*-----------.
  | Comparison |
  `-----------*/

  bool
  Date::operator ==(rDate rhs) const
  {
    return _stamp == rhs->_stamp;
  }

  bool
  Date::operator <(rDate rhs) const
  {
    return _stamp < rhs->_stamp;
  }

  /*------------.
  | Conversions |
  `------------*/

  std::string
  Date::as_string()
  {
    return libport::format("%s-%s-%s %s:%s:%s",
                           _year, _month, _day, _hour, _min, _sec);
  }

  /*---------------.
  | Binding system |
  `---------------*/

  void
  Date::initialize(CxxObject::Binder<Date>& bind)
  {
    bind(SYMBOL(asString), &Date::as_string);
    bind(SYMBOL(init), &Date::init);
    bind(SYMBOL(EQ_EQ), &Date::operator ==);
    bind(SYMBOL(LT), (bool (Date::*)(rDate rhs) const)&Date::operator <);
  }

  rObject
  Date::proto_make()
  {
    return new Date(0);
  }

  URBI_CXX_OBJECT_REGISTER(Date);
}
