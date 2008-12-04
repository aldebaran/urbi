#ifndef URBI_EXPORT_HH
# define URBI_EXPORT_HH

# include <libport/detect-win32.h>

# ifdef BUILDING_URBI_SDK
#  ifdef WIN32
#   define URBI_SDK_API __declspec(dllexport)
#  else
#    define URBI_SDK_API __attribute__((visibility("default")))
#  endif
# endif

# ifndef URBI_SDK_API
#  ifdef WIN32
#   define URBI_SDK_API __declspec(dllimport)
#  else
#   define URBI_SDK_API
#  endif
# endif

#endif
