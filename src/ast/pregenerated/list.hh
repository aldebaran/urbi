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
 ** \file ast/list.hh
 ** \brief Declaration of ast::List.
 */

#ifndef AST_LIST_HH
# define AST_LIST_HH

# include <ast/exp.hh>
# include <ast/exps-type.hh>

namespace ast
{

  /// List.
  class List : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a List node.
    List (const loc& location, exps_type* value);
    /// Destroy a List node.
    virtual ~List ();
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
    List(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return stored list value.
    const exps_type& value_get () const;
    /// Return stored list value.
    exps_type& value_get ();
    /** \} */

  protected:
    /// Stored list value.
    exps_type* value_;
  };

} // namespace ast

# include <ast/list.hxx>

#endif // !AST_LIST_HH
