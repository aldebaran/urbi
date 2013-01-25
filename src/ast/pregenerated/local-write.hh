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
 ** \file ast/local-write.hh
 ** \brief Declaration of ast::LocalWrite.
 */

#ifndef AST_LOCAL_WRITE_HH
# define AST_LOCAL_WRITE_HH

# include <ast/exp.hh>
# include <libport/symbol.hh>

namespace ast
{

  /// LocalWrite.
  class LocalWrite : public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a LocalWrite node.
    LocalWrite (const loc& location, const libport::Symbol& what,
                const rExp& value);
    /// Destroy a LocalWrite node.
    virtual ~LocalWrite ();
    /** \} */

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    LocalWrite(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return name of the symbol being declared or updated.
    const libport::Symbol& what_get () const;
    /// Return name of the symbol being declared or updated.
    libport::Symbol& what_get ();
    /// Set name of the symbol being declared or updated.
    void what_set (const libport::Symbol&);
    /// Return initialization value.
    const rExp& value_get () const;
    /// Return initialization value.
    rExp& value_get ();
    /// Set initialization value.
    void value_set (const rExp&);
    /// Return local_index.
    const unsigned& local_index_get () const;
    /// Return local_index.
    unsigned& local_index_get ();
    /// Set local_index.
    void local_index_set (unsigned);
    /** \} */

  protected:
    /// Name of the symbol being declared or updated.
    libport::Symbol what_;
    /// Initialization value.
    rExp value_;
    /// local_index.
    unsigned local_index_;
  };

} // namespace ast

# include <ast/local-write.hxx>

#endif // !AST_LOCAL_WRITE_HH
