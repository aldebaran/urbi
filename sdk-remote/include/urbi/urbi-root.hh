/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
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
# include <vector>

# include <libport/detect-win32.h>
# include <libport/windows.hh>

# include <urbi/export.hh>

# ifdef WIN32
typedef HMODULE RTLD_HANDLE;
# else
typedef void* RTLD_HANDLE;
# endif

# ifndef URBI_ROOT_NOT_DLL
#  define URBI_SDK_MAYBE_API URBI_SDK_API
# else
#  define URBI_SDK_MAYBE_API
# endif

class URBI_SDK_MAYBE_API UrbiRoot
{
  /*---------------.
  | Construction.  |
  `---------------*/

public:
  /// Load an Urbi SDK.
  /// \param program The command used to invoke urbi-launch or urbi.
  /// \param static_build true if this is a static build:do not dlopen anything
  UrbiRoot(const std::string& program, bool static_build=false);

  /*---------------.
  | Entry points.  |
  `---------------*/

public:
  int urbi_launch(int argc, const char** argv);
  int urbi_launch(int argc, char** argv);
  int urbi_main(const std::vector<std::string>& args, bool block, bool errors);

  /*--------------------------.
  | UObject library loaders.  |
  `--------------------------*/

public:
  void load_plugin();
  void load_remote();
  void load_custom(const std::string& path);

  /*--------.
  | Paths.  |
  `--------*/

public:
  /// Root of the Urbi installation.
  const std::string& root() const;
  /// Plugin and remote libuobjects parent directory.
  std::string core_path() const;
  /// "doc" location. Contains THANKS.txt, etc.
  /// In a normal install, the same as share_dir.
  std::string doc_dir() const;
  /// "share" location. Contains urbi/, images/.
  std::string share_dir() const;
  /// Standard uobjects directories.
  std::vector<std::string> uobjects_path() const;

  /// The library suffix (e.g., -vc90 or -vc90-d for Visual.)
  /// Typically empty.
  std::string library_suffix() const;

private:
  /// Load the library "${libdir}/${base}${ext}", unless the envvar
  /// named "URBI_ROOT_LIB${BASE}" points to another location.
  RTLD_HANDLE
  library_load(const std::string& base, const std::string& env_suffix = "");

  std::string program_;
  std::string root_;
  RTLD_HANDLE handle_libjpeg_;
  RTLD_HANDLE handle_libport_;
  RTLD_HANDLE handle_libsched_;
  RTLD_HANDLE handle_libserialize_;
  RTLD_HANDLE handle_liburbi_;
  RTLD_HANDLE handle_libuobject_;
};

extern "C"
{
  typedef int(*urbi_launch_type)(int argc, const char* argv[], UrbiRoot& root);

  int
  URBI_SDK_API
  urbi_launch(int argc, const char* argv[], UrbiRoot& root);
}

#endif
