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
 ** \file ast/continue.hh
 ** \brief Declaration of ast::Continue.
 */

#ifndef AST_CONTINUE_HH
# define AST_CONTINUE_HH

# include <ast/exp.hh>

namespace ast
{

/// Continue execution of a loop.
  class Continue : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Continue node.
    Continue (const loc& location);
    /// Destroy a Continue node.
    virtual ~Continue ();
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
    Continue(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif

  };

} // namespace ast

# include <ast/continue.hxx>

#endif // !AST_CONTINUE_HH
