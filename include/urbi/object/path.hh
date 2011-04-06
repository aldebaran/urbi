/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_PATH_HH
# define OBJECT_PATH_HH

# include <libport/compiler.hh>
# include <libport/path.hh>

# include <urbi/object/cxx-object.hh>
# include <urbi/object/directory.hh>
# include <urbi/object/string.hh>
# include <urbi/object/equality-comparable.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Path
      : public CxxObject
      , public EqualityComparable<Path, libport::path>
    {

    /*--------------.
    | C++ methods.  |
    `--------------*/

    public:
      typedef Path self_type;
      typedef libport::path value_type;
      const value_type& value_get() const;
      void value_set(const value_type&);

    /*---------------.
    | Urbi methods.  |
    `---------------*/

    public:

      // Construction.
      Path();
      Path(rPath model);
      Path(const std::string& value);
      void init(const std::string& path);

      // Comparison.
      bool operator<=(const rPath& rhs) const;

      // Global informations.
      static rPath cwd();

      // Operations.
      std::string basename() const;
      rPath cd() const;
      rPath dirname() const;
      rObject open() const;
      rDate last_modified_date() const;
      rPath path_concat(rPath other) const;
      rPath parent() const;
      rPath string_concat(rString other) const;

      // Stat.
      bool absolute() const;
      bool exists() const;
      bool is_dir() const;
      bool is_reg() const;
      bool is_root() const;
      bool readable() const;
      bool writable() const;

      // Conversions.
      rList as_list() const;
      virtual std::string as_string() const;
      std::string as_printable() const;

    /*----------.
    | Details.  |
    `----------*/

    private:
      value_type path_;

      ATTRIBUTE_NORETURN
      void handle_any_error() const;

      friend class Directory;

      // Stat the file and handle all errors
      struct stat stat() const;

      URBI_CXX_OBJECT(Path, CxxObject);
    };


    /*-----------------------------.
    | Conversions: libport::path.  |
    `-----------------------------*/

    template <>
    struct CxxConvert<libport::path>
    {
      typedef libport::path target_type;
      static target_type
      to(const rObject& o)
      {
        if (rString str = o->as<String>())
          return str->value_get();
        type_check<Path>(o);
        return o->as<String>()->value_get();
      }

      static rObject
      from(const target_type& v)
      {
        return new Path(v);
      }
    };

    /*---------------------.
    | Conversions: rPath.  |
    `---------------------*/

    template<>
    struct CxxConvert<rPath>
    {
      typedef rPath target_type;
      typedef rPath source_type;

      static target_type
      to(const rObject& o)
      {
        if(rPath path = o->as<Path>())
          return path;
        if (rString str = o->as<String>())
          return new Path(str->value_get());

        runner::raise_type_error(o, Path::proto);
      }

      static rObject
      from(source_type v)
      {
        return v;
      }
    };

  }
}

#endif
