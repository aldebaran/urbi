/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <cstring>
#include <cstdlib>

#include <libport/config.h>
#include <libport/detect-win32.h>

#include "urbi-root.hh"

#ifdef WIN32

const char* lib_rel_path = "bin";
const char* lib_ext = ".dll";

#else
const char* lib_rel_path = "lib";
# ifdef __APPLE__
const char* LD_LIBRARY_PATH_NAME = "DYLD_LIBRARY_PATH";
const char* lib_ext = ".dylib";
# else
const char* LD_LIBRARY_PATH_NAME = "LD_LIBRARY_PATH";
const char* lib_ext = ".so";
# endif
#endif

/// Return the content of URBI_ROOT, or extract it from argv[0].
std::string
get_urbi_root(const char* arg0)
{
  const char* uroot = getenv("URBI_ROOT");
  if (uroot)
    return uroot;
  int p = 0;
  for (p = strlen(arg0)-1; p>=0 && arg0[p] != '/' && arg0[p] != '\\'
         ; --p)
    ;
  if (p<0)
    return ".";
  return std::string(arg0, arg0 + p) + "/..";
}
