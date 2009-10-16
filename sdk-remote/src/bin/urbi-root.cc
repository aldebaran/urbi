/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include "urbi-root.hh"

#include <cstring>
#include <cstdlib>

#ifdef WIN32

static const char* lib_rel_path = "bin";
static const char* lib_ext = ".dll";

typedef std::list<std::string> strings_type;

static strings_type
split(std::string lib)
{
  strings_type res;
  size_t pos;
  while ((pos = lib.find_first_of(':')) != lib.npos)
  {
    std::string s = lib.substr(0, pos);
    lib = lib.substr(pos + 1, lib.npos);
    // In case we split "c:\foo" into "c" and "\foo", glue them
    // together again.
    if (s[0] == '\\'
        && !res.empty()
        && res.back().length() == 1)
      res.back() += ':' + s;
    else
      res.push_back(s);
  }
  if (!lib.empty())
    res.push_back(lib);
  return res;
}


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
