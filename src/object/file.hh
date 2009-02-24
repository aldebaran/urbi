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
    static rFile create(rObject, const std::string& path);
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

  URBI_CXX_OBJECT(File);
  };
}

#endif
