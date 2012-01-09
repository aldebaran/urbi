/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/lexical-cast.hh>

#include <urbi/object/symbols.hh>
#include <object/system.hh>

#include <urbi/object/cxx-conversions.hh>
#include <urbi/object/global.hh>
#include <urbi/object/location.hh>
#include <urbi/object/position.hh>

namespace urbi
{
  namespace object
  {

    /*---------------.
    | Construction.  |
    `---------------*/

    Location::Location()
      : loc_()
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Location::Location(const value_type& loc)
      : loc_(loc)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Location::Location(rLocation model)
      : loc_(model->loc_)
    {
      proto_add(proto);
    }

    URBI_CXX_OBJECT_INIT(Location)
      : loc_()
    {
      bind(SYMBOL(init),
           static_cast<void (Location::*)()>(&Location::init));
      bind(SYMBOL(init),
           static_cast<void (Location::*)(const Position::value_type&)>
             (&Location::init));
      bind(SYMBOL(init),
           static_cast<void (Location::*)(const Position::value_type&,
                                          const Position::value_type&)>
             (&Location::init));
      bind(SYMBOL(EQ_EQ),
           (bool (Location::*)(rLocation rhs) const) &Location::operator ==);

      // For some reason, cl.exe refuses "&Location::value_type::begin"
      // with error: function cannot access 'yy::location::begin'
      bind(SYMBOL(begin), &yy::location::begin);
      bind(SYMBOL(end),   &yy::location::end);

#define DECLARE(Name, Cxx)                             \
      bind(SYMBOL_(Name), &Location::Cxx)

      DECLARE(asString,         as_string);
      DECLARE(isSystemLocation, is_system_location);

#undef DECLARE
    }

    void
    Location::init()
    {}

    void
    Location::init(const Position::value_type& begin)
    {
      init(begin, begin + 1);
    }

    void
    Location::init(const Position::value_type& begin,
                   const Position::value_type& end)
    {
      loc_ = value_type(begin, end);
    }

    /*--------------.
    | Comparisons.  |
    `--------------*/

    bool
    operator ==(const Location::value_type& lhs,
                const Location::value_type& rhs)
    {
      return lhs.begin == rhs.begin && lhs.end == rhs.end;
    }

    bool
    Location::is_system_location() const
    {
      return urbi::object::is_system_location(loc_);
    }

    /*--------------.
    | Conversions.  |
    `--------------*/

    std::string
    Location::as_string() const
    {
      return string_cast(loc_);
    }
  } // namespace object
} // namespace urbi
