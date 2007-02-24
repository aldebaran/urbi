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
# define USTRING_HH

# include "libport/cstring"
# include <string>
# include <iosfwd>
# include "memorymanager/memorymanager.hh"

//! UString is used to handle strings in the URBI server
/*! The only reason why we had to introduce UString is to keep a
    tight control on how memory is managed, as far as strings are concerned.
    This class does all the ADDMEM/FREEMEM job for you. Use it.
*/
class UString
{
 public:
  MEMORY_MANAGED;
  UString(const char* s = 0);
  UString(const UString& s);
  UString(const std::string& s);

  /// Concat \c s1 and \c s2 with a dot in the middle.
  UString(const UString& s1, const UString& s2);

  ~UString();

  const char* c_str() const
  {
    return str_;
  }

  int size() const
  {
    return len_;
  }

  UString* copy() const;
  const char* ext(int deb, int length);
  bool tagequal(const UString& s) const;

  UString& operator= (const std::string& s);
  UString& operator= (const char* s);
  UString& operator= (const UString* s);

 private:
  int  len_;
  char* str_;
};

inline
bool
operator== (const UString& lhs, const UString& rhs)
{
  return STREQ(lhs.c_str(), rhs.c_str());
}

inline
bool
operator!= (const UString& lhs, const UString& rhs)
{
  return !(lhs == rhs);
}


inline
bool
operator== (const UString& lhs, const char* rhs)
{
  return STREQ(lhs.c_str(), rhs);
}

inline
bool
operator!= (const UString& lhs, const char* rhs)
{
  return !(lhs == rhs);
}

inline
UString*
UString::copy() const
{
  return new UString(*this);
}

inline
UString&
UString::operator=(const std::string& s)
{
  return *this = s.c_str();
}

inline
std::ostream&
operator<< (std::ostream& o, const UString& s)
{
  if (s.c_str())
    return o << s.c_str();
  else
    return o << "<null UString>";
}

// Update a pointer to UString.
inline
UString*
update (UString*& s, const char* v)
{
  if (s)
    *s = v;
  else
    s = new UString(v);
  return s;
}


#endif // !USTRING_HH
