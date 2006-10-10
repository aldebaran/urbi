/*! \file unamedparameters.h
 *******************************************************************************

 File: unamedparameters.h\n
 Definition of the UNamedParameters class.

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

#ifndef UNAMEDPARAMETERS_H_DEFINED
#define UNAMEDPARAMETERS_H_DEFINED

#include "fwd.hh"
#include "utypes.h"
#include "ustring.h"

// *****************************************************************************
//! Contains any list of identifiers, expressions or combinaison of both.
class UNamedParameters
{
public:
  UNamedParameters(UString* name,
		   UExpression *expression,
		   UNamedParameters* next=0);

  UNamedParameters(UExpression *expression, UNamedParameters* next = 0);
  virtual ~UNamedParameters();

  void print();

  UNamedParameters* rank(int n);
  int               size();
  UNamedParameters* copy();

  UString            *name;         ///< The name
  UExpression        *expression;   ///< The expression
  UNamedParameters   *next;         ///< Next element in the list
};

#endif
