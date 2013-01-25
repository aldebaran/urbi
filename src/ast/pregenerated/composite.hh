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
 ** \file ast/composite.hh
 ** \brief Declaration of ast::Composite.
 */

#ifndef AST_COMPOSITE_HH
# define AST_COMPOSITE_HH

# include <ast/exp.hh>
# include <ast/exps-type.hh>

namespace ast
{

  /// Composite.
  class Composite : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Composite node.
    Composite (const loc& location, const exps_type& children);
    /// Destroy a Composite node.
    virtual ~Composite ();
    /** \} */

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    Composite(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return subexpressions.
    const exps_type& children_get () const;
    /// Return subexpressions.
    exps_type& children_get ();
    /// Set subexpressions.
    void children_set (const exps_type&);
    /** \} */

  protected:
    /// Subexpressions.
    exps_type children_;
  };

} // namespace ast

# include <ast/composite.hxx>

#endif // !AST_COMPOSITE_HH
