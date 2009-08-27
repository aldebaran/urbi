#include <stdlib.h>

#include <boost/format.hpp>

#include <fstream>

#include <object/file.hh>
#include <object/global.hh>
#include <object/path.hh>
#include <object/symbols.hh>

#include <runner/raise.hh>

namespace object
{
  using boost::format;

  /*------------.
  | C++ methods |
  `------------*/

  File::value_type File::value_get()
  {
    return path_;
  }


  /*-------------.
  | Urbi methods |
  `-------------*/

  // Construction

  File::File()
    : path_(new Path("/"))
  {
    proto_add(proto);
  }

  File::File(rFile model)
    : path_(model.get()->path_)
  {
    proto_add(model);
  }

  File::File(const std::string& value)
    : path_(new Path(value))
  {
    proto_add(proto ? proto : Object::proto);
  }

  rFile File::create(rObject, const std::string& p)
  {
    libport::path path(p);
    path.create();
    return new File(p);
  }

  void File::init(rPath path)
  {
    if (!path->is_reg())
      FRAISE("Not a regular file: %s", path->as_string());
    path_ = path;
  }

  void File::init(const std::string& path)
  {
    init(new Path(path));
  }

  // Conversions

  static bool
  split_point(const std::string& str, size_t& pos, size_t& size)
  {
    pos = std::string::npos;
    size_t where;

#define SPLIT(Str)                                      \
    where = str.find(Str);                              \
    if (where != std::string::npos && where < pos)      \
    {                                                   \
      pos = where;                                      \
      size = sizeof(Str) - 1;                           \
    }
    SPLIT("\n");
    SPLIT("\r\n");
#undef SPLIT
    return pos != std::string::npos;
  }

  static void
  get_buf(std::istream& input, std::string& output)
  {
    char buf[BUFSIZ + 1];

    input.read(buf, sizeof buf - 1);
    output += std::string(buf, input.gcount());
  }

  rList File::as_list() const
  {
    std::ifstream s(path_->as_string().c_str());
    if (!s.good())
      FRAISE("File not readable: %s", as_string());

    List::value_type res;
    std::string line;

    // Split in lines, 'Back in the stone age' version.
    while (!s.eof())
    {
      get_buf(s, line);
      // Initialized because g++ warns that size may be used
      // uninitialized otherwise
      size_t pos = std::string::npos;
      size_t size = 0;
      while (split_point(line, pos, size))
      {
        res << new String(line.substr(0, pos));
        line = line.substr(pos + size, std::string::npos);
      }
    }

    // Bad bad bad user! The file does not finish with a \n! Handle it
    // anyway ...
    if (!line.empty())
      res << new String(line);

    return new List(res);
  }

  std::string File::as_string() const
  {
    return path_->as_string();
  }

  std::string File::as_printable() const
  {
    return libport::format("File(\"%s\")", path_->as_string());
  }

  rObject File::content() const
  {
    std::ifstream s(path_->as_string().c_str());
    if (!s.good())
      RAISE("File not readable: " + as_string());

    std::string res;
    while (!s.eof())
      get_buf(s, res);

    CAPTURE_GLOBAL(Binary);
    return Binary->call(SYMBOL(new), to_urbi(std::string()), to_urbi(res));
  }

  /*--------.
  | Details |
  `--------*/

  OVERLOAD_TYPE(init_bouncer, 1, 1,
                Path,
                (void (File::*)(rPath)) &File::init,
                String,
                (void (File::*)(const std::string&)) &File::init);


  /*---------------.
  | Binding system |
  `---------------*/

  void
  File::initialize(CxxObject::Binder<File>& bind)
  {
    bind(SYMBOL(asList), &File::as_list);
    bind(SYMBOL(asPrintable), &File::as_printable);
    bind(SYMBOL(asString), &File::as_string);
    bind(SYMBOL(content), &File::content);
    bind(SYMBOL(create), &File::create);

    proto->slot_set(SYMBOL(init), new Primitive(&init_bouncer));
  }

  rObject
  File::proto_make()
  {
    return new File("/");
  }

  URBI_CXX_OBJECT_REGISTER(File);
}
