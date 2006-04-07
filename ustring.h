/*! \file ustring.h
 *******************************************************************************

 File: ustring.h\n
 Definition of useful types in the URBI server kernel.

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

#ifndef USTRING_H_DEFINED
#define USTRING_H_DEFINED

#include <string.h>
#include <stdlib.h>
#include "memorymanager/memorymanager.h"

//! UString is used to handle strings in the URBI server
/*! The only reason why we had to introduce UString is to keep a tight control on how
    memory is managed, as far as strings are concerned.
    This class does all the ADDMEM/FREEMEM job for you. Use it.
*/   
class UString {
public:
  MEMORY_MANAGED;
  UString(const char* s);
  UString(UString *s);
  UString(UString *s1,UString *s2); ///< concat s1 and s2 with a dot in the middle...

  ~UString();

  const char* str() const;
// {
//    return str_;
//  }

  int len() {
    return len_;
  }
  
  UString* copy() {
    
    return (new UString(this));
  }

  char* ext(int deb, int length);
  bool equal(UString *s);
  bool equal(const char *s);
  void update(const char *s);
  void update(UString *s);

private:

  int  len_;
  char *str_;
};

#endif
