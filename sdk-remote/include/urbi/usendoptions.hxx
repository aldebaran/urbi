#ifndef URBI_USENDOPTIONS_HXX
# define URBI_USENDOPTIONS_HXX

namespace urbi
{

  inline
  USendOptions::USendOptions(libport::utime_t usecs)
    : useconds(usecs)
    , mtag(0)
    , mmod(0)
  {}

  inline
  USendOptions::USendOptions(const char* t,
			     const char* m,
			     libport::utime_t usecs)
    : useconds(usecs)
    , mtag(t)
    , mmod(m)
  {}

  inline USendOptions&
  USendOptions::setUSeconds(libport::utime_t usec)
  {
    useconds = usec;
    return *this;
  }

  inline USendOptions&
  USendOptions::setMTag(const char* tag)
  {
    mtag = tag;
    return *this;
  }

  inline USendOptions&
  USendOptions::setMMod(const char* mod)
  {
    mmod = mod;
    return *this;
  }

}

#endif // !URBI_USENDOPTIONS_HXX
