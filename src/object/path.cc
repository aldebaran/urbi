// For stat, getcwd
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <libport/unistd.h>
#include <fcntl.h>

// For bad_alloc
#include <exception>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <object/directory.hh>
#include <object/file.hh>
#include <object/path.hh>
#include <runner/raise.hh>

namespace object
{
  using boost::format;

  /*------------.
  | C++ methods |
  `------------*/

  const Path::value_type& Path::value_get() const
  {
    return path_;
  }


  /*-------------.
  | Urbi methods |
  `-------------*/

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
    proto_add(proto ? proto : object_class);
  }

  void Path::init(const std::string& path)
  {
    path_ = path;
  }

  // Global informations

  rPath Path::cwd()
  {
    return new Path(boost::filesystem::current_path().string());
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

  rPath Path::concat(rPath other)
  {
    return new Path(path_ / (*other).path_);
  }

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
    runner::raise_primitive_error(str(format("Unsupported file type: %s.") %
				      path_));
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
    runner::raise_primitive_error(str(format("%1%: %2%") %
				      strerror(errno) % path_));
  }

  /*---------------.
  | Binding system |
  `---------------*/

  void Path::initialize(CxxObject::Binder<Path>& bind)
  {
    bind(SYMBOL(absolute), &Path::absolute);
    bind(SYMBOL(asList), &Path::as_list);
    bind(SYMBOL(asPrintable), &Path::as_printable);
    bind(SYMBOL(asString), &Path::as_string);
    bind(SYMBOL(basename), &Path::basename);
    bind(SYMBOL(cd), &Path::cd);
    bind(SYMBOL(cwd), &Path::cwd);
    bind(SYMBOL(dirname), &Path::dirname);
    bind(SYMBOL(exists), &Path::exists);
    bind(SYMBOL(init), &Path::init);
    bind(SYMBOL(isDir), &Path::is_dir);
    bind(SYMBOL(isReg), &Path::is_reg);
    bind(SYMBOL(open), &Path::open);
    bind(SYMBOL(readable), &Path::readable);
    bind(SYMBOL(SLASH), &Path::concat);
    bind(SYMBOL(writable), &Path::writable);
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
