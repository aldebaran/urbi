/*! \file mem-track.hh
 *******************************************************************************

 File: mem-track.h\n
 Definition of useful types in the URBI server kernel.

 This file must be included if someone wants to reuse in an other context some
 generic classes like the UStream circular buffer. Actually, from a design point
 of view, this is the only reason to be for utype.h :-)

 This file is part of
 %URBI Kernel, version __kernelversion__\n
Copyright (c) 2007 Jean-Christophe Baillie.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef MEM_TRACK_HH
# define MEM_TRACK_HH

/*--------------------.
| Memory allocation.  |
`--------------------*/

/// Keep track of how much memory has been used for commands, buffers,
/// etc.
extern  int   usedMemory;
/// Total amount of free memory in the system.
extern  int   availableMemory;

// FIXME: Why applying the 1.15 threshold here instead of where we
// consult usedMemory?
# define ADDMEM(X)   usedMemory += static_cast<int> ((X) * 1.15)
# define FREEMEM(X)  usedMemory -= static_cast<int> ((X) * 1.15)


# define ADDOBJ(X)   ADDMEM (sizeof (X))
# define FREEOBJ(X)  FREEMEM (sizeof (X))

# define LIBERATE(X)				\
  do {						\
    if ((X) && (X)->liberate() == 0)		\
      delete X;					\
  } while (0)

#endif // MEM_TRACK_HH
