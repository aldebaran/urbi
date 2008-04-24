/*! \file utypes.hh
 *******************************************************************************

 File: utypes.h\n
 Definition of useful types in the URBI server kernel.

 This file must be included if someone wants to reuse in an other context some
 generic classes like the UStream circular buffer. Actually, from a design point
 of view, this is the only reason to be for utype.h :-)

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

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
