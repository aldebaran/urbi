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
 ** \file ast/assignment.hh
 ** \brief Declaration of ast::Assignment.
 */

#ifndef AST_ASSIGNMENT_HH
# define AST_ASSIGNMENT_HH

# include <ast/write.hh>

namespace ast
{

  /// Assignment.
  class Assignment : public Write
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct an Assignment node.
    Assignment (const loc& location, const rLValue& what,
                const rExp& value);
    /// Destroy an Assignment node.
    virtual ~Assignment ();
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
    Assignment(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif

  };

} // namespace ast

# include <ast/assignment.hxx>

#endif // !AST_ASSIGNMENT_HH
