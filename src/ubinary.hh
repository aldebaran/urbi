/*! \file ubinary.hh
 *******************************************************************************

 File: ubinary.h\n
 Definition of the UBinary class.

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

#ifndef UBINARY_HH
# define UBINARY_HH

# include "fwd.hh"
# include "utypes.hh"


/// Binary data, an the binary buffer in particular.
class UBinary
{
public:
  UBinary(int bufferSize, UNamedParameters *parameters);
  ~UBinary();

  void     print() const;

  /// The size of the buffer.
  int               bufferSize;
  /// The buffer used to store the binary data.
  ubyte             *buffer;
  /// The optional parameters of the binary.
  UNamedParameters  *parameters;
};

#endif
