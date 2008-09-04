#ifndef OBJECT_PATH_HH
# define OBJECT_PATH_HH

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

    void handle_any_error(const libport::Symbol& msg)  __attribute__ ((noreturn));
    // Handle EBADF, EFAULT, ELOOP, ENAMETOOLONG, ENOMEM
    void handle_hard_error(const libport::Symbol& msg);
    // Handle EACCES, ENOENT, ENOTDIR
    void handle_access_error(const libport::Symbol& msg);
    // Handle EEXIST
    void handle_perm_error(const libport::Symbol& msg);
    // Signal an unhandled errno value
    void unhandled_error(const libport::Symbol& msg) __attribute__ ((noreturn));
    // Stat the file and handle all errors
    struct stat stat(const libport::Symbol& msg);

  /*---------------.
  | Binding system |
  `---------------*/

  public:
    static void initialize(CxxObject::Binder<Path>& binder);
    static bool path_added;
    static const std::string type_name;
    virtual std::string type_name_get() const;
    static rObject proto;
  };
}

#endif
