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
 ** \file ast/watch.hh
 ** \brief Declaration of ast::Watch.
 */

#ifndef AST_WATCH_HH
# define AST_WATCH_HH

# include <ast/exp.hh>

namespace ast
{

  /// Watch.
  class Watch : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Watch node.
    Watch (const loc& location, const rExp& exp);
    /// Destroy a Watch node.
    virtual ~Watch ();
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
    Watch(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return exp.
    const rExp& exp_get () const;
    /// Return exp.
    rExp& exp_get ();
    /// Set exp.
    void exp_set (const rExp&);
    /** \} */

  protected:
    /// exp.
    rExp exp_;
  };

} // namespace ast

# include <ast/watch.hxx>

#endif // !AST_WATCH_HH
