#ifndef URBI_USENDOPTIONS_HH
# define URBI_USENDOPTIONS_HH

namespace urbi
{

  struct USendOptions
  {
    USendOptions(libport::utime_t = 0);
    USendOptions(const char *, const char *, libport::utime_t = 0);

    libport::utime_t useconds;
    const char* mtag;
    const char* mmod;
  };

}

# include <urbi/usendoptions.hxx>

#endif // !URBI_USENDOPTIONS_HH
