/*! \file uproperty.hh
 *******************************************************************************

 File: uproperty.h\n
 Definition of the UProperty class.

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

#ifndef UPROPERTY_HH
# define UPROPERTY_HH

# include "kernel/fwd.hh"

//! Contains a group definition, as a result of a GROUP command
class UProperty
{
public:
  UProperty(UVariableName* variablename,
	    UString* property);

  ~UProperty();

  UVariableName* variablename;
  UString* property;
};

#endif
