/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
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
    class URBI_SDK_API Directory: public CxxObject
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
      Directory(rPath path);

      void init(rPath);
      void init(const std::string&);
      static rObject create(rObject, rPath);
      static rObject create_all(rObject, rPath);

      // Modifiers
      rPath as_path() const;
      void clear();
      bool empty() const;
      bool exists() const;
      rString basename() const;
      rDate last_modified_date() const;
      rDirectory parent() const;
      void remove();
      void remove_all();
      rDirectory rename(const std::string&);

    /*---------------------.
    | Global information.  |
    `---------------------*/

    public:
      static rDirectory current();

    private:
      static void create_all_recursive(rPath path);
      static rDirectory create_directory(rPath path);
      void create_events();
      static rDirectory instanciate_directory(rPath path);
      static rDirectory instanciate_directory(const std::string&);
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

      URBI_CXX_OBJECT(Directory, CxxObject);
    };

    rObject
    directory_mk_string(Directory&, const std::string& entry);
    rObject
    directory_mk_path(Directory& d, const std::string& entry);
  }
}

#endif
