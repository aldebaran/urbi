#include <stdlib.h>

#include <boost/format.hpp>

#include <fstream>

#include <object/file.hh>
#include <object/path.hh>

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

  File::File(rFile)
    : path_(0)
  {
    throw PrimitiveError(SYMBOL(clone),
			 "`File' objects cannot be cloned");
  }

  File::File(const std::string& value)
    : path_(new Path(value))
  {
    proto_add(proto);
  }

  void File::init(rPath path)
  {
    if (!path->is_reg())
      throw PrimitiveError
        (SYMBOL(init),
         str(format("Not a regular file: '%s'") % path->as_string()));
    path_ = path;
  }

  void File::init(const std::string& path)
  {
    init(new Path(path));
  }

  // Conversions

  rList File::as_list()
  {
    std::ifstream s(path_->as_string().c_str());
    List::value_type res;

    char buf[BUFSIZ + 1];
    std::string line;

    // Split in lines, 'Back in the stone age' version.
    while (!s.eof())
    {
      s.read(buf, sizeof buf - 1);
      buf[s.gcount()] = 0;
      char* str = buf;
      while (char* cut = strstr(str, "\n"))
      {
        *cut = 0;
        res << new String(line + str);
        line = "";
        str = cut + 1;
      }
      line += str;
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
    return "File " + path_->as_printable();
  }


  /*--------.
  | Details |
  `--------*/

#define OVERLOAD_TYPE(Name, Msg, T1, V1, T2, V2)                        \
                                                                        \
  static rObject Name(runner::Runner& r, object::objects_type args)     \
  {                                                                     \
    static rPrimitive v1 = make_primitive(V1, SYMBOL(Msg));             \
    static rPrimitive v2 = make_primitive(V2, SYMBOL(Msg));             \
                                                                        \
    object::check_arg_count (2, args.size(), SYMBOL(Msg));              \
    if (args[1]->is_a<T2>())                                            \
      return (*v2)(r, args);                                            \
    else                                                                \
      return (*v1)(r, args);                                            \
  }                                                                     \

  OVERLOAD_TYPE(init_bouncer, init,
                Path,
                (void (File::*)(rPath)) &File::init,
                String,
                (void (File::*)(const std::string&)) &File::init);


  /*---------------.
  | Binding system |
  `---------------*/

  void File::initialize(CxxObject::Binder<File>& bind)
  {
    bind(SYMBOL(asList), &File::as_list);
    bind(SYMBOL(asPrintable), &File::as_printable);
    bind(SYMBOL(asString), &File::as_string);

    proto->slot_set(SYMBOL(init), new Primitive(&init_bouncer));
  }

  std::string File::type_name_get() const
  {
    return type_name;
  }

  bool File::file_added = CxxObject::add<File>("File", File::proto);
  const std::string File::type_name = "File";
  rObject File::proto;

}
