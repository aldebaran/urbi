#ifndef URBI_EXPORT_HH
# define URBI_SDK_EXPORT_HH

# include <libport/detect-win32.h>

# ifdef WIN32
#  ifdef BUILDING_URBI_SDK
#   define URBI_SDK_API __declspec(dllexport)
#  else
#   define URBI_SDK_API __declspec(dllimport)
#  endif
# else
#  define URBI_SDK_API __attribute__((visibility("default")))
# endif

#endif // URBI_EXPORT_HH
