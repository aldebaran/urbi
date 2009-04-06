#ifndef LIBUOBJECT_COMPATIBILITY_HH
# define LIBUOBJECT_COMPATIBILITY_HH

namespace urbi
{
  namespace compatibility
  {
    /// Set up the channel \a name.  Must be followed by channel_destroy().
    std::string channel_construct(const std::string& name);

    /// Destroy a channel set up by channel_construct().
    std::string channel_destroy(const std::string& name);

    /// Return the string to emit \a event in k1 or k2.
    std::string emit(const std::string& event);

    /// Return the string to test whether \a exp is void in k1 or k2.
    std::string isvoid(const std::string& exp);
  }
}

# include "compatibility.hxx"

#endif // LIBUOBJECT_COMPATIBILITY_HH
