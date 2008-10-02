#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <object/directory.hh>
#include <object/path.hh>

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

  Directory::Directory(rDirectory)
    : path_(0)
  {
    throw PrimitiveError(SYMBOL(clone),
			 "`Directory' objects cannot be cloned");
  }

  Directory::Directory(const std::string& value)
    : path_(new Path(value))
  {
    proto_add(proto);
  }

  void Directory::init(rPath path)
  {
    if (!path->is_dir())
      throw PrimitiveError
        (SYMBOL(init),
         str(format("Not a directory: '%s'") % path->as_string()));
    path_ = path;
  }

  // Conversions

  std::string Directory::as_string()
  {
    return path_->as_string();
  }

  std::string Directory::as_printable()
  {
    return "Directory " + path_->as_printable();
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
      res << F(*this, it->path().string());

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


  /*---------------.
  | Binding system |
  `---------------*/

  void Directory::initialize(CxxObject::Binder<Directory>& bind)
  {
    bind(SYMBOL(asPrintable), &Directory::as_printable);
    bind(SYMBOL(asString), &Directory::as_string);
    bind(SYMBOL(content), &Directory::list<&details::mk_path>);
    bind(SYMBOL(init), &Directory::init);
    bind(SYMBOL(list), &Directory::list<&details::mk_string>);
  }

  std::string Directory::type_name_get() const
  {
    return type_name;
  }

  bool Directory::directory_added = CxxObject::add<Directory>();
  const std::string Directory::type_name = "Directory";
  rObject Directory::proto;

}
