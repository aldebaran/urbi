#ifndef LIBUOBJECT_COMPATIBILITY_HH
# define LIBUOBJECT_COMPATIBILITY_HH

namespace urbi
{
  namespace compatibility
  {
    /// Return the string to emit \a event in k1 or k2.
    std::string emit(const std::string& event);

    /// Return the string to test whether \a exp is void in k1 or k2.
    std::string isvoid(const std::string& exp);
  }
}

# include "compatibility.hxx"

#endif // LIBUOBJECT_COMPATIBILITY_HH
