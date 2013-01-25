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
 ** \file ast/meta-id.hh
 ** \brief Declaration of ast::MetaId.
 */

#ifndef AST_META_ID_HH
# define AST_META_ID_HH

# include <ast/lvalue-args.hh>

namespace ast
{

  /// MetaId.
  class MetaId : public LValueArgs
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a MetaId node.
    MetaId (const loc& location, exps_type* arguments, unsigned id);
    /// Destroy a MetaId node.
    virtual ~MetaId ();
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
    MetaId(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return id.
    const unsigned& id_get () const;
    /// Return id.
    unsigned& id_get ();
    /** \} */

  protected:
    /// id.
    unsigned id_;
  };

} // namespace ast

# include <ast/meta-id.hxx>

#endif // !AST_META_ID_HH
