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
 ** \file ast/meta-lvalue.hh
 ** \brief Declaration of ast::MetaLValue.
 */

#ifndef AST_META_LVALUE_HH
# define AST_META_LVALUE_HH

# include <ast/lvalue-args.hh>

namespace ast
{

/// A node which is a placeholder for a Call.
  class MetaLValue : public LValueArgs
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a MetaLValue node.
    MetaLValue (const loc& location, exps_type* arguments, unsigned id);
    /// Destroy a MetaLValue node.
    virtual ~MetaLValue ();
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
    MetaLValue(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the index of the meta-variable.
    const unsigned& id_get () const;
    /// Return the index of the meta-variable.
    unsigned& id_get ();
    /** \} */

  protected:
    /// The index of the meta-variable.
    unsigned id_;
  };

} // namespace ast

# include <ast/meta-lvalue.hxx>

#endif // !AST_META_LVALUE_HH
