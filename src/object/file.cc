/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cstdlib>
#include <fstream>

#include <libport/file-system.hh>

#include <urbi/object/file.hh>
#include <urbi/object/global.hh>
#include <urbi/object/path.hh>
#include <object/symbols.hh>

#include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {
    using boost::format;

    /*--------------.
    | C++ methods.  |
    `--------------*/

    File::value_type File::value_get()
    {
      return path_;
    }


    /*---------------------.
    | urbiscript methods.  |
    `---------------------*/

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
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    rFile File::create(rObject, const std::string& p)
    {
      libport::path path(p);
      if (path.exists())
        path.remove();
      if (!path.create())
        FRAISE("cannot create file: %s: %s", path, strerror(errno));
      return new File(p);
    }

    void File::rename(const std::string& dst)
    {
      libport::path path = path_->value_get();
      path.rename(dst);
      path_->value_set(path);
    }

    void File::remove()
    {
      path_->value_get().remove();
    }

    void File::init(rPath path)
    {
      if (!path->exists())
        FRAISE("does not exist: %s", *path);
      if (path->is_dir())
        FRAISE("is a directory: %s", *path);
      path_ = path;
    }

    void File::init(const std::string& path)
    {
      init(new Path(path));
    }

    /*--------------.
    | Conversions.  |
    `--------------*/

    static bool
    split_point(const std::string& str, size_t& pos, size_t& size)
    {
      pos = std::string::npos;
      size_t where;

#define SPLIT(Str)                                      \
      where = str.find(Str);                            \
      if (where != std::string::npos && where < pos)    \
      {                                                 \
        pos = where;                                    \
        size = sizeof(Str) - 1;                         \
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
        // uninitialized otherwise.
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
      CAPTURE_GLOBAL(Binary);
      return Binary->call(SYMBOL(new),
                          to_urbi(std::string()),
                          to_urbi(libport::file_content(path_->as_string())));
    }

    /*----------.
    | Details.  |
    `----------*/

    OVERLOAD_TYPE(init_bouncer, 1, 1,
                  Path,
                  (void (File::*)(rPath)) &File::init,
                  String,
                  (void (File::*)(const std::string&)) &File::init);


    /*----------.
    | Binding.  |
    `----------*/

    void
    File::initialize(CxxObject::Binder<File>& bind)
    {
      bind(SYMBOL(asList), &File::as_list);
      bind(SYMBOL(asPrintable), &File::as_printable);
      bind(SYMBOL(asString), &File::as_string);
      bind(SYMBOL(content), &File::content);
      bind(SYMBOL(create), &File::create);
      bind(SYMBOL(rename), &File::rename);
      bind(SYMBOL(remove), &File::remove);

      proto->slot_set(SYMBOL(init), new Primitive(&init_bouncer));
    }

    URBI_CXX_OBJECT_REGISTER(File)
      : path_(new Path("/"))
    {}
  }
}
