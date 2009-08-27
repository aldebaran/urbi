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
    rList as_list() const;
    std::string as_string() const;
    std::string as_printable() const;

    /// The contents of the file.  Might not be a text file, hence it
    /// returns an instance of Binary, not a std::string.
    rObject content() const;


  /*--------.
  | Details |
  `--------*/

  private:
    value_type path_;

  URBI_CXX_OBJECT(File);
  };
}

#endif
