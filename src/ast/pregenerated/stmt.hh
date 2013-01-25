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
 ** \file ast/stmt.hh
 ** \brief Declaration of ast::Stmt.
 */

#ifndef AST_STMT_HH
# define AST_STMT_HH

# include <ast/exp.hh>
# include <ast/flavored.hh>

namespace ast
{

/// An expression with its flavor.
  class Stmt : public Flavored
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Stmt node.
    Stmt (const loc& location, const flavor_type& flavor,
          const rExp& expression);
    /// Destroy a Stmt node.
    virtual ~Stmt ();
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
    Stmt(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return unqualified expression.
    const rExp& expression_get () const;
    /// Return unqualified expression.
    rExp& expression_get ();
    /** \} */

  protected:
    /// Unqualified expression.
    rExp expression_;
  };

} // namespace ast

# include <ast/stmt.hxx>

#endif // !AST_STMT_HH
