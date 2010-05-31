/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/lexical-cast.hh>

#include <object/symbols.hh>
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

    void
    Location::init(const objects_type& args)
    {
      check_arg_count(args.size(), 0, 2);
      switch (args.size())
      {
      case 1:
        {
          Position::value_type pos =
            type_check<Position>(args[0])->value_get();
          loc_ = value_type(pos, pos + 1);
        }
        return;
      case 2:
        loc_ = value_type(type_check<Position>(args[0], 0u)->value_get(),
                          type_check<Position>(args[1], 1u)->value_get());
        return;
      }
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

    /*-----------------.
    | Binding system.  |
    `-----------------*/

    void
    Location::initialize(CxxObject::Binder<Location>& bind)
    {
      bind(SYMBOL(EQ_EQ),
           (bool (Location::*)(rLocation rhs) const) &Location::operator ==);
      bind(SYMBOL(asString), &Location::as_string);
      bind(SYMBOL(init), &Location::init);
      bind(SYMBOL(isSystemLocation), &Location::is_system_location);

#define DECLARE(Name)                                           \
      bind.var(SYMBOL( Name ), &Location:: Name ## _ref)

      DECLARE(begin);
      DECLARE(end);
#undef DECLARE
    }

    URBI_CXX_OBJECT_REGISTER(Location)
      : loc_()
    {}


  } // namespace object
} // namespace urbi
