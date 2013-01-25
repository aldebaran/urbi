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
 ** \file ast/lvalue-args.hh
 ** \brief Declaration of ast::LValueArgs.
 */

#ifndef AST_LVALUE_ARGS_HH
# define AST_LVALUE_ARGS_HH

# include <ast/exps-type.hh>
# include <ast/lvalue.hh>

namespace ast
{

  /// LValueArgs.
  class LValueArgs : public LValue
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Construct a LValueArgs node.
    LValueArgs (const loc& location, exps_type* arguments);
    /// Destroy a LValueArgs node.
    virtual ~LValueArgs ();
    /** \} */

#if defined ENABLE_SERIALIZATION
    /// \name Serialization.
    /// \{ */
  public:
    template <typename T>
    LValueArgs(libport::serialize::ISerializer<T>& ser);
    template <typename T>
    void
    serialize(libport::serialize::OSerializer<T>& ser) const;
    /// \}
#endif


    /** \name Accessors.
     ** \{ */
  public:
    /// Return arguments passed to the function.
    const exps_type* arguments_get () const;
    /// Return arguments passed to the function.
    exps_type* arguments_get ();
    /// Set arguments passed to the function.
    void arguments_set (exps_type*);
    /** \} */

  protected:
    /// Arguments passed to the function.
    exps_type* arguments_;
  };

} // namespace ast

# include <ast/lvalue-args.hxx>

#endif // !AST_LVALUE_ARGS_HH
