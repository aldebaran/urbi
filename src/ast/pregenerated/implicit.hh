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
 ** \file ast/implicit.hh
 ** \brief Declaration of ast::Implicit.
 */

#ifndef AST_IMPLICIT_HH
# define AST_IMPLICIT_HH

# include <ast/exp.hh>

namespace ast
{

/// Implicit target for messages.
  class Implicit : public Exp
  {
  public:
    /// Whether is an instance of ast::Implicit.
    virtual bool implicit() const;

    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct an Implicit node.
    Implicit (const loc& location);
    /// Destroy an Implicit node.
    virtual ~Implicit ();
    /** \} */

    /// \name Visitors entry point.
    /// \{ */
  public:
    /// Accept a const visitor \a v.
    virtual void accept (ConstVisitor& v) const;
    /// Accept a non-const visitor \a v.
    virtual void accept (Visitor& v);
    /// Evaluate the node in AST interpreter \a r.
    virtual urbi::object::rObject
    eval (runner::Job& r) const;
    /// Return the node type
    virtual std::string node_type() const;
    /// \}

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    Implicit(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif

  };

} // namespace ast

# include <ast/implicit.hxx>

#endif // !AST_IMPLICIT_HH
