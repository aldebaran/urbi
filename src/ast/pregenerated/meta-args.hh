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
 ** \file ast/meta-args.hh
 ** \brief Declaration of ast::MetaArgs.
 */

#ifndef AST_META_ARGS_HH
# define AST_META_ARGS_HH

# include <ast/lvalue.hh>

namespace ast
{

  /// MetaArgs.
  class MetaArgs : public LValue
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a MetaArgs node.
    MetaArgs (const loc& location, const rLValue& lvalue, unsigned id);
    /// Destroy a MetaArgs node.
    virtual ~MetaArgs ();
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
    MetaArgs(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return target metavalue.
    const rLValue& lvalue_get () const;
    /// Return target metavalue.
    rLValue& lvalue_get ();
    /// Return index of the arguments meta-variable.
    const unsigned& id_get () const;
    /// Return index of the arguments meta-variable.
    unsigned& id_get ();
    /** \} */

  protected:
    /// Target metavalue.
    rLValue lvalue_;
    /// Index of the arguments meta-variable.
    unsigned id_;
  };

} // namespace ast

# include <ast/meta-args.hxx>

#endif // !AST_META_ARGS_HH
