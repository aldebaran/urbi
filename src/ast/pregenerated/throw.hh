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
 ** \file ast/throw.hh
 ** \brief Declaration of ast::Throw.
 */

#ifndef AST_THROW_HH
# define AST_THROW_HH

# include <ast/exp.hh>

namespace ast
{

  /// Throw.
  class Throw : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Throw node.
    Throw (const loc& location, rExp value);
    /// Destroy a Throw node.
    virtual ~Throw ();
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
    Throw(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the value to throw.
    const rExp& value_get () const;
    /// Return the value to throw.
    rExp& value_get ();
    /** \} */

  protected:
    /// The value to throw.
    rExp value_;
  };

} // namespace ast

# include <ast/throw.hxx>

#endif // !AST_THROW_HH
