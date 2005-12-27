/*! \file utype.h
 *******************************************************************************

 File: utype.h\n
 Definition of useful types in the URBI server kernel.

 This file must be included if someone wants to reuse in an other context some 
 generic classes like the UStream circular buffer. Actually, from a design point
 of view, this is the only reason to be for utype.h :-)

 This file is part of 
 %URBI, version 1.0\n
 (c) Jean-Christophe Baillie, 2004.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://urbi.sourceforge.net

 **************************************************************************** */

#ifndef UTYPE_H_DEFINED
#define UTYPE_H_DEFINED

enum UErrorValue {
  USUCCESS,
  UFAIL
};

typedef unsigned char ubyte;

#endif
