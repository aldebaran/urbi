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
 ** \file ast/try.hh
 ** \brief Declaration of ast::Try.
 */

#ifndef AST_TRY_HH
# define AST_TRY_HH

# include <ast/exp.hh>
# include <ast/catches-type.hh>
# include <ast/scope.hh>

namespace ast
{

  /// Try.
  class Try : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Try node.
    Try (const loc& location, const rScope& body,
         const catches_type& handlers, rScope elseclause);
    /// Destroy a Try node.
    virtual ~Try ();
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
    Try(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the code to protect against an exception.
    const rScope& body_get () const;
    /// Return the code to protect against an exception.
    rScope& body_get ();
    /// Return the exception handlers.
    const catches_type& handlers_get () const;
    /// Return the exception handlers.
    catches_type& handlers_get ();
    /// Return run if no exception was thrown.
    const rScope& elseclause_get () const;
    /// Return run if no exception was thrown.
    rScope& elseclause_get ();
    /** \} */

  protected:
    /// The code to protect against an exception.
    rScope body_;
    /// The exception handlers.
    catches_type handlers_;
    /// Run if no exception was thrown.
    rScope elseclause_;
  };

} // namespace ast

# include <ast/try.hxx>

#endif // !AST_TRY_HH
