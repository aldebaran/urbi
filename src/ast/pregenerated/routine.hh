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
 ** \file ast/routine.hh
 ** \brief Declaration of ast::Routine.
 */

#ifndef AST_ROUTINE_HH
# define AST_ROUTINE_HH

# include <ast/exp.hh>
# include <ast/local-declarations-type.hh>
# include <ast/scope.hh>

namespace ast
{

  /// Routine.
  class Routine : public Exp
  {
  public:
    /// Whether the arguments must be evaluated.
    bool strict() const;

    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Routine node.
    Routine (const loc& location, bool closure,
             local_declarations_type* formals, rScope body);
    /// Destroy a Routine node.
    virtual ~Routine ();
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
    Routine(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return whether is a closure, or a function.
    const bool& closure_get () const;
    /// Return whether is a closure, or a function.
    bool& closure_get ();
    /// Return formal arguments.  if null, a lazy function.
    const local_declarations_type* formals_get () const;
    /// Return formal arguments.  if null, a lazy function.
    local_declarations_type* formals_get ();
    /// Set formal arguments.  if null, a lazy function.
    void formals_set (local_declarations_type*);
    /// Return body.
    const rScope& body_get () const;
    /// Return body.
    rScope& body_get ();
    /// Set body.
    void body_set (rScope);
    /// Return local_variables.
    const local_declarations_type* local_variables_get () const;
    /// Return local_variables.
    local_declarations_type* local_variables_get ();
    /// Set local_variables.
    void local_variables_set (local_declarations_type*);
    /// Return captured_variables.
    const local_declarations_type* captured_variables_get () const;
    /// Return captured_variables.
    local_declarations_type* captured_variables_get ();
    /// Set captured_variables.
    void captured_variables_set (local_declarations_type*);
    /// Return local_size.
    const unsigned& local_size_get () const;
    /// Set local_size.
    void local_size_set (unsigned);
    /// Return uses_call.
    const bool& uses_call_get () const;
    /// Set uses_call.
    void uses_call_set (bool);
    /// Return whether an import directive is present inside.
    const bool& has_imports_get () const;
    /// Return whether an import directive is present inside.
    bool& has_imports_get ();
    /// Set whether an import directive is present inside.
    void has_imports_set (bool);
    /** \} */

  protected:
    /// Whether is a closure, or a function.
    bool closure_;
    /// Formal arguments.  If null, a lazy function.
    local_declarations_type* formals_;
    /// Body.
    rScope body_;
    /// local_variables.
    local_declarations_type* local_variables_;
    /// captured_variables.
    local_declarations_type* captured_variables_;
    /// local_size.
    unsigned local_size_;
    /// uses_call.
    bool uses_call_;
    /// Whether an import directive is present inside.
    bool has_imports_;
  };

} // namespace ast

# include <ast/routine.hxx>

#endif // !AST_ROUTINE_HH
