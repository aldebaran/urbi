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
 ** \file ast/decrementation.hh
 ** \brief Declaration of ast::Decrementation.
 */

#ifndef AST_DECREMENTATION_HH
# define AST_DECREMENTATION_HH

# include <ast/unary.hh>

namespace ast
{

  /// Decrementation.
  class Decrementation : public Unary
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Decrementation node.
    Decrementation (const loc& location, const rLValue& exp, bool post);
    /// Destroy a Decrementation node.
    virtual ~Decrementation ();
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
    Decrementation(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif

  };

} // namespace ast

# include <ast/decrementation.hxx>

#endif // !AST_DECREMENTATION_HH
