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
 ** \file ast/tagged-stmt.hh
 ** \brief Declaration of ast::TaggedStmt.
 */

#ifndef AST_TAGGED_STMT_HH
# define AST_TAGGED_STMT_HH

# include <ast/exp.hh>

namespace ast
{

  /// TaggedStmt.
  class TaggedStmt : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a TaggedStmt node.
    TaggedStmt (const loc& location, const rExp& tag, const rExp& exp);
    /// Destroy a TaggedStmt node.
    virtual ~TaggedStmt ();
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
    TaggedStmt(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the tagging expression.
    const rExp& tag_get () const;
    /// Return the tagging expression.
    rExp& tag_get ();
    /// Return the tagged expression.
    const rExp& exp_get () const;
    /// Return the tagged expression.
    rExp& exp_get ();
    /** \} */

  protected:
    /// The tagging expression.
    rExp tag_;
    /// The tagged expression.
    rExp exp_;
  };

} // namespace ast

# include <ast/tagged-stmt.hxx>

#endif // !AST_TAGGED_STMT_HH
