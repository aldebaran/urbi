/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// Generated, do not edit by hand.

/**
 ** \file ast/local-declaration.hxx
 ** \brief Inline methods of ast::LocalDeclaration.
 */

#ifndef AST_LOCAL_DECLARATION_HXX
# define AST_LOCAL_DECLARATION_HXX

# include <ast/local-declaration.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  LocalDeclaration::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    LocalWrite::serialize(ser);
    ser.template serialize< bool >("constant", constant_);
    ser.template serialize< bool >("list", list_);
    ser.template serialize< bool >("is_import", is_import_);
    ser.template serialize< bool >("is_star", is_star_);
    ser.template serialize< rExp >("type", type_);
  }

  template <typename T>
  LocalDeclaration::LocalDeclaration(libport::serialize::ISerializer<T>& ser)
    : LocalWrite(ser)
  {
    LIBPORT_USE(ser);
    constant_ = ser.template unserialize< bool >("constant");
    list_ = ser.template unserialize< bool >("list");
    is_import_ = ser.template unserialize< bool >("is_import");
    is_star_ = ser.template unserialize< bool >("is_star");
    type_ = ser.template unserialize< rExp >("type");
  }
#endif

  inline const bool&
  LocalDeclaration::constant_get () const
  {
    return constant_;
  }
  inline void
  LocalDeclaration::constant_set (bool constant)
  {
    constant_ = constant;
  }

  inline const bool&
  LocalDeclaration::list_get () const
  {
    return list_;
  }
  inline void
  LocalDeclaration::list_set (bool list)
  {
    list_ = list;
  }

  inline const bool&
  LocalDeclaration::is_import_get () const
  {
    return is_import_;
  }
  inline void
  LocalDeclaration::is_import_set (bool is_import)
  {
    is_import_ = is_import;
  }

  inline const bool&
  LocalDeclaration::is_star_get () const
  {
    return is_star_;
  }
  inline void
  LocalDeclaration::is_star_set (bool is_star)
  {
    is_star_ = is_star;
  }

  inline const rExp&
  LocalDeclaration::type_get () const
  {
    return type_;
  }
  inline rExp&
  LocalDeclaration::type_get ()
  {
    return type_;
  }
  inline void
  LocalDeclaration::type_set (const rExp& type)
  {
    type_ = type;
  }


} // namespace ast

#endif // !AST_LOCAL_DECLARATION_HXX
