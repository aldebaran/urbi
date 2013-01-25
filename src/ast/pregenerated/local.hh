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
 ** \file ast/local.hh
 ** \brief Declaration of ast::Local.
 */

#ifndef AST_LOCAL_HH
# define AST_LOCAL_HH

# include <ast/exp.hh>
# include <ast/local-declaration.hh>
# include <ast/exps-type.hh>
# include <libport/symbol.hh>

namespace ast
{

  /// Local.
  class Local : public Exp
  {
public:
  unsigned local_index_get() const;

    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Local node.
    Local (const loc& location, const libport::Symbol& name,
           exps_type* arguments, unsigned depth);
    /// Destroy a Local node.
    virtual ~Local ();
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
    Local(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return name of the local variable.
    const libport::Symbol& name_get () const;
    /// Return name of the local variable.
    libport::Symbol& name_get ();
    /// Return arguments passed to the function.
    const exps_type* arguments_get () const;
    /// Return arguments passed to the function.
    exps_type* arguments_get ();
    /// Set arguments passed to the function.
    void arguments_set (exps_type*);
    /// Return how many contexts we have to rewind to find the actual value of the variable (thus 0 means this is an actual local variable).
    const unsigned& depth_get () const;
    /// Return how many contexts we have to rewind to find the actual value of the variable (thus 0 means this is an actual local variable).
    unsigned& depth_get ();
    /// Set how many contexts we have to rewind to find the actual value of the variable (thus 0 means this is an actual local variable).
    void depth_set (unsigned);
    /// Return declaration.
    const rLocalDeclaration& declaration_get () const;
    /// Return declaration.
    rLocalDeclaration& declaration_get ();
    /// Set declaration.
    void declaration_set (const rLocalDeclaration&);
    /** \} */

  protected:
    /// Name of the local variable.
    libport::Symbol name_;
    /// Arguments passed to the function.
    exps_type* arguments_;
    /// How many contexts we have to rewind to find the actual value of the variable (thus 0 means this is an actual local variable).
    unsigned depth_;
    /// declaration.
    rLocalDeclaration declaration_;
  };

} // namespace ast

# include <ast/local.hxx>

#endif // !AST_LOCAL_HH
