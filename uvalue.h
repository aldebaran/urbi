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

#include "utypes.h"
#include "ustring.h"

class UBinary;

// *****************************************************************************
//! Contains a value: can be numeric, string, binary
class UValue
{
public:

  UValue();
  UValue(double val);
  UValue(const char* str);
  ~UValue();

  UDataType       dataType;     ///< Type of the value

  union {    ///< union of the possible types
    double  val;
    UString *str;    
    URefPt<UBinary> *refBinary;    
  };

  int eventid; ///< Used to identify the events from which the boolean value come from
  UValue *list;

  UValue* copy();
  UValue* add(UValue* v);
  bool    equal(UValue* v);
};

UTestResult booleval(UValue *,bool freeme = true);

#endif
