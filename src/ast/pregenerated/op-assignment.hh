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
 ** \file ast/op-assignment.hh
 ** \brief Declaration of ast::OpAssignment.
 */

#ifndef AST_OP_ASSIGNMENT_HH
# define AST_OP_ASSIGNMENT_HH

# include <libport/symbol.hh>
# include <ast/write.hh>

namespace ast
{

  /// OpAssignment.
  class OpAssignment : public Write
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct an OpAssignment node.
    OpAssignment (const loc& location, const rLValue& what,
                  const rExp& value, const libport::Symbol& op);
    /// Destroy an OpAssignment node.
    virtual ~OpAssignment ();
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
    OpAssignment(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return op.
    const libport::Symbol& op_get () const;
    /// Return op.
    libport::Symbol& op_get ();
    /** \} */

  protected:
    /// op.
    libport::Symbol op_;
  };

} // namespace ast

# include <ast/op-assignment.hxx>

#endif // !AST_OP_ASSIGNMENT_HH
