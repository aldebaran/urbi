/*! \file uvariablelist.hh
 *******************************************************************************

 File: uvariablelist.h\n
 Definition of the UVariableList class.

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

#ifndef UVARIABLELIST_HH
#define UVARIABLELIST_HH

#include "fwd.hh"
#include "utypes.hh"


// *****************************************************************************
//! Contains a list of variable names
class UVariableList
{
public:
  UVariableList(UVariableName* variablename,
		UVariableList* next=0);

  virtual ~UVariableList();

  void print();

  UVariableList* rank(int n);
  int            size();
  UVariableList* copy();

  UVariableName      *variablename; ///< The name
  UVariableList      *next;         ///< Next element in the list
};

#endif
