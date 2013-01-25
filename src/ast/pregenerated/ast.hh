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
 ** \file ast/ast.hh
 ** \brief Declaration of ast::Ast.
 */

#ifndef AST_AST_HH
# define AST_AST_HH

# include <libport/ref-counted.hh>
# include <ast/loc.hh>
# include <libport/hierarchy.hh>
// At least OS X's GCC does not actually export functions that
// are flagged as exported, but use types that are not exported.
// So we need to export these, for sake of binder::bind.
// The impact on the size is neglectible.
# include <urbi/export.hh>

# include <ast/fwd.hh>
# include <urbi/object/fwd.hh>
# include <urbi/runner/fwd.hh>

# include <kernel/config.h>
# if defined ENABLE_SERIALIZATION
#  include <serialize/serialize.hh>
# endif


namespace ast
{

  /// Ast.
  class URBI_SDK_API Ast : public libport::RefCounted, public libport::meta::Hierarchy<Ast, Nodes>
  {
  public:
    /// Store a reference onto the original node.
    /// If the original node also has an original reference,
    /// reuse it.
    void original_set(const rConstAst&);

    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct an Ast node.
    Ast (const loc& location);
    /// Destroy an Ast node.
    virtual ~Ast ();
    /** \} */

    /// \name Visitors entry point.
    /// \{ */
  public:
    /// Accept a const visitor \a v.
    virtual void accept (ConstVisitor& v) const = 0;
    /// Accept a non-const visitor \a v.
    virtual void accept (Visitor& v) = 0;
    /// Evaluate the node in AST interpreter \a r.
    virtual urbi::object::rObject
    eval (runner::Job& r) const = 0;
    /// Return the node type
    virtual std::string node_type() const = 0;
    /// \}

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    Ast(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return scanner position information.
    const loc& location_get () const;
    /// Set scanner position information.
    void location_set (const loc&);
    /// Return original node, if this is a rewrite.
    const rConstAst& original_get () const;
    /** \} */

  protected:
    /// Scanner position information.
    loc location_;
    /// Original node, if this is a rewrite.
    rConstAst original_;
  };

} // namespace ast

# include <ast/ast.hxx>

#endif // !AST_AST_HH
