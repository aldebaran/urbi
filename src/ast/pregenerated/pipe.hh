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
 ** \file ast/pipe.hh
 ** \brief Declaration of ast::Pipe.
 */

#ifndef AST_PIPE_HH
# define AST_PIPE_HH

# include <ast/composite.hh>

namespace ast
{

  /// Pipe.
  class Pipe : public Composite
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Pipe node.
    Pipe (const loc& location, const exps_type& children);
    /// Destroy a Pipe node.
    virtual ~Pipe ();
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
    Pipe(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif

  };

} // namespace ast

# include <ast/pipe.hxx>

#endif // !AST_PIPE_HH
