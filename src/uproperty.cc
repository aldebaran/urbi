/*! \file uproperty.cc
 *******************************************************************************

 File: uproperty.cc\n
 Implementation of the UProperty class.

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

#include "uproperty.hh"
#include "uvariablename.hh"
#include "ustring.hh"


// **************************************************************************
//! UProperty constructor
UProperty::UProperty( UVariableName *variablename,
		      UString *property)
{
  this->variablename = variablename;
  this->property = property;
}

//! UProperty destructor
UProperty:: ~UProperty()
{
  delete variablename;
  delete property;
}
