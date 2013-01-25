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
 ** \file ast/local.hxx
 ** \brief Inline methods of ast::Local.
 */

#ifndef AST_LOCAL_HXX
# define AST_LOCAL_HXX

# include <ast/local.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Local::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< libport::Symbol >("name", name_);
    ser.template serialize< exps_type* >("arguments", arguments_);
    ser.template serialize< unsigned >("depth", depth_);
    ser.template serialize< rLocalDeclaration >("declaration", declaration_);
  }

  template <typename T>
  Local::Local(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    name_ = ser.template unserialize< libport::Symbol >("name");
    arguments_ = ser.template unserialize< exps_type* >("arguments");
    depth_ = ser.template unserialize< unsigned >("depth");
    declaration_ = ser.template unserialize< rLocalDeclaration >("declaration");
  }
#endif

  inline const libport::Symbol&
  Local::name_get () const
  {
    return name_;
  }
  inline libport::Symbol&
  Local::name_get ()
  {
    return name_;
  }

  inline const exps_type*
  Local::arguments_get () const
  {
    return arguments_;
  }
  inline exps_type*
  Local::arguments_get ()
  {
    return arguments_;
  }
  inline void
  Local::arguments_set (exps_type* arguments)
  {
    delete arguments_;
    arguments_ = arguments;
  }

  inline const unsigned&
  Local::depth_get () const
  {
    return depth_;
  }
  inline unsigned&
  Local::depth_get ()
  {
    return depth_;
  }
  inline void
  Local::depth_set (unsigned depth)
  {
    depth_ = depth;
  }

  inline const rLocalDeclaration&
  Local::declaration_get () const
  {
    return declaration_;
  }
  inline rLocalDeclaration&
  Local::declaration_get ()
  {
    return declaration_;
  }
  inline void
  Local::declaration_set (const rLocalDeclaration& declaration)
  {
    declaration_ = declaration;
  }


} // namespace ast

#endif // !AST_LOCAL_HXX
