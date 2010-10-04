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

    /*--------------.
    | Comparisons.  |
    `--------------*/

    inline
    bool
    Location::operator ==(rLocation rhs) const
    {
      return loc_ == rhs->loc_;
    }

  } // namespace object
} // namespace urbi

#endif
