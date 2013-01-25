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
 ** \file ast/foreach.hh
 ** \brief Declaration of ast::Foreach.
 */

#ifndef AST_FOREACH_HH
# define AST_FOREACH_HH

# include <ast/exp.hh>
# include <ast/local-declaration.hh>
# include <ast/flavored.hh>
# include <ast/scope.hh>

namespace ast
{

  /// Foreach.
  class Foreach : public Flavored
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Foreach node.
    Foreach (const loc& location, const flavor_type& flavor,
             const rLocalDeclaration& index, const rExp& list,
             const rScope& body);
    /// Destroy a Foreach node.
    virtual ~Foreach ();
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
    Foreach(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return the variable storing de values on which iterate.
    const rLocalDeclaration& index_get () const;
    /// Return the variable storing de values on which iterate.
    rLocalDeclaration& index_get ();
    /// Return the list containing the values on which iterate.
    const rExp& list_get () const;
    /// Return the list containing the values on which iterate.
    rExp& list_get ();
    /// Return executed for each value in the list.
    const rScope& body_get () const;
    /// Return executed for each value in the list.
    rScope& body_get ();
    /** \} */

  protected:
    /// The variable storing de values on which iterate.
    rLocalDeclaration index_;
    /// The list containing the values on which iterate.
    rExp list_;
    /// Executed for each value in the list.
    rScope body_;
  };

} // namespace ast

# include <ast/foreach.hxx>

#endif // !AST_FOREACH_HH
