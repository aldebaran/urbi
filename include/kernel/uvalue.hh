/*! \file uvalue.hh
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

#ifndef UVALUE_HH
# define UVALUE_HH

# include <string>

# include "libport/fwd.hh"
# include "libport/ufloat.h"
# include "libport/ref-pt.hh"

# include "kernel/fwd.hh"
# include "kernel/memorymanager.hh"
# include "kernel/utypes.hh"

// ****************************************************************************
//! Contains a value: can be numeric, string, binary
class UValue
{
public:
  MEMORY_MANAGED;
  UValue();
  UValue(ufloat val);
  UValue(const char* str);
  UValue(const urbi::UValue&);

  /// \pre \a t is DATA_FILE, or DATA_STRING, or DATA_OBJ.
  UValue(UDataType t, const char* s);

  UValue& operator = (const urbi::UBinary&);
  UValue& operator = (const urbi::UImage&);
  UValue& operator = (const urbi::USound&);
  UValue& operator = (const urbi::UList&);
  operator urbi::UImage ();
  operator urbi::USound();
  operator urbi::UList();
  operator urbi::UBinary();
  operator urbi::UBinary*();
  ~UValue();

  /// Type of the value.
  UDataType dataType;

  ufloat val; // must be out of the union in case of reimplementation
  /// Union of the possible types.
  union
  {
    UString* str;
    libport::RefPt<UBinary>* refBinary;
  };

  UValue* liststart;
  UValue* next;

  UValue* copy() const;
  UValue* add(UValue* v);
  bool	  equal(UValue* v);
  void	  echo(UConnection* connection, bool human_readable=false);
  std::string  echo(bool human_readable=false);

  urbi::UValue* urbiValue();
private:
  UValue& operator = (const UValue &);
};

UTestResult booleval(UValue* , bool freeme = true);

#endif
