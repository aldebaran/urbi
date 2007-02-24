/*! \file ustring.cc
 *******************************************************************************

 File: ustring.cc\n
 Implementation of the UString class.

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
#include "libport/compiler.hh"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include "libport/cstring"

#include "ustring.hh"
#include "userver.hh"

MEMORY_MANAGER_INIT(UString);

UString::UString(const UString& s)
  : str_ (s.str_)
{
  ADDOBJ(UString);
  ADDMEM(size());
}

UString::UString(const std::string& s)
  : str_ (s)
{
  ADDOBJ(UString);
  ADDMEM(size());
}

UString::UString(const char* s)
  : str_ (s)
{
  ADDOBJ(UString);
  ADDMEM(size());
}

UString::UString(const UString& s1, const UString& s2)
  : str_ (s1.str_ + "." + s2.str_)
{
  ADDOBJ(UString);
  ADDMEM(size());
}


UString::~UString()
{
  FREEOBJ(UString);
  FREEMEM(size());
}

bool UString::tagequal(const UString& s) const
{
  // Get the prefix of S: the part before the first `.', or the whole string.
  size_t pos = s.str().find('.');
  std::string pre =
    ((pos && pos != std::string::npos)
     ? s.str().substr (pos - 1, s.size())
     : s.str());

  // Check that we are equal to that prefix.
  return str_ == pre;
}

UString&
UString::operator=(const char* s)
{
  assert (s);
  FREEMEM(size());
  str_ = s;
  ADDMEM(size());
  return *this;
}

UString&
UString::operator=(const UString* s)
{
  assert (s);
  FREEMEM(size());
  str_ = s->str_;
  ADDMEM(size());
  return *this;
}
