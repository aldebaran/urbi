/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_LOCATION_HH
# define OBJECT_LOCATION_HH

# include <ast/loc.hh>

# include <urbi/object/position.hh>
# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {

    class Location: public CxxObject
    {

    /*---------------.
    | Construction.  |
    `---------------*/

    public:
      typedef ::ast::loc value_type;
      Location();
      Location(const value_type& loc);
      Location(rLocation model);
      void init(const objects_type& args);

    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      std::string as_string() const;

      inline value_type& value_get()
      {
        return loc_;
      }

    /*-----------.
    | Accessor.  |
    `-----------*/

    private:
      inline Position::value_type* begin_ref()
      {
        return &loc_.begin;
      };

      inline Position::value_type* end_ref()
      {
        return &loc_.end;
      };

    /*----------.
    | Details.  |
    `----------*/

    private:
      value_type loc_;

      URBI_CXX_OBJECT_(Location);
    };


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
