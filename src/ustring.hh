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

  const char* str() const
  {
    return str_;
  }

  int len() const
  {
    return len_;
  }

  UString* copy() const;
  const char* ext(int deb, int length);
  bool tagequal(const UString& s) const;

  void update(const std::string& s);
  void update(const char* s);
  void update(const UString* s);

  void setLen(int l);

 private:
  int  len_;
  char* str_;
};

inline
bool
operator== (const UString& lhs, const UString& rhs)
{
  return STREQ(lhs.str(), rhs.str());
}

inline
bool
operator== (const UString& lhs, const char* rhs)
{
  return STREQ(lhs.str(), rhs);
}

inline
UString*
UString::copy() const
{
  return new UString(*this);
}

inline
void
UString::update(const std::string& s)
{
  update(s.c_str());
}

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

#endif // !USTRING_HH
