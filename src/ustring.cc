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

#include <cstdlib>
#include <sstream>
#include <iostream>
#include "libport/cstring"

#include "ustring.hh"
#include "userver.hh"

MEMORY_MANAGER_INIT(UString);

UString::UString(const UString& s)
  : len_ (s.len_),
    str_ (s.str_ ? strdup (s.str_) : 0)
{
  ADDOBJ(UString);
  ADDMEM(len_);
}

UString::UString(const std::string& s)
  : len_ (s.size ()),
    str_ (strdup (s.c_str ()))
{
  ADDOBJ(UString);
  ADDMEM(len_);
}

UString::UString(const char* s)
  : len_ (s ? strlen(s) : 0),
    str_ (s ? strdup (s) : strdup(""))
{
  ADDOBJ(UString);
  ADDMEM(len_);
}

UString::UString(const UString& s1, const UString& s2)
{
  ADDOBJ(UString);

  std::string tmpname = s1.str();
  tmpname = tmpname + "." + s2.str();

  str_ = static_cast<char*> (malloc (tmpname.length()+1));
  strcpy(str_, tmpname.c_str());

  if (str_ != 0)
    len_ = tmpname.length();
  else
    len_ = 0;
  ADDMEM(len_);
}


UString::~UString()
{
  FREEOBJ(UString);
  if (str_)
    free(str_);
  FREEMEM(len_);
}

const char* UString::ext(int deb, int length)
{
  if (length<0)
    length=0;
  if (deb>=len_)
    return str_+len_;
  if (deb+length<len_)
    str_[deb+length]=0;
  return str_+deb;
}

bool UString::tagequal(const UString& s) const
{
  // Oh, my God...
  char* p = const_cast<char*>(strchr(s.str(), '.'));
  if (p)
    *p = 0;
  bool res = STREQ(s.str(), str_);
  if (p)
    *p = '.';
  return res;
}

UString&
UString::operator=(const char* s)
{
  if (s == 0 || s == str_ /*|| (STREQ(s, str_)) */)
    return *this;

  if (str_)
    free(str_);
  FREEMEM(len_);
  int slen = strlen(s);
  str_ = static_cast<char*> (malloc (slen+1));
  strcpy(str_, s);
  len_ = slen;
  ADDMEM(len_);
  return *this;
}

UString&
UString::operator=(const UString* s)
{
  if (!s)
    return *this;
  if (str_)
    free(str_);
  FREEMEM(len_);

  str_ = static_cast<char*> (malloc (s->len()+1));
  strcpy(str_, s->str());
  len_ = s->len();
  ADDMEM(len_);
  return *this;
}
