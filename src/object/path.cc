// For stat, getcwd
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// For bad_alloc
#include <exception>

#include <boost/format.hpp>

#include <object/directory.hh>
#include <object/path.hh>

namespace object
{

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

  Path::Path(rPath)
    : path_("/")
  {
    throw PrimitiveError(SYMBOL(clone),
			 "`Path' objects cannot be cloned");
  }

  Path::Path(const std::string& value)
    : path_(value)
  {
    proto_add(proto);
  }

  void Path::init(const std::string& path)
  {
    path_ = path;
  }

  // Global informations

  rPath Path::cwd()
  {
    // C *sucks*
    static char buf[PATH_MAX];
    char* res =  getcwd(buf, PATH_MAX);
    if (!res)
      throw PrimitiveError(SYMBOL(pwd),
                           "Current directory name too long.");
    return new Path(res);
  }

  // Stats

  bool Path::absolute()
  {
    return path_.absolute_get();
  }

  bool Path::exists()
  {
    struct stat dummy;

    if (!::stat(path_.to_string().c_str(), &dummy))
      return true;

    handle_hard_error(SYMBOL(exists));
    switch (errno)
    {
      case EACCES:
        // Permission denied on one of the parent directory.
      case ENOENT:
        // No such file or directory.
      case ENOTDIR:
        // One component isn't a directory
        return false;
      default:
        unhandled_error(SYMBOL(exists));
    }
  }

  struct stat Path::stat(const libport::Symbol& msg)
  {
    struct stat res;

    if (::stat(path_.to_string().c_str(), &res))
    {
      handle_hard_error(msg);
      handle_access_error(msg);
      handle_perm_error(msg);
      unhandled_error(msg);
    }

    return res;
  }

  bool Path::is_dir()
  {
    return stat(SYMBOL(isDir)).st_mode & S_IFDIR;
  }

  bool Path::is_reg()
  {
    return stat(SYMBOL(isReg)).st_mode & S_IFREG;
  }

  bool Path::readable()
  {
    int fd = ::open(path_.to_string().c_str(), O_RDONLY | O_LARGEFILE);

    if (fd != -1)
    {
      close(fd);
      return true;
    }
    else if (errno == EACCES)
      return false;

    handle_hard_error(SYMBOL(readable));
    handle_access_error(SYMBOL(readable));
    unhandled_error(SYMBOL(readable));

    // Not handled errors, because neither NOATIME or NONBLOCK is used:

    // EPERM: The O_NOATIME flag was specified, but the effective user
    // ID of the caller did not match the owner of the file and the
    // caller was not privileged (CAP_FOWNER).

    // EWOULDBLOCK: The O_NONBLOCK flag was specified, and an
    // incompatible lease was held on the file (see fcntl(2)).
  }

  bool Path::writable()
  {
    int fd = ::open(path_.to_string().c_str(),
                    O_WRONLY | O_LARGEFILE | O_APPEND);

    if (fd != -1)
    {
      close(fd);
      return true;
    }
    // EROFS = file is on a read only file system.
    else if (errno == EACCES || errno == EROFS)
      return false;

    handle_hard_error(SYMBOL(readable));
    handle_access_error(SYMBOL(readable));
    handle_perm_error(SYMBOL(readable));
    unhandled_error(SYMBOL(readable));

    // EPERM and EWOULDBLOCK not handled, see readable().
  }

  // Operations

  std::string Path::basename()
  {
    return path_.basename();
  }

  rPath Path::concat(rPath other)
  {
    return new Path(path_ / (*other).path_);
  }

  std::string Path::dirname()
  {
    return path_.dirname();
  }

  rObject Path::open()
  {
    if (is_dir())
      return new Directory(path_);
    throw PrimitiveError
      (SYMBOL(open),
       str(boost::format("Unsupported file type: '%s'.") % path_));
  }

  // Conversions

  std::string Path::as_string()
  {
    return path_.to_string();
  }

  std::string Path::as_printable()
  {
    return str(boost::format("\"%s\"") % as_string());
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

  void Path::handle_hard_error(const libport::Symbol& msg)
  {
    switch (errno)
    {
      case EBADF:
        // Theoretically impossible.
        throw PrimitiveError
          (msg,
           "Unhandled error: bad file descriptor");
      case EFAULT:
        // Theoretically impossible.
        throw PrimitiveError
          (msg,
           "Unhandled error: bad address");
      case EFBIG:
        // Theoretically impossible, since O_LARGEFILE is always used
        throw PrimitiveError
          (msg,
           str(boost::format("Unhandled error: file too big: %s.") % path_));
      case EINTR:
        // Theoretically impossible.
        throw PrimitiveError
          (msg,
           "Unhandled error: file opening interrupted by a signal");
      case ELOOP:
        throw PrimitiveError
          (msg,
           str(boost::format("Too many symbolic link: '%s'.") % path_));
      case EMFILE:
        throw PrimitiveError
          (msg,
           str(boost::format("The kernel has reached"
                             " its maximum opened file limit: '%s'.") % path_));
      case ENAMETOOLONG:
        throw PrimitiveError
          (msg,
           str(boost::format("File name too long: '%s'.") % path_));

      case ENFILE:
        throw PrimitiveError
          (msg,
           str(boost::format("The system has reached"
                             " its maximum opened file limit: '%s'.") % path_));
      case ENOMEM:
        // Out of memory.
        throw std::bad_alloc();
      case ENOSPC:
        throw PrimitiveError
          (msg,
           str(boost::format("No space left on device: '%s'.") % path_));
      default:
        // Nothing
        break;
    }
  }

  void Path::handle_access_error(const libport::Symbol& msg)
  {
    switch (errno)
    {
      case EACCES:
        throw PrimitiveError
          (msg, str(boost::format("Permission denied "
                                  "for a parent directory: '%s'.")
                    % path_));
      case ENOENT:
      case ENOTDIR:
        throw PrimitiveError
          (msg, str(boost::format("No such file or directory: '%s'.")
                    % path_));
      default:
        // Nothing
        break;
    }
  }

  void Path::handle_perm_error(const libport::Symbol& msg)
  {
    switch (errno)
    {
      case EEXIST:
        throw PrimitiveError
          (msg, str(boost::format("File already exists: '%s'.")
                    % path_));
      case EISDIR:
        throw PrimitiveError
          (msg, str(boost::format("File is a directory: '%s'.")
                    % path_));
      case EROFS:
        throw PrimitiveError
          (msg, str(boost::format("File is on a read only file-system: '%s'.")
                    % path_));
      case ETXTBSY:
        throw PrimitiveError
          (msg, str(boost::format("File is currently being executed: '%s'.")
                    % path_));
      case ENODEV:
      case ENXIO:
        throw PrimitiveError
          (msg, str(boost::format("File is an unopened FIFO "
                                  "or an orphaned device: '%s'.")
                    % path_));
      default:
        // Nothing
        break;
    }
  }

  void Path::unhandled_error(const libport::Symbol& msg)
  {
    throw PrimitiveError
      (msg,
       str(boost::format("Unhandled errno for stat(2): %i (%s).")
           % errno
           % strerror(errno)));
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

  std::string Path::type_name_get() const
  {
    return type_name;
  }

  bool Path::path_added = CxxObject::add<Path>("Path", Path::proto);
  const std::string Path::type_name = "Path";
  rObject Path::proto;

}
