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
 ** \file ast/do.hh
 ** \brief Declaration of ast::Do.
 */

#ifndef AST_DO_HH
# define AST_DO_HH

# include <ast/exp.hh>
# include <ast/scope.hh>

namespace ast
{

  /// Do.
  class Do : public Scope
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Do node.
    Do (const loc& location, const rExp& body, const rExp& target);
    /// Destroy a Do node.
    virtual ~Do ();
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
    Do(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the default target.
    const rExp& target_get () const;
    /// Return the default target.
    rExp& target_get ();
    /** \} */

  protected:
    /// The default target.
    rExp target_;
  };

} // namespace ast

# include <ast/do.hxx>

#endif // !AST_DO_HH
