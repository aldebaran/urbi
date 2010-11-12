/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_ENUMERATION_HH
# define URBI_OBJECT_ENUMERATION_HH

# include <libport/preproc.hh>
# include <libport/symbol.hh>

# include <urbi/export.hh>
# include <urbi/object/global.hh>

# define URBI_ENUM_PUSH(Elt)                            \
  values << BOOST_PP_STRINGIZE(LIBPORT_SECOND(Elt));

# define URBI_ENUM_REG(Elt)                                             \
  LIBPORT_CAT(_urbi_enum_u2c, __LINE__)()                               \
  [e->getSlot(BOOST_PP_STRINGIZE(LIBPORT_SECOND(Elt)))] =               \
    LIBPORT_FIRST(Elt);                                                 \
  LIBPORT_CAT(_urbi_enum_c2u, __LINE__)()[LIBPORT_FIRST(Elt)] =         \
    e->getSlot(BOOST_PP_STRINGIZE(LIBPORT_SECOND(Elt)));                \

# define URBI_ENUM_BIND(Name, UName, ...)                               \
                                                                        \
  static boost::unordered_map< ::urbi::object::rObject, Name>&          \
  LIBPORT_CAT(_urbi_enum_u2c, __LINE__)()                               \
  {                                                                     \
    static boost::unordered_map< ::urbi::object::rObject, Name> res;    \
    return res;                                                         \
  }                                                                     \
                                                                        \
  static boost::unordered_map<Name, ::urbi::object::rObject>&           \
  LIBPORT_CAT(_urbi_enum_c2u, __LINE__)()                               \
  {                                                                     \
    static boost::unordered_map<Name, ::urbi::object::rObject> res;     \
    return res;                                                         \
  }                                                                     \
                                                                        \
  ::urbi::object::rObject&                                              \
  LIBPORT_CAT(_urbi_enum_enum, __LINE__)()                              \
  {                                                                     \
    static ::urbi::object::rObject res;                                 \
    return res;                                                         \
  }                                                                     \
                                                                        \
  static void LIBPORT_CAT(_urbi_enum_create, __LINE__)()                \
  {                                                                     \
    std::string name = #UName;                                          \
    ::urbi::object::rObject dest =                                      \
        ::urbi::object::resolve_namespace(name);                        \
    std::vector<std::string> values;                                    \
    LIBPORT_LIST_FLATTEN                                                \
      (LIBPORT_LIST_MAP(URBI_ENUM_PUSH, LIBPORT_LIST(__VA_ARGS__,)));   \
    CAPTURE_GLOBAL(Enumeration);                                        \
    ::urbi::object::rObject e = Enumeration->call                       \
        ("new",                                                         \
         ::urbi::object::to_urbi(name),                                 \
         ::urbi::object::to_urbi(values));                              \
    LIBPORT_CAT(_urbi_enum_enum, __LINE__)() = e;                       \
    dest->setSlot(name, e);                                             \
    LIBPORT_LIST_FLATTEN                                                \
      (LIBPORT_LIST_MAP(URBI_ENUM_REG, LIBPORT_LIST(__VA_ARGS__,)));    \
  }                                                                     \
  URBI_INITIALIZATION_REGISTER                                          \
  (&LIBPORT_CAT(_urbi_enum_create, __LINE__));                          \
                                                                        \
  namespace urbi                                                        \
  {                                                                     \
    namespace object                                                    \
    {                                                                   \
      template<>                                                        \
        struct CxxConvert<Name>                                         \
      {                                                                 \
        typedef Name target_type;                                       \
        static target_type                                              \
          to(const rObject& o)                                          \
        {                                                               \
          type_check(o, LIBPORT_CAT(_urbi_enum_enum, __LINE__)());      \
          return LIBPORT_CAT(_urbi_enum_u2c, __LINE__)()[o];            \
        }                                                               \
                                                                        \
        static rObject                                                  \
          from(target_type v)                                           \
        {                                                               \
          return LIBPORT_CAT(_urbi_enum_c2u, __LINE__)()[v];            \
        }                                                               \
      };                                                                \
    }                                                                   \
  }                                                                     \

#endif
