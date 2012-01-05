/*
 * Copyright (C) 2005-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/kernel/utypes.hh
/// \brief Definition of UErrorValue.

#ifndef KERNEL_UTYPES_HH
# define KERNEL_UTYPES_HH

# include <iosfwd>

/// Return code values
enum UErrorValue
{
  USUCCESS,
  UFAIL,
};

std::ostream& operator<< (std::ostream& o, UErrorValue v);

#endif // !KERNEL_UTYPES_HH
