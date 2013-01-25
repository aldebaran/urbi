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
 ** \file ast/property-action.hh
 ** \brief Declaration of ast::PropertyAction.
 */

#ifndef AST_PROPERTY_ACTION_HH
# define AST_PROPERTY_ACTION_HH

# include <ast/exp.hh>
# include <ast/lvalue.hh>
# include <libport/symbol.hh>

namespace ast
{

  /// PropertyAction.
  class PropertyAction : public LValue
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a PropertyAction node.
    PropertyAction (const loc& location, const rExp& owner,
                    const libport::Symbol& name);
    /// Destroy a PropertyAction node.
    virtual ~PropertyAction ();
    /** \} */

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    PropertyAction(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return owner.
    const rExp& owner_get () const;
    /// Return owner.
    rExp& owner_get ();
    /// Return name.
    const libport::Symbol& name_get () const;
    /// Return name.
    libport::Symbol& name_get ();
    /** \} */

  protected:
    /// owner.
    rExp owner_;
    /// name.
    libport::Symbol name_;
  };

} // namespace ast

# include <ast/property-action.hxx>

#endif // !AST_PROPERTY_ACTION_HH
