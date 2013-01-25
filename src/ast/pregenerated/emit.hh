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
 ** \file ast/emit.hh
 ** \brief Declaration of ast::Emit.
 */

#ifndef AST_EMIT_HH
# define AST_EMIT_HH

# include <ast/exp.hh>
# include <ast/exps-type.hh>

namespace ast
{

  /// Emit.
  class Emit : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct an Emit node.
    Emit (const loc& location, const rExp& event, exps_type* arguments,
          rExp duration);
    /// Destroy an Emit node.
    virtual ~Emit ();
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
    Emit(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the emitted event.
    const rExp& event_get () const;
    /// Return the emitted event.
    rExp& event_get ();
    /// Return values carried by the event.
    const exps_type* arguments_get () const;
    /// Return values carried by the event.
    exps_type* arguments_get ();
    /// Return duration of the emission.
    const rExp& duration_get () const;
    /// Return duration of the emission.
    rExp& duration_get ();
    /** \} */

  protected:
    /// The emitted event.
    rExp event_;
    /// Values carried by the event.
    exps_type* arguments_;
    /// Duration of the emission.
    rExp duration_;
  };

} // namespace ast

# include <ast/emit.hxx>

#endif // !AST_EMIT_HH
