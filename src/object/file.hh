#ifndef OBJECT_FILE_HH
# define OBJECT_FILE_HH

# include <libport/path.hh>

# include <object/cxx-object.hh>

namespace object
{
  class File: public CxxObject
  {

  /*------------.
  | C++ methods |
  `------------*/

  public:

    typedef rPath value_type;
    value_type value_get();


  /*-------------.
  | Urbi methods |
  `-------------*/

  public:

    // Construction
    File();
    File(rFile model);
    File(const std::string& path);
    void init(rPath path);
    void init(const std::string& path);

    // Conversions
    rList as_list();
    std::string as_string();
    std::string as_printable();


  /*--------.
  | Details |
  `--------*/

  private:
    value_type path_;


  /*---------------.
  | Binding system |
  `---------------*/

  public:
    static void initialize(CxxObject::Binder<File>& binder);
    static bool file_added;
    static const std::string type_name;
    virtual std::string type_name_get() const;
    static rObject proto;

  private:
    friend class TypeInitializer<File>;
    static rObject proto_make();
  };
}

#endif
