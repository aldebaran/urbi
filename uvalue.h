/*! \file uvalue.h
 *******************************************************************************

 File: uvalue.h\n
 Definition of the UValue class.

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

#ifndef UVALUE_H_DEFINED
#define UVALUE_H_DEFINED

#include "memorymanager/memorymanager.h"
#include "utypes.h"
#include "ustring.h"
#include "ufloat.h"

namespace urbi {
  class UValue;
};

class UBinary;
class UConnection;


// *****************************************************************************
//! Contains a value: can be numeric, string, binary
class UValue
{
public:
  MEMORY_MANAGED;
  UValue();
  UValue(UFloat val);
  UValue(const char* str);
  UValue(urbi::UValue);
  ~UValue();

  UDataType       dataType;     ///< Type of the value

  UFloat val; // must be out of the union in case of reimplementation
  union {    ///< union of the possible types
    UString *str;    
    URefPt<UBinary> *refBinary;    
  };

  int eventid; ///< Used to identify the events from which the boolean value come from
  UValue *liststart;
  UValue *next;

  UValue* copy();
  UValue* add(UValue* v);
  bool    equal(UValue* v);
  void    echo(UConnection* connection, bool human_readable=false);

  urbi::UValue* urbiValue();
};

UTestResult booleval(UValue *,bool freeme = true);

#endif
