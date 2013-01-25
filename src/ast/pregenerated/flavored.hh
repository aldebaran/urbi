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
 ** \file ast/flavored.hh
 ** \brief Declaration of ast::Flavored.
 */

#ifndef AST_FLAVORED_HH
# define AST_FLAVORED_HH

# include <ast/exp.hh>
# include <ast/flavor.hh>

namespace ast
{

  /// Flavored.
  class Flavored : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Flavored node.
    Flavored (const loc& location, const flavor_type& flavor);
    /// Destroy a Flavored node.
    virtual ~Flavored ();
    /** \} */

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    Flavored(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return execution model.
    const flavor_type& flavor_get () const;
    /// Set execution model.
    void flavor_set (const flavor_type&);
    /** \} */

  protected:
    /// Execution model.
    flavor_type flavor_;
  };

} // namespace ast

# include <ast/flavored.hxx>

#endif // !AST_FLAVORED_HH
