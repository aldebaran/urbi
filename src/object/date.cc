#include <object/date.hh>
#include <object/global.hh>

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
