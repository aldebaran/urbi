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
 ** \file ast/if.hh
 ** \brief Declaration of ast::If.
 */

#ifndef AST_IF_HH
# define AST_IF_HH

# include <ast/exp.hh>
# include <ast/scope.hh>

namespace ast
{

  /// If.
  class If : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct an If node.
    If (const loc& location, const rExp& test, const rScope& thenclause,
        const rScope& elseclause);
    /// Destroy an If node.
    virtual ~If ();
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
    If(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return condition to select thenclause or elseclause.
    const rExp& test_get () const;
    /// Return condition to select thenclause or elseclause.
    rExp& test_get ();
    /// Return executed if test is true.
    const rScope& thenclause_get () const;
    /// Return executed if test is true.
    rScope& thenclause_get ();
    /// Return executed if test is false.
    const rScope& elseclause_get () const;
    /// Return executed if test is false.
    rScope& elseclause_get ();
    /** \} */

  protected:
    /// Condition to select thenclause or elseclause.
    rExp test_;
    /// Executed if test is true.
    rScope thenclause_;
    /// Executed if test is false.
    rScope elseclause_;
  };

} // namespace ast

# include <ast/if.hxx>

#endif // !AST_IF_HH
