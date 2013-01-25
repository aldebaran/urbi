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
 ** \file ast/unscope.hh
 ** \brief Declaration of ast::Unscope.
 */

#ifndef AST_UNSCOPE_HH
# define AST_UNSCOPE_HH

# include <ast/exp.hh>

namespace ast
{

  /// Unscope.
  class Unscope : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct an Unscope node.
    Unscope (const loc& location, unsigned count);
    /// Destroy an Unscope node.
    virtual ~Unscope ();
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
    Unscope(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return number of scope level to bypass.
    const unsigned& count_get () const;
    /** \} */

  protected:
    /// number of scope level to bypass.
    unsigned count_;
  };

} // namespace ast

# include <ast/unscope.hxx>

#endif // !AST_UNSCOPE_HH
