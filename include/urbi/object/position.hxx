/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_POSITION_HXX
# define OBJECT_POSITION_HXX

# include <urbi/object/position.hh>

namespace urbi
{
  namespace object
  {

    /*--------------.
    | Conversions.  |
    `--------------*/

    inline Position::value_type& Position::value_get()
    {
      return pos_;
    }

    /*-------------.
    | Comparison.  |
    `-------------*/

    inline
    bool
    Position::operator ==(rPosition rhs) const
    {
      return pos_ == rhs->pos_;
    }

    inline
    bool
    Position::operator <(rPosition rhs) const
    {
      return pos_ < rhs->pos_;;
    }

    /*-----------.
    | Accessor.  |
    `-----------*/

    inline unsigned int* Position::line_ref()
    {
      return &pos_.line;
    }

    inline unsigned int* Position::column_ref()
    {
      return &pos_.column;
    }

    /*-----------------.
    | ::yy::position.  |
    `-----------------*/

    template <>
    struct CxxConvert<Position::value_type>
    {
      typedef Position::value_type target_type;
      static target_type
      to(const rObject& o, unsigned idx)
      {
        type_check<Position>(o, idx);
        return o->as<Position>()->value_get();
      }

      static rObject
      from(target_type v)
      {
        return new Position(v);
      }
    };


  } // namespace object
} // namespace urbi

#endif
