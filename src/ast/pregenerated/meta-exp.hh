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
 ** \file ast/meta-exp.hh
 ** \brief Declaration of ast::MetaExp.
 */

#ifndef AST_META_EXP_HH
# define AST_META_EXP_HH

# include <ast/exp.hh>

namespace ast
{

/// A node which is a placeholder for an Exp.
  class MetaExp : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a MetaExp node.
    MetaExp (const loc& location, unsigned id);
    /// Destroy a MetaExp node.
    virtual ~MetaExp ();
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
    MetaExp(libport::serialize::ISerializer<T>& ser);
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

# include <ast/meta-exp.hxx>

#endif // !AST_META_EXP_HH
