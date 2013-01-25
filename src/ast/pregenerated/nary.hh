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
 ** \file ast/nary.hh
 ** \brief Declaration of ast::Nary.
 */

#ifndef AST_NARY_HH
# define AST_NARY_HH

# include <ast/exp.hh>
# include <ast/exps-type.hh>
#include <ast/flavor.hh>


namespace ast
{

/// A list of \c Exp.
  class URBI_SDK_API Nary : public Exp
  {
  public:
    /// A completely empty Nary node.
    Nary();

    /// Drop the children.
    void clear();

    /// Are there any children?
    virtual bool empty() const;

    /// Is there just one child?
    virtual bool single() const;

    /// Push a new expression, creating a Statement node.
    void push_back(rExp e, flavor_type f = flavor_none);

    /// Sets the flavor of the last child.
    void back_flavor_set(flavor_type k, const loc& l);

    /// Sets the flavor of the last child without modifying location.
    void back_flavor_set(flavor_type k);

    /// Get the flavor of the last child.
    flavor_type back_flavor_get() const;

    /// Drop the last children.
    void pop_back();

    /// Splice the content of \a rhs at the back this list.
    void splice_back(rNary rhs);

 private:
    /// Return the last child if there is one of the right type.
    template <typename T>
    libport::intrusive_ptr<T> back();

    /// Return the last child if there is one of the right type.
    template <typename T>
    libport::intrusive_ptr<const T> back() const;

    /// Adjust the whole location.
    void location_adjust();

    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Nary node.
    Nary (const loc& location);
    /// Destroy a Nary node.
    virtual ~Nary ();
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
    Nary(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return subtrees possibly qualified to be background.
    const exps_type& children_get () const;
    /// Return subtrees possibly qualified to be background.
    exps_type& children_get ();
    /// Set subtrees possibly qualified to be background.
    void children_set (exps_type*);
    /** \} */

  protected:
    /// Subtrees possibly qualified to be background.
    exps_type* children_;
  };

} // namespace ast

# include <ast/nary.hxx>

#endif // !AST_NARY_HH
