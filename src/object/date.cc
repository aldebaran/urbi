#include <object/date.hh>
#include <object/global.hh>

namespace object
{
  /*---------------.
  | Construction.  |
  `---------------*/

  Date::Date()
  {
    proto_add(proto ? proto : Object::proto);
    split(time(0));
  }

  Date::Date(time_t time)
  {
    proto_add(proto ? proto : Object::proto);
    split(time);
  }

  Date::Date(rDate time)
    : stamp_(time)
    , year_(time->year_)
    , month_(time->month_)
    , day_(time->day_)
    , hour_(time->hour_)
    , min_(time->min_)
    , sec_(time->sec_)
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
      split(time(0));
  }

  void
  Date::split(time_t time)
  {
    struct tm date;
    localtime_r(&time, &date);
    stamp_ = time;
    year_ = date.tm_year + 1900;
    month_ = date.tm_mon;
    day_ = date.tm_mday;
    hour_ = date.tm_hour;
    min_ = date.tm_min;
    sec_ = date.tm_sec;
  }

  /*-------------.
  | Comparison.  |
  `-------------*/

  bool
  Date::operator ==(rDate rhs) const
  {
    return stamp_ == rhs->stamp_;
  }

  bool
  Date::operator <(rDate rhs) const
  {
    return stamp_ < rhs->stamp_;
  }

  /*--------------.
  | Conversions.  |
  `--------------*/

  std::string
  Date::as_string()
  {
    return libport::format("%s-%s-%s %s:%s:%s",
                           year_, month_, day_, hour_, min_, sec_);
  }

  /*-----------------.
  | Binding system.  |
  `-----------------*/

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
