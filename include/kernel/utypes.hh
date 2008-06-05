/// \file kernel/utypes.hh
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
