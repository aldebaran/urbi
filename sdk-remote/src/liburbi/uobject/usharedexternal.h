/*! \file usharedexternal.h
 *******************************************************************************

 File: usharedexternal.h\n
 Definition of common structures between modules and the kernel

 This file is part of LIBURBI and URBI Kernel\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#ifndef USHAREDEXTERNAL_H_DEFINED
#define USHAREDEXTERNAL_H_DEFINED

namespace URBI {

  enum USystemExternalMessage {
    UEM_EVALFUNCTION,
    UEM_ASSIGNVALUE,
    UEM_EMITEVENT,
    UEM_ENDEVENT
  };
} // namespace URBI

#endif

