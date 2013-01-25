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
 ** \file ast/class.hh
 ** \brief Declaration of ast::Class.
 */

#ifndef AST_CLASS_HH
# define AST_CLASS_HH

# include <ast/exp.hh>
# include <ast/exps-type.hh>
# include <ast/lvalue.hh>

namespace ast
{

  /// Class.
  class Class : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Class node.
    Class (const loc& location, const rLValue& what, exps_type* protos,
           const rExp& content, bool is_package);
    /// Destroy a Class node.
    virtual ~Class ();
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
    Class(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return lvalue where to store the class.
    const rLValue& what_get () const;
    /// Return lvalue where to store the class.
    rLValue& what_get ();
    /// Return prototypes.
    const exps_type* protos_get () const;
    /// Return prototypes.
    exps_type* protos_get ();
    /// Return class content.
    const rExp& content_get () const;
    /// Return class content.
    rExp& content_get ();
    /// Return is this a class or a package.
    const bool& is_package_get () const;
    /// Set is this a class or a package.
    void is_package_set (bool);
    /** \} */

  protected:
    /// Lvalue where to store the class.
    rLValue what_;
    /// Prototypes.
    exps_type* protos_;
    /// Class content.
    rExp content_;
    /// Is this a class or a package.
    bool is_package_;
  };

} // namespace ast

# include <ast/class.hxx>

#endif // !AST_CLASS_HH
