/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_ROOT_HH
# define URBI_ROOT_HH

# include <string>

/// The last component of the directory containing our libraries.
/// ("bin" on Windows, "lib" elsewhere).

extern const char* lib_rel_path;

/// Extension of the libraries (".dll", ".dylib", or ".so").
extern const char* lib_ext;

/// The name for "LD_LIBRARY_PATH" on this architecture.
extern const char* LD_LIBRARY_PATH_NAME;

/// Return $URBI_ROOT, or guess from argv[0].
std::string get_urbi_root(const char* arg0);

#endif
