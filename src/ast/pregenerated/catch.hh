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
 ** \file ast/catch.hh
 ** \brief Declaration of ast::Catch.
 */

#ifndef AST_CATCH_HH
# define AST_CATCH_HH

# include <ast/exp.hh>
# include <ast/match.hh>

namespace ast
{

  /// Catch.
  class Catch : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Catch node.
    Catch (const loc& location, rMatch match, const rExp& body);
    /// Destroy a Catch node.
    virtual ~Catch ();
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
    Catch(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return match.
    const rMatch& match_get () const;
    /// Return match.
    rMatch& match_get ();
    /// Return the handler to execute if we have a match.
    const rExp& body_get () const;
    /// Return the handler to execute if we have a match.
    rExp& body_get ();
    /** \} */

  protected:
    /// match.
    rMatch match_;
    /// The handler to execute if we have a match.
    rExp body_;
  };

} // namespace ast

# include <ast/catch.hxx>

#endif // !AST_CATCH_HH
