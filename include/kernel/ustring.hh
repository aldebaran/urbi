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

# include "kernel/memorymanager.hh"

//! UString is used to handle strings in the URBI server
/*! The only reason why we had to introduce UString is to keep a
    tight control on how memory is managed, as far as strings are concerned.
    Since we no longer manage memory, this class is now useless and ought to
    be either removed or replaced by Symbol.
*/
class UString
{
 public:
  MEMORY_MANAGED;
  UString(const char* s);
  UString(const UString& s);
  UString(const std::string& s);

  UString& operator= (const char* s);
  UString& operator= (const UString* s);
  UString& operator= (const std::string& s);

  /// Concat \c s1 and \c s2 with a dot in the middle.
  UString(const UString& s1, const UString& s2);

  ~UString();

  /// The underlying standard string.
  const std::string& str() const
  {
    return str_;
  }

  std::string& str()
  {
    return str_;
  }

  /// The content as a C string.
  const char* c_str() const
  {
    return str_.c_str();
  }

  /// The length of the string.
  size_t size() const
  {
    return str_.size();
  }

  UString* copy() const;

 private:
  std::string str_;
};

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
  str_ = s;
  return *this;
}


/*-------------------------.
| Freestanding functions.  |
`-------------------------*/

inline
std::ostream&
operator<< (std::ostream& o, const UString& s)
{
  return o << s.str();
}

inline
bool
operator== (const UString& lhs, const UString& rhs)
{
  return lhs.str() == rhs.str();
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
  return lhs.str() == rhs;
}

inline
bool
operator!= (const UString& lhs, const char* rhs)
{
  return !(lhs == rhs);
}


inline
UString
operator+ (const UString& lhs, const UString& rhs)
{
  return lhs.str() + rhs.str();
}



/// Return the part after the first `.', or the whole string if there is none.
std::string suffix (const std::string& name);

/// Return the part before the `.', or an empty string.
std::string prefix (const std::string& name);

/// Return the part after the first `.', or the whole string if there is none.
inline
std::string suffix (const UString& name)
{
  return suffix(name.str());
}

/// Return the part before the `.', or an empty string.
inline
std::string prefix (const UString& name)
{
  return prefix(name.str());
}

/// Update a pointer to UString.
inline
UString*
update (UString*& s, const std::string& v)
{
  if (s)
    *s = v;
  else
    s = new UString(v);
  return s;
}

#endif // !USTRING_HH
