/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_DIRECTORY_HH
# define OBJECT_DIRECTORY_HH

# include <libport/path.hh>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class Directory: public CxxObject
    {

    /*--------------.
    | C++ methods.  |
    `--------------*/

    public:

      typedef rPath value_type;
      value_type value_get();

    /*---------------------.
    | urbiscript methods.  |
    `---------------------*/

    public:

      // Construction
      Directory();
      Directory(rDirectory model);
      Directory(const std::string& path);
      void init(rPath path);
      void init(const std::string& path);

  //     // Global informations
  //     static rDirectory cwd();

  //     // Operations
  //     rDirectory concat(rDirectory other);
  //     std::string basename();
  //     std::string dirname();

    private:
      void create_events();
      rObject on_file_created_;
      rObject on_file_deleted_;

    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      virtual std::string as_string() const;
      std::string as_printable() const;

      // Stat.
      template <rObject (*F) (Directory& d, const std::string& entry)>
      rList list();

    /*--------.
    | Details |
    `--------*/

    private:
      value_type path_;

    URBI_CXX_OBJECT_(Directory);
    };
  }
}

#endif
