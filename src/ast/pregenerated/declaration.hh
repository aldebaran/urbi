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
 ** \file ast/declaration.hh
 ** \brief Declaration of ast::Declaration.
 */

#ifndef AST_DECLARATION_HH
# define AST_DECLARATION_HH

# include <ast/write.hh>

namespace ast
{

  /// Declaration.
  class Declaration : public Write
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Declaration node.
    Declaration (const loc& location, const rLValue& what,
                 const rExp& value);
    /// Destroy a Declaration node.
    virtual ~Declaration ();
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
    Declaration(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return whether this slot should be constant.
    const bool& constant_get () const;
    /// Set whether this slot should be constant.
    void constant_set (bool);
    /** \} */

  protected:
    /// Whether this slot should be constant.
    bool constant_;
  };

} // namespace ast

# include <ast/declaration.hxx>

#endif // !AST_DECLARATION_HH
