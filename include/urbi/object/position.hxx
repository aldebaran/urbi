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

  } // namespace object
} // namespace urbi

#endif
