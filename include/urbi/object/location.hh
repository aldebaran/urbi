/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_LOCATION_HH
# define OBJECT_LOCATION_HH

# include <urbi/object/cxx-object.hh>
# include <urbi/object/position.hh>
# include <urbi/parser/location.hh>

namespace urbi
{
  namespace object
  {

    class URBI_SDK_API Location: public CxxObject
    {

    /*---------------.
    | Construction.  |
    `---------------*/

    public:
      typedef ::yy::location value_type;
      Location();
      Location(const value_type& loc);
      Location(rLocation model);
      void init();
      void init(const Position::value_type& begin);
      void init(const Position::value_type& begin,
                const Position::value_type& end);

    /*--------------.
    | Comparisons.  |
    `--------------*/

    public:
      bool operator ==(rLocation rhs) const;
      bool is_system_location() const;

    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      virtual std::string as_string() const;
      inline value_type& value_get();

    /*----------.
    | Details.  |
    `----------*/

    private:
      value_type loc_;

      URBI_CXX_OBJECT(Location, CxxObject);
    };

    bool
    operator ==(const Location::value_type& lhs,
                const Location::value_type& rhs);

    /*-------------.
    | Conversion.  |
    `-------------*/

    template <>
    struct CxxConvert<Location::value_type>
    {
      typedef       Location::value_type& target_type;
      typedef const Location::value_type& source_type;
      static target_type
      to(const rObject& o)
      {
        type_check<Location>(o);
        return o->as<Location>()->value_get();
      }

      static rObject
      from(source_type v)
      {
        return new Location(v);
      }
    };

  } // namespace object
} // namespace urbi


#include <urbi/object/location.hxx>

#endif
