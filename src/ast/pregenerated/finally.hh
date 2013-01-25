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
 ** \file ast/finally.hh
 ** \brief Declaration of ast::Finally.
 */

#ifndef AST_FINALLY_HH
# define AST_FINALLY_HH

# include <ast/exp.hh>

namespace ast
{

  /// Finally.
  class Finally : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Finally node.
    Finally (const loc& location, const rExp& body, const rExp& finally);
    /// Destroy a Finally node.
    virtual ~Finally ();
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
    Finally(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return body.
    const rExp& body_get () const;
    /// Return body.
    rExp& body_get ();
    /// Return finally.
    const rExp& finally_get () const;
    /// Return finally.
    rExp& finally_get ();
    /** \} */

  protected:
    /// body.
    rExp body_;
    /// finally.
    rExp finally_;
  };

} // namespace ast

# include <ast/finally.hxx>

#endif // !AST_FINALLY_HH
