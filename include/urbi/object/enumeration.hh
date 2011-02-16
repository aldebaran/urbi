/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_ENUMERATION_HH
# define URBI_OBJECT_ENUMERATION_HH

# include <boost/preprocessor/seq/for_each.hpp>

# include <libport/preproc.hh>
# include <libport/symbol.hh>

# include <urbi/export.hh>
# include <urbi/object/global.hh>

# define URBI_ENUM_PUSH(R, D, Elt)                      \
  values << BOOST_PP_STRINGIZE(LIBPORT_SECOND(Elt));

# define URBI_ENUM_DECLARE(Name)                                        \
                                                                        \
  namespace urbi                                                        \
  {                                                                     \
    namespace object                                                    \
    {                                                                   \
      template<>                                                        \
        struct CxxConvert< Name>                                        \
      {                                                                 \
        typedef Name target_type;                                       \
                                                                        \
        static boost::unordered_map< ::urbi::object::rObject, Name>&    \
          urbi_enum_u2c()                                               \
        {                                                               \
          static boost::unordered_map< ::urbi::object::rObject, Name >  \
            res;                                                        \
          return res;                                                   \
        }                                                               \
                                                                        \
        static boost::unordered_map< Name, ::urbi::object::rObject>&    \
          urbi_enum_c2u()                                               \
        {                                                               \
          static boost::unordered_map< Name, ::urbi::object::rObject>   \
            res;                                                        \
          return res;                                                   \
        }                                                               \
                                                                        \
        static rObject&                                                 \
          urbi_enum()                                                   \
        {                                                               \
          static ::urbi::object::rObject res;                           \
          return res;                                                   \
        }                                                               \
                                                                        \
        static target_type                                              \
          to(const rObject& o)                                          \
        {                                                               \
          type_check(o, urbi_enum());                                   \
          return urbi_enum_u2c()[o];                                    \
        }                                                               \
                                                                        \
        static rObject                                                  \
          from(target_type v)                                           \
        {                                                               \
          return urbi_enum_c2u()[v];                                    \
        }                                                               \
      };                                                                \
    }                                                                   \
  }

# define URBI_ENUM_REG(R, Name, Elt)                                    \
  ::urbi::object::CxxConvert< Name >::urbi_enum_u2c()                   \
  [e->getSlot(BOOST_PP_STRINGIZE(LIBPORT_SECOND(Elt)))] =               \
            LIBPORT_FIRST(Elt);                                         \
  ::urbi::object::CxxConvert< Name >::urbi_enum_c2u()                   \
  [LIBPORT_FIRST(Elt)] =                                                \
            e->getSlot(BOOST_PP_STRINGIZE(LIBPORT_SECOND(Elt)));

# define URBI_ENUM_REGISTER(Name, UName, ...)                   \
                                                                \
  static void                                                   \
  BOOST_PP_CAT(urbi_enum_create, __LINE__)()                    \
  {                                                             \
    std::string name = #UName;                                  \
    ::urbi::object::rObject dest =                              \
        ::urbi::object::resolve_namespace(name);                \
    std::vector<std::string> values;                            \
    BOOST_PP_SEQ_FOR_EACH(URBI_ENUM_PUSH, _,                    \
                          LIBPORT_LIST(__VA_ARGS__,));          \
    CAPTURE_GLOBAL(Enumeration);                                \
    ::urbi::object::rObject e = Enumeration->call               \
        ("new",                                                 \
         ::urbi::object::to_urbi(name),                         \
         ::urbi::object::to_urbi(values));                      \
    ::urbi::object::CxxConvert< Name >::urbi_enum() = e;        \
    dest->setSlot(name, e);                                     \
    BOOST_PP_SEQ_FOR_EACH(URBI_ENUM_REG, Name,                  \
                          LIBPORT_LIST(__VA_ARGS__,));          \
  }                                                             \
                                                                \
  URBI_INITIALIZATION_REGISTER(&BOOST_PP_CAT(urbi_enum_create,  \
                                             __LINE__))


#endif
