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
 ** \file ast/return.hh
 ** \brief Declaration of ast::Return.
 */

#ifndef AST_RETURN_HH
# define AST_RETURN_HH

# include <ast/exp.hh>

namespace ast
{

/// Return a value from a function or closure.
  class Return : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Return node.
    Return (const loc& location, rExp value);
    /// Destroy a Return node.
    virtual ~Return ();
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
    Return(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the value to return. may be empty.
    const rExp& value_get () const;
    /// Return the value to return. may be empty.
    rExp& value_get ();
    /** \} */

  protected:
    /// The value to return. May be empty.
    rExp value_;
  };

} // namespace ast

# include <ast/return.hxx>

#endif // !AST_RETURN_HH
