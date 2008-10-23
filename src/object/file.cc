#include <stdlib.h>

#include <boost/format.hpp>

#include <fstream>

#include <object/file.hh>
#include <object/path.hh>
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
    proto_add(proto ? proto : object_class);
  }

  void File::init(rPath path)
  {
    if (!path->is_reg())
      runner::raise_primitive_error(str(format("Not a regular file: '%s'") %
					path->as_string()));
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
    pos = str.find("\n");
    size = 1;
#ifdef WIN32
    size_t rn = str.find("\r\n");
    if (rn != std::string::npos && rn < pos)
    {
      pos = rn;
      size = 2;
    }
#endif
    return pos != std::string::npos;
  }

  static void
  get_buf(std::istream& input, std::string& output)
  {
    char buf[BUFSIZ + 1];

    input.read(buf, sizeof buf - 1);
    buf[input.gcount()] = 0;
    output += buf;
  }

  rList File::as_list()
  {
    std::ifstream s(path_->as_string().c_str());
    if (!s.good())
      runner::raise_primitive_error("File not readable: " + as_string());

    List::value_type res;
    std::string line;

    // Split in lines, 'Back in the stone age' version.
    while (!s.eof())
    {
      get_buf(s, line);
      size_t pos;
      size_t size;
      while (split_point(line, pos, size))
      {
        res << new String(line.substr(0, pos));
        line = line.substr(pos + size, std::string::npos);
      }
    }

    // Bad bad bad user! The file does not finish with a \n! Handle it
    // anyway ...
    if (line != "")
      res << new String(line);

    return new List(res);
  }

  std::string File::as_string()
  {
    return path_->as_string();
  }

  std::string File::as_printable()
  {
    return (boost::format("File(\"%s\")") % path_->as_string()).str();
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

    proto->slot_set(SYMBOL(init), new Primitive(&init_bouncer));
  }

  rObject
  File::proto_make()
  {
    return new File("/");
  }

  URBI_CXX_OBJECT_REGISTER(File);
}
