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
 ** \file ast/property-write.hh
 ** \brief Declaration of ast::PropertyWrite.
 */

#ifndef AST_PROPERTY_WRITE_HH
# define AST_PROPERTY_WRITE_HH

# include <ast/exp.hh>
# include <ast/property-action.hh>

namespace ast
{

  /// PropertyWrite.
  class PropertyWrite : public PropertyAction
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a PropertyWrite node.
    PropertyWrite (const loc& location, const rExp& owner,
                   const libport::Symbol& name, const rExp& value);
    /// Destroy a PropertyWrite node.
    virtual ~PropertyWrite ();
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
    PropertyWrite(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return value.
    const rExp& value_get () const;
    /// Return value.
    rExp& value_get ();
    /** \} */

  protected:
    /// value.
    rExp value_;
  };

} // namespace ast

# include <ast/property-write.hxx>

#endif // !AST_PROPERTY_WRITE_HH
