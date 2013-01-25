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
 ** \file ast/match.hh
 ** \brief Declaration of ast::Match.
 */

#ifndef AST_MATCH_HH
# define AST_MATCH_HH

# include <ast/exp.hh>
# include <ast/ast.hh>

namespace ast
{

  /// Match.
  class Match : public Ast
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Match node.
    Match (const loc& location, const rExp& pattern, rExp guard);
    /// Destroy a Match node.
    virtual ~Match ();
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
    Match(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return pattern.
    const rExp& pattern_get () const;
    /// Return pattern.
    rExp& pattern_get ();
    /// Set pattern.
    void pattern_set (const rExp&);
    /// Return guard.
    const rExp& guard_get () const;
    /// Return guard.
    rExp& guard_get ();
    /// Set guard.
    void guard_set (rExp);
    /// Return bindings.
    const rExp& bindings_get () const;
    /// Return bindings.
    rExp& bindings_get ();
    /// Set bindings.
    void bindings_set (rExp);
    /** \} */

  protected:
    /// pattern.
    rExp pattern_;
    /// guard.
    rExp guard_;
    /// bindings.
    rExp bindings_;
  };

} // namespace ast

# include <ast/match.hxx>

#endif // !AST_MATCH_HH
