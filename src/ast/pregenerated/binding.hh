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
 ** \file ast/binding.hh
 ** \brief Declaration of ast::Binding.
 */

#ifndef AST_BINDING_HH
# define AST_BINDING_HH

# include <ast/lvalue.hh>

namespace ast
{

  /// Binding.
  class Binding : public LValue
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Binding node.
    Binding (const loc& location, const rLValue& what);
    /// Destroy a Binding node.
    virtual ~Binding ();
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
    Binding(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return what.
    const rLValue& what_get () const;
    /// Return what.
    rLValue& what_get ();
    /// Return whether this slot should be constant.
    const bool& constant_get () const;
    /// Set whether this slot should be constant.
    void constant_set (bool);
    /** \} */

  protected:
    /// what.
    rLValue what_;
    /// Whether this slot should be constant.
    bool constant_;
  };

} // namespace ast

# include <ast/binding.hxx>

#endif // !AST_BINDING_HH
