#ifndef URBI_USENDOPTIONS_HH
# define URBI_USENDOPTIONS_HH

# include <libport/utime.hh>

namespace urbi
{

  struct USendOptions
  {
    USendOptions(libport::utime_t = 0);
    USendOptions(const char *, const char *, libport::utime_t = 0);

    USendOptions& setUSeconds(libport::utime_t);
    USendOptions& setMTag(const char*);
    USendOptions& setMMod(const char*);

    libport::utime_t useconds;
    const char* mtag;
    const char* mmod;
    static const USendOptions default_options;
  };

}

# include <urbi/usendoptions.hxx>

#endif // !URBI_USENDOPTIONS_HH
