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
 ** \file ast/call.hh
 ** \brief Declaration of ast::Call.
 */

#ifndef AST_CALL_HH
# define AST_CALL_HH

# include <ast/exp.hh>
# include <libport/symbol.hh>
# include <ast/lvalue-args.hh>

namespace ast
{

/// Function calls, and attribute look-ups.
  class Call : public LValueArgs
  {
  public:
    /// Whether the target is implicit.
    bool target_implicit() const;

    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Call node.
    Call (const loc& location, exps_type* arguments, const rExp& target,
          const libport::Symbol& name);
    /// Destroy a Call node.
    virtual ~Call ();
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
    Call(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the target of the call.
    const rExp& target_get () const;
    /// Return the target of the call.
    rExp& target_get ();
    /// Set the target of the call.
    void target_set (const rExp&);
    /// Return name of the called function.
    const libport::Symbol& name_get () const;
    /// Return name of the called function.
    libport::Symbol& name_get ();
    /** \} */

  protected:
    /// The target of the call.
    rExp target_;
    /// Name of the called function.
    libport::Symbol name_;
  };

} // namespace ast

# include <ast/call.hxx>

#endif // !AST_CALL_HH
