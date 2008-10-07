#ifndef OBJECT_PATH_HH
# define OBJECT_PATH_HH

# include <libport/compiler.hh>
# include <libport/path.hh>

# include <object/cxx-object.hh>

namespace object
{
  class Path: public CxxObject
  {

  /*------------.
  | C++ methods |
  `------------*/

  public:

    typedef libport::path value_type;
    const value_type& value_get() const;

  /*-------------.
  | Urbi methods |
  `-------------*/

  public:

    // Construction
    Path();
    Path(rPath model);
    Path(const std::string& value);
    void init(const std::string& path);

    // Global informations
    static rPath cwd();

    // Operations
    std::string basename();
    rPath cd();
    rPath concat(rPath other);
    std::string dirname();
    rObject open();

    // Stat
    bool absolute();
    bool exists();
    bool is_dir();
    bool is_reg();
    bool readable();
    bool writable();

    // Conversions
    rList as_list();
    std::string as_string();
    std::string as_printable();

  /*--------.
  | Details |
  `--------*/

  private:
    value_type path_;

    ATTRIBUTE_NORETURN
    void handle_any_error();
    // Stat the file and handle all errors
    struct stat stat();

    URBI_CXX_OBJECT(Path);
  };
}

#endif
