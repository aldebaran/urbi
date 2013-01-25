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
 ** \file ast/float.hh
 ** \brief Declaration of ast::Float.
 */

#ifndef AST_FLOAT_HH
# define AST_FLOAT_HH

# include <ast/exp.hh>
# include <libport/ufloat.hh>

namespace ast
{

  /// Float.
  class Float : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Float node.
    Float (const loc& location, const libport::ufloat& value);
    /// Destroy a Float node.
    virtual ~Float ();
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
    Float(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return float value.
    const libport::ufloat& value_get () const;
    /** \} */

  protected:
    /// Float value.
    libport::ufloat value_;
  };

} // namespace ast

# include <ast/float.hxx>

#endif // !AST_FLOAT_HH
