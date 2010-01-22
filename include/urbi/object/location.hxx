/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_LOCATION_HXX
# define OBJECT_LOCATION_HXX

# include <urbi/object/location.hh>

namespace urbi
{
  namespace object
  {

    /*--------------.
    | Conversions.  |
    `--------------*/

    inline Location::value_type& Location::value_get()
    {
      return loc_;
    }

    /*-----------.
    | Accessor.  |
    `-----------*/

    inline Position::value_type* Location::begin_ref()
    {
      return &loc_.begin;
    }

    inline Position::value_type* Location::end_ref()
    {
      return &loc_.end;
    }

    /*-------------.
    | ::ast::loc.  |
    `-------------*/

    template <>
    struct CxxConvert<Location::value_type>
    {
      typedef Location::value_type target_type;
      static target_type
      to(const rObject& o, unsigned idx)
      {
        type_check<Location>(o, idx);
        return o->as<Location>()->value_get();
      }

      static rObject
      from(target_type v)
      {
        return new Location(v);
      }
    };

  } // namespace object
} // namespace urbi

#endif
