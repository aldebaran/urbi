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
 ** \file ast/string.hh
 ** \brief Declaration of ast::String.
 */

#ifndef AST_STRING_HH
# define AST_STRING_HH

# include <ast/exp.hh>
# include <string>

namespace ast
{

  /// String.
  class String : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a String node.
    String (const loc& location, const std::string& value);
    /// Destroy a String node.
    virtual ~String ();
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
    String(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return string value.
    const std::string& value_get () const;
    /** \} */

  protected:
    /// String value.
    std::string value_;
  };

} // namespace ast

# include <ast/string.hxx>

#endif // !AST_STRING_HH
