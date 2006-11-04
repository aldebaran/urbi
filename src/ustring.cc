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

#include <cstring>
#include <cstdlib>

#include "ustring.h"
#include "userver.h"

MEMORY_MANAGER_INIT(UString);

UString::UString(const char* s)
{
  ADDOBJ(UString);
  int slen = s?strlen(s):0;
  if (s==0)
    str_ = strdup("");
  else
    {
      str_ = (char*)malloc(slen+1);
      strcpy(str_, s);
    }

  len_ = slen;
  ADDMEM(len_);
}

UString::UString(UString *s)
{
  ADDOBJ(UString);
  if (s==0)
  {
    len_ = 0;
    str_ = (char*)malloc(1);
    strcpy(str_, "");
  }
  else
  {
    len_ = s->len();
    str_ = (char*)malloc(len_+1);
    strcpy(str_, s->str());
  }
  if (str_ == 0)
    len_ = 0;
  ADDMEM(len_);
}


UString::UString(UString *s1, UString* s2)
{
  ADDOBJ(UString);

  std::string tmpname = s1->str();
  tmpname = tmpname + "." + s2->str();

  str_ = (char*)malloc(tmpname.length()+1);
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
  if (str_) free(str_);
  FREEMEM(len_);
}

char* UString::ext(int deb, int length)
{
  if (length<0)
    length=0;
  if (deb>=len_)
    return str_+len_;
  if (deb+length<len_)
    str_[deb+length]=0;
  return str_+deb;
}

bool UString::equal(UString *s)
{
  if (s==0)
    return false;
  return (strcmp(s->str(), (const char*)str_)==0);
}

bool UString::tagequal(UString *s)
{
  if (s==0)
    return false;
  char* p = const_cast<char*>(strchr(s->str(), '.'));
  if (p) p[0]=0;
  bool res = strcmp(s->str(), (const char*)str_) == 0;
  if (p)
    p[0]='.';
  return res;
}

bool UString::equal(const char *s)
{
  if (s==0)
    return false;
  return strcmp(s, (const char*)str_)==0;
}

void UString::update(const char *s)
{
  if (s==0 || s == str_ /*|| (strcmp(s,str_)==0) */)
    return;

  if (str_)
    free(str_);
  FREEMEM(len_);
  int slen = strlen(s);
  str_ = (char*)malloc(slen+1);
  strcpy(str_, s);
  len_ = slen;
  ADDMEM(len_);
}

void UString::update(UString *s)
{
  if (!s)
    return;
  if (str_)
    free(str_);
  FREEMEM(len_);

  str_ = (char*)malloc(s->len()+1);
  strcpy(str_, s->str());
  len_ = s->len();
  ADDMEM(len_);

}

char*
UString::unArmor ()
{
  char* position = str_;
  int pos = 0;
  while (pos < len_)
  {
    if  (*position=='\\' && pos+1<len_)
    {
      if (   *(position+1) == 'n'
          || *(position+1) == 't'
          || *(position+1) == '\\'
          || *(position+1) == '"')
      {
        if ( *(position+1) ==  'n') *(position+1) = '\n';
        if ( *(position+1) ==  't') *(position+1) = '\t';

        memmove((void*)position,
                (void*)(position+1),
                len_-pos);
        len_ --;
        if (*position=='\\') position++;
      }
      else
        position++;
    }
    else
      position++;
    pos = position - str_;
  }

  return str_;
}

char*
UString::armor ()
{
  // unimplemented
  return str_;
}

