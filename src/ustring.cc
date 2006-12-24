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

#include "libport/cstring"

#include "ustring.hh"
#include "userver.hh"

MEMORY_MANAGER_INIT(UString);

UString::UString(const UString& s)
  : len_ (s.len_),
    str_ (s.str_ ? strdup (s.str_) : 0),
    fast_armor_ (s.fast_armor_)
{
  ADDOBJ(UString);
  ADDMEM(len_);
}

UString::UString(const std::string& s)
  : len_ (s.size ()),
    str_ (strdup (s.c_str ())),
    fast_armor_ (false)
{
  ADDOBJ(UString);
  ADDMEM(len_);
  fast_armor ();
}

UString::UString(const char* s)
  : len_ (s ? strlen(s) : 0),
    str_ (s ? strdup (s) : strdup("")),
    fast_armor_ (false)
{
  ADDOBJ(UString);
  ADDMEM(len_);
  fast_armor ();
}

UString::UString(const UString* s)
{
  ADDOBJ(UString);
  if (s==0)
  {
    len_ = 0;
    str_ = static_cast<char*> (malloc (1));
    strcpy(str_, "");
    fast_armor_ = true;
  }
  else
  {
    len_ = s->len();
    str_ = static_cast<char*> (malloc (len_+1));
    strcpy(str_, s->str());
    fast_armor_ = s->fast_armor_;
  }
  if (str_ == 0)
    len_ = 0;
  ADDMEM(len_);
}


UString::UString(const UString* s1, const UString* s2)
{
  ADDOBJ(UString);

  std::string tmpname = s1->str();
  tmpname = tmpname + "." + s2->str();

  str_ = static_cast<char*> (malloc (tmpname.length()+1));
  strcpy(str_, tmpname.c_str());

  if (str_ != 0)
    len_ = tmpname.length();
  else
    len_ = 0;
  fast_armor ();
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

bool UString::equal(const UString* s) const
{
  if (s == 0)
    return false;
  return STREQ(s->str(), str_);
}

bool UString::tagequal(const UString* s) const
{
  if (s == 0)
    return false;
  char* p = const_cast<char*>(strchr(s->str(), '.'));
  if (p)
    *p = 0;
  bool res = STREQ(s->str(), str_);
  if (p)
    *p = '.';
  return res;
}

bool UString::equal(const char* s) const
{
  if (s == 0)
    return false;
  return STREQ(s, str_);
}

void UString::update(const char* s)
{
  if (s == 0 || s == str_ /*|| (STREQ(s, str_)) */)
    return;

  if (str_)
    free(str_);
  FREEMEM(len_);
  int slen = strlen(s);
  str_ = static_cast<char*> (malloc (slen+1));
  strcpy(str_, s);
  len_ = slen;
  fast_armor ();
  ADDMEM(len_);
}

void UString::update(const UString* s)
{
  if (!s)
    return;
  if (str_)
    free(str_);
  FREEMEM(len_);

  str_ = static_cast<char*> (malloc (s->len()+1));
  strcpy(str_, s->str());
  len_ = s->len();
  fast_armor ();
  ADDMEM(len_);
}

char*
UString::un_armor ()
{
  char* cp = str_;
  int pos = 0;
  while (pos < len_)
  {
    if  (cp[0] == '\\' && pos + 1 < len_)
    {
      if (cp[1] == 'n'
	  || cp[1] == 't'
	  || cp[1] == '\\'
	  || cp[1] == '"')
      {
	if (cp[1] ==  'n')
	  cp[1] = '\n';
	if (cp[1] ==  't')
	  cp[1] = '\t';

	memmove (static_cast<void*> (cp),
		 static_cast<void*> (cp + 1),
		 len_ - pos);
	len_--;
	if (cp[0] == '\\')
	  cp++;
      }
      else
	cp++;
    }
    else
      cp++;
    pos = cp - str_;
  }

  return str_;
}

std::string
UString::armor ()
{
  // speedup
  if (fast_armor_)
    return std::string (str_);

  std::string res;
  res.reserve (len_);
  for (char* cp = str_; *cp; ++cp)
  {
    if (*cp=='"' || *cp=='\\')
      res += '\\';
    res += *cp;
  }
  return res;
}

void
UString::fast_armor ()
{
  fast_armor_= !strchr (str_, '"') && !strchr (str_, '\\');
}
