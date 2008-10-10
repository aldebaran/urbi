#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <libport/detect-win32.h>

#include <object/directory.hh>
#include <object/path.hh>
#include <runner/raise.hh>

namespace object
{
  using boost::format;

  /*------------.
  | C++ methods |
  `------------*/

  Directory::value_type Directory::value_get()
  {
    return path_;
  }


  /*-------------.
  | Urbi methods |
  `-------------*/

  // Construction

  Directory::Directory()
    : path_(new Path("/"))
  {
    proto_add(proto);
  }

  Directory::Directory(rDirectory model)
    : path_(model.get()->path_)
  {
    proto_add(model);
  }

  Directory::Directory(const std::string& value)
    : path_(new Path(value))
  {
    proto_add(proto ? proto : object_class);
  }

  void Directory::init(rPath path)
  {
    if (!path->is_dir())
      runner::raise_primitive_error(str(format("Not a directory: '%s'") %
					path->as_string()));
    path_ = path;
  }

  void Directory::init(const std::string& path)
  {
    init(new Path(path));
  }

  // Conversions

  std::string Directory::as_string()
  {
    return path_->as_string();
  }

  std::string Directory::as_printable()
  {
    return (boost::format("Directory(\"%s\")") % path_->as_string()).str();
  }

  // Stat
  template <rObject (*F) (Directory& d, const std::string& entry)>
  rList Directory::list()
  {
    using boost::filesystem::directory_iterator;
    List::value_type res;

    directory_iterator end;
    for (directory_iterator it (path_->value_get().to_string());
         it != end;
         ++it)
      res << F(*this, basename(it->path()));

    return new List(res);
  }


  /*--------.
  | Details |
  `--------*/

  namespace details
  {
    rObject mk_string(Directory&, const std::string&);
    rObject mk_path(Directory&, const std::string&);

    rObject
    mk_string(Directory&, const std::string& entry)
    {
      return new String(entry);
    }

    rObject
    mk_path(Directory& d, const std::string& entry)
    {
      return new Path(d.value_get()->value_get() / entry);
    }
  }

  OVERLOAD_TYPE(init_bouncer, init,
                Path,
                (void (Directory::*)(rPath)) &Directory::init,
                String,
                (void (Directory::*)(const std::string&)) &Directory::init);

  /*---------------.
  | Binding system |
  `---------------*/

  void
  Directory::initialize(CxxObject::Binder<Directory>& bind)
  {
    bind(SYMBOL(asPrintable), &Directory::as_printable);
    bind(SYMBOL(asString), &Directory::as_string);
    bind(SYMBOL(content), &Directory::list<&details::mk_path>);
    bind(SYMBOL(list), &Directory::list<&details::mk_string>);

    proto->slot_set(SYMBOL(init), new Primitive(&init_bouncer));
  }

  rObject
  Directory::proto_make()
  {
#ifdef WIN32
    return new Directory("C:\\");
#else
    return new Directory("/");
#endif
  }

  URBI_CXX_OBJECT_REGISTER(Directory);
}
