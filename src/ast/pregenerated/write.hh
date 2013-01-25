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
 ** \file ast/write.hh
 ** \brief Declaration of ast::Write.
 */

#ifndef AST_WRITE_HH
# define AST_WRITE_HH

# include <ast/exp.hh>
# include <ast/lvalue.hh>

namespace ast
{

  /// Write.
  class Write : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Write node.
    Write (const loc& location, const rLValue& what, const rExp& value);
    /// Destroy a Write node.
    virtual ~Write ();
    /** \} */

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    Write(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return what is being declared.
    const rLValue& what_get () const;
    /// Return what is being declared.
    rLValue& what_get ();
    /// Return initialization value.
    const rExp& value_get () const;
    /// Return initialization value.
    rExp& value_get ();
    /** \} */

  protected:
    /// What is being declared.
    rLValue what_;
    /// Initialization value.
    rExp value_;
  };

} // namespace ast

# include <ast/write.hxx>

#endif // !AST_WRITE_HH
