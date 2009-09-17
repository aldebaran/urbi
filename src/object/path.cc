/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
// For stat, getcwd
#include <libport/cerrno>
#include <libport/cstring>
#include <libport/fcntl.h>
#include <libport/sys/types.h>
#include <libport/sys/stat.h>

#include <libport/unistd.h>
#include <libport/file-system.hh>
#include <libport/lexical-cast.hh>

// For bad_alloc
#include <exception>

#include <libport/format.hh>

#include <object/directory.hh>
#include <object/file.hh>
#include <object/path.hh>
#include <object/symbols.hh>
#include <runner/raise.hh>

namespace object
{
  using boost::format;

  /*--------------.
  | C++ methods.  |
  `--------------*/

  const Path::value_type& Path::value_get() const
  {
    return path_;
  }


  /*---------------.
  | Urbi methods.  |
  `---------------*/

  // Construction

  Path::Path()
    : path_("/")
  {
    proto_add(proto);
  }

  Path::Path(rPath model)
    : path_(model->value_get())
  {
    proto_add(model);
  }

  Path::Path(const std::string& value)
    : path_(value)
  {
    proto_add(proto ? proto : Object::proto);
  }

  void Path::init(const std::string& path)
  {
    path_ = path;
  }

  bool
  Path::operator<=(const rPath& rhs) const
  {
    return path_.to_string() <= rhs->path_.to_string();
  }


  /*---------------------.
  | Global information.  |
  `---------------------*/

  rPath Path::cwd()
  {
    return new Path(libport::get_current_directory());
  }

  // Stats

  bool Path::absolute()
  {
    return path_.absolute_get();
  }

  bool Path::exists()
  {
    if (!::access(path_.to_string().c_str(), F_OK))
      return true;
    if (errno == ENOENT)
      return false;
    handle_any_error();
  }

  struct stat Path::stat()
  {
    struct stat res;

    if (::stat(path_.to_string().c_str(), &res))
      handle_any_error();

    return res;
  }

  bool Path::is_dir()
  {
    return stat().st_mode & S_IFDIR;
  }

  bool Path::is_reg()
  {
    return stat().st_mode & S_IFREG;
  }

  bool Path::readable()
  {
    if (!::access(path_.to_string().c_str(), R_OK))
      return true;
    if (errno == EACCES)
      return false;
    handle_any_error();
  }

  bool Path::writable()
  {
    if (!::access(path_.to_string().c_str(), W_OK))
      return true;
    if (errno == EACCES || errno == EROFS)
      return false;
    handle_any_error();
  }

  // Operations

  std::string Path::basename()
  {
    return path_.basename();
  }

  rPath Path::cd()
  {
    if (chdir(as_string().c_str()))
      handle_any_error();
    return cwd();
  }

  rPath Path::path_concat(rPath other)
  {
    try
    {
      return new Path(path_ / other->path_);
    }
    catch (const libport::path::invalid_path& e)
    {
      RAISE(e.what());
    }
  }

  rPath Path::string_concat(rString other)
  {
    return path_concat(new Path(other->value_get()));
  }

  OVERLOAD_TYPE(concat_bouncer, 1, 1,
		Path, &Path::path_concat,
		String, &Path::string_concat);

  rPath Path::dirname()
  {
    return new Path(path_.dirname());
  }

  rObject Path::open()
  {
    if (is_dir())
      return new Directory(path_);
    if (is_reg())
      return new File(path_);
    RAISE(str(format("Unsupported file type: %s.") % path_));
  }

  // Conversions

  std::string Path::as_string()
  {
    return path_.to_string();
  }

  std::string Path::as_printable()
  {
    return str(format("Path(\"%s\")") % as_string());
  }

  rList Path::as_list()
  {
    List::value_type res;
    foreach (const std::string& c, path_.components())
      res << new String(c);
    return new List(res);
  }


  /*--------.
  | Details |
  `--------*/

  void Path::handle_any_error()
  {
    RAISE(str(format("%1%: %2%") % strerror(errno) % path_));
  }

  /*---------------.
  | Binding system |
  `---------------*/

  void Path::initialize(CxxObject::Binder<Path>& bind)
  {
    bind(SYMBOL(EQ_EQ),
         static_cast<bool (self_type::*)(const rObject&) const>
                    (&self_type::operator==));
    bind(SYMBOL(SLASH), concat_bouncer);

#define DECLARE(Urbi, Cxx)                      \
    bind(SYMBOL(Urbi), &Path::Cxx)

    DECLARE(LT_EQ, operator<=);
    DECLARE(absolute, absolute);
    DECLARE(asList, as_list);
    DECLARE(asPrintable, as_printable);
    DECLARE(asString, as_string);
    DECLARE(basename, basename);
    DECLARE(cd, cd);
    DECLARE(cwd, cwd);
    DECLARE(dirname, dirname);
    DECLARE(exists, exists);
    DECLARE(init, init);
    DECLARE(isDir, is_dir);
    DECLARE(isReg, is_reg);
    DECLARE(open, open);
    DECLARE(readable, readable);
    DECLARE(writable, writable);
  }

  rObject
  Path::proto_make()
  {
#ifdef WIN32
    return new Path("C:\\");
#else
    return new Path("/");
#endif
  }

  URBI_CXX_OBJECT_REGISTER(Path);
}
