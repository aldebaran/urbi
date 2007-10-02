/*! \file ustring.hh
 *******************************************************************************

 File: ustring.h\n
 Definition of useful types in the URBI server kernel.

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

#ifndef USTRING_HH
#define USTRING_HH

#include <string>
#include <iosfwd>
#include "memorymanager/memorymanager.hh"

//! UString is used to handle strings in the URBI server
/*! The only reason why we had to introduce UString is to keep a
    tight control on how memory is managed, as far as strings are concerned.
    This class does all the ADDMEM/FREEMEM job for you. Use it.
*/
class UString
{
 public:
  MEMORY_MANAGED;
  UString(const char* s);
  UString(const UString* s);
  UString(const UString& s);
  UString(const std::string& s);

  /// Concat \c s1 and \c s2 with a dot in the middle.
  UString(const UString* s1, const UString* s2);

  ~UString();

  const char* str() const
  {
    return str_;
  }

  int len() const
  {
    return len_;
  }

  UString* copy() const
  {
    return new UString(this);
  }

  const char* ext(int deb, int length);
  bool equal(const UString *s) const;
  bool tagequal(const UString *s) const;
  bool equal(const char *s) const;

  void update(const char *s);
  void update(const UString *s);

  void setLen(int l);

  /// Decode \n, \\, \", and \t, in place.
  char* un_armor();

  // Return the string with " and \ escaped.
  std::string armor();

  /// put "p1.p2" in name
  static inline void makeName(char * name, const UString &p1, 
				const UString & p2) {
    memcpy(name,  p1.str(), p1.len());
    name[p1.len()] = '.';
    memcpy(name + p1.len() +1, p2.str(), p2.len());
    name[p1.len() + 1 + p2.len()] = 0;
  }

  /// put "p1.p2" in name
  static inline void makeName(char * name, const UString &p1, 
				const char * p2) {
    memcpy(name,  p1.str(), p1.len());
    name[p1.len()] = '.';
    strcpy(name + p1.len() +1, p2);
  }
 private:
  int  len_;
  char* str_;
};

inline void
UString::setLen(int l)
{
  len_ = l;
}

inline
std::ostream&
operator<< (std::ostream& o, const UString& s)
{
  if (s.str())
    return o << s.str();
  else
    return o << "<null UString>";
}

#endif
