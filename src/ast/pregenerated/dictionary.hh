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
 ** \file ast/dictionary.hh
 ** \brief Declaration of ast::Dictionary.
 */

#ifndef AST_DICTIONARY_HH
# define AST_DICTIONARY_HH

# include <ast/exp.hh>
# include <vector>

namespace ast
{

  /// Dictionary.
  class Dictionary : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a Dictionary node.
    Dictionary (const loc& location, const dictionary_elts_type& value);
    /// Destroy a Dictionary node.
    virtual ~Dictionary ();
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
    Dictionary(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return stored dictionary.
    const dictionary_elts_type& value_get () const;
    /// Return stored dictionary.
    dictionary_elts_type& value_get ();
    /** \} */

  protected:
    /// Stored dictionary.
    dictionary_elts_type value_;
  };

} // namespace ast

# include <ast/dictionary.hxx>

#endif // !AST_DICTIONARY_HH
