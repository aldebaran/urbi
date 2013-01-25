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
 ** \file ast/subscript.hh
 ** \brief Declaration of ast::Subscript.
 */

#ifndef AST_SUBSCRIPT_HH
# define AST_SUBSCRIPT_HH

# include <ast/exp.hh>
# include <ast/lvalue-args.hh>

namespace ast
{

  /// Subscript.
  class Subscript : public LValueArgs
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Subscript node.
    Subscript (const loc& location, exps_type* arguments,
               const rExp& target);
    /// Destroy a Subscript node.
    virtual ~Subscript ();
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
    Subscript(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return target.
    const rExp& target_get () const;
    /// Return target.
    rExp& target_get ();
    /** \} */

  protected:
    /// target.
    rExp target_;
  };

} // namespace ast

# include <ast/subscript.hxx>

#endif // !AST_SUBSCRIPT_HH
