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
 ** \file ast/local-assignment.hxx
 ** \brief Inline methods of ast::LocalAssignment.
 */

#ifndef AST_LOCAL_ASSIGNMENT_HXX
# define AST_LOCAL_ASSIGNMENT_HXX

# include <ast/local-assignment.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  LocalAssignment::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    LocalWrite::serialize(ser);
    ser.template serialize< unsigned >("depth", depth_);
    ser.template serialize< rLocalDeclaration >("declaration", declaration_);
  }

  template <typename T>
  LocalAssignment::LocalAssignment(libport::serialize::ISerializer<T>& ser)
    : LocalWrite(ser)
  {
    LIBPORT_USE(ser);
    depth_ = ser.template unserialize< unsigned >("depth");
    declaration_ = ser.template unserialize< rLocalDeclaration >("declaration");
  }
#endif

  inline const unsigned&
  LocalAssignment::depth_get () const
  {
    return depth_;
  }
  inline unsigned&
  LocalAssignment::depth_get ()
  {
    return depth_;
  }
  inline void
  LocalAssignment::depth_set (unsigned depth)
  {
    depth_ = depth;
  }

  inline const rLocalDeclaration&
  LocalAssignment::declaration_get () const
  {
    return declaration_;
  }
  inline rLocalDeclaration&
  LocalAssignment::declaration_get ()
  {
    return declaration_;
  }
  inline void
  LocalAssignment::declaration_set (const rLocalDeclaration& declaration)
  {
    declaration_ = declaration;
  }


} // namespace ast

#endif // !AST_LOCAL_ASSIGNMENT_HXX
