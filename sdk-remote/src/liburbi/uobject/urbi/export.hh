#ifndef URBI_EXPORT_HH
# define URBI_EXPORT_HH
#include <libport/detect-win32.h>

# if defined WIN32
#  if defined BUILDING_URBI_SDK
#   define USDK_API __declspec(dllexport)
#  else
#   define USDK_API __declspec(dllimport)
#  endif
# else
# define USDK_API
# endif


#endif
