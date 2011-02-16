/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_FILE_HH
# define OBJECT_FILE_HH

# include <libport/path.hh>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API File: public CxxObject
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

      rString basename() const;
      static rFile createTemp(rObject);
      rDate last_modified_date() const;
      void remove();
      rFile rename(const std::string& dst);
      rFloat size() const;

      // Conversions
      rList as_list() const;
      rPath as_path() const;
      virtual std::string as_string() const;
      std::string as_printable() const;

      /// The contents of the file.  Might not be a text file, hence it
      /// returns an instance of Binary, not a std::string.
      rObject content() const;


    /*--------.
    | Details |
    `--------*/

    private:
      value_type path_;

      URBI_CXX_OBJECT(File, CxxObject);
    };
  }
}

#endif
