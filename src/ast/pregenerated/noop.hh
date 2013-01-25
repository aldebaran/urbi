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
 ** \file ast/noop.hh
 ** \brief Declaration of ast::Noop.
 */

#ifndef AST_NOOP_HH
# define AST_NOOP_HH

# include <ast/scope.hh>

namespace ast
{

/// Also known as nop, noop etc.
  class Noop : public Scope
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Noop node.
    Noop (const loc& location, const rExp& body);
    /// Destroy a Noop node.
    virtual ~Noop ();
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
    Noop(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif

  };

} // namespace ast

# include <ast/noop.hxx>

#endif // !AST_NOOP_HH
