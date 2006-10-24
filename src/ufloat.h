#ifndef UFLOAT_H
# define UFLOAT_H

# include "config.h"

// Floating point definition (emulated or real)
# ifdef HAVE_LIBPORT_UFLOAT_HH
#  include "libport/ufloat.hh"
# else
namespace urbi
{ 
  typedef double ufloat; 
  static const ufloat PI = ufloat(3.14159265358979323846264338327950288);
}
# endif

static const urbi::ufloat UINFINITY   = urbi::ufloat(999999999999999.0);

// Currently we don't prefix all the uses in the kernel.
using urbi::ufloat;
using urbi::PI;

#endif
