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
 ** \file ast/lvalue.hh
 ** \brief Declaration of ast::LValue.
 */

#ifndef AST_LVALUE_HH
# define AST_LVALUE_HH

# include <ast/exp.hh>

namespace ast
{

  /// LValue.
  class LValue : public Exp
  {
  public:
    /// This lvalue as a call
    rCall call();

    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a LValue node.
    LValue (const loc& location);
    /// Destroy a LValue node.
    virtual ~LValue ();
    /** \} */

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    LValue(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif

  };

} // namespace ast

# include <ast/lvalue.hxx>

#endif // !AST_LVALUE_HH
