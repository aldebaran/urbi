#ifndef OBJECT_DIRECTORY_HH
# define OBJECT_DIRECTORY_HH

# include <libport/path.hh>

# include <object/cxx-object.hh>

namespace object
{
  class Directory: public CxxObject
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
    Directory();
    Directory(rDirectory model);
    Directory(const std::string& path);
    void init(rPath path);

//     // Global informations
//     static rDirectory cwd();

//     // Operations
//     rDirectory concat(rDirectory other);
//     std::string basename();
//     std::string dirname();

    // Conversions
    std::string as_string();
    std::string as_printable();

    // Stat
    template <rObject (*F) (Directory& d, const std::string& entry)>
    rList list();

  /*--------.
  | Details |
  `--------*/

  private:
    value_type path_;

  URBI_CXX_OBJECT(Directory);
  };
}

#endif
