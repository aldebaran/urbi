/*! \file ucallid.h
 *******************************************************************************

 File: ucallid.h\n
 Definition of the UCallid class.

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

#ifndef UCALLID_H_DEFINED
#define UCALLID_H_DEFINED

#include <list>

#include "fwd.hh"
#include "ustring.h"
#include "uvariable.h"


// ****************************************************************************
//! Contains a group definition, as a result of a GROUP command
class UCallid
{
public:

  UCallid(const char* fun_id, const char* self_id, UCommand_TREE* root);
  ~UCallid();

  std::list<UVariable*> stack;
  UString* fun_id;
  UString* self_id;
  UCommand_TREE* root;
  UVariable* returnVar;

  void         store(UVariable *variable);
  const char*  str();
  const char*  self();
  void         setReturnVar( UVariable *v);
};

#endif
