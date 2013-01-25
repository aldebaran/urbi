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
 ** \file ast/routine.hxx
 ** \brief Inline methods of ast::Routine.
 */

#ifndef AST_ROUTINE_HXX
# define AST_ROUTINE_HXX

# include <ast/routine.hh>

namespace ast
{

    inline bool Routine::strict() const
    {
      return formals_;
    }


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Routine::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Exp::serialize(ser);
    ser.template serialize< bool >("closure", closure_);
    ser.template serialize< local_declarations_type* >("formals", formals_);
    ser.template serialize< rScope >("body", body_);
    ser.template serialize< local_declarations_type* >("local_variables", local_variables_);
    ser.template serialize< local_declarations_type* >("captured_variables", captured_variables_);
    ser.template serialize< unsigned >("local_size", local_size_);
    ser.template serialize< bool >("uses_call", uses_call_);
    ser.template serialize< bool >("has_imports", has_imports_);
  }

  template <typename T>
  Routine::Routine(libport::serialize::ISerializer<T>& ser)
    : Exp(ser)
  {
    LIBPORT_USE(ser);
    closure_ = ser.template unserialize< bool >("closure");
    formals_ = ser.template unserialize< local_declarations_type* >("formals");
    body_ = ser.template unserialize< rScope >("body");
    local_variables_ = ser.template unserialize< local_declarations_type* >("local_variables");
    captured_variables_ = ser.template unserialize< local_declarations_type* >("captured_variables");
    local_size_ = ser.template unserialize< unsigned >("local_size");
    uses_call_ = ser.template unserialize< bool >("uses_call");
    has_imports_ = ser.template unserialize< bool >("has_imports");
  }
#endif

  inline const bool&
  Routine::closure_get () const
  {
    return closure_;
  }
  inline bool&
  Routine::closure_get ()
  {
    return closure_;
  }

  inline const local_declarations_type*
  Routine::formals_get () const
  {
    return formals_;
  }
  inline local_declarations_type*
  Routine::formals_get ()
  {
    return formals_;
  }
  inline void
  Routine::formals_set (local_declarations_type* formals)
  {
    delete formals_;
    formals_ = formals;
  }

  inline const rScope&
  Routine::body_get () const
  {
    return body_;
  }
  inline rScope&
  Routine::body_get ()
  {
    return body_;
  }
  inline void
  Routine::body_set (rScope body)
  {
    body_ = body;
  }

  inline const local_declarations_type*
  Routine::local_variables_get () const
  {
    return local_variables_;
  }
  inline local_declarations_type*
  Routine::local_variables_get ()
  {
    return local_variables_;
  }
  inline void
  Routine::local_variables_set (local_declarations_type* local_variables)
  {
    delete local_variables_;
    local_variables_ = local_variables;
  }

  inline const local_declarations_type*
  Routine::captured_variables_get () const
  {
    return captured_variables_;
  }
  inline local_declarations_type*
  Routine::captured_variables_get ()
  {
    return captured_variables_;
  }
  inline void
  Routine::captured_variables_set (local_declarations_type* captured_variables)
  {
    delete captured_variables_;
    captured_variables_ = captured_variables;
  }

  inline const unsigned&
  Routine::local_size_get () const
  {
    return local_size_;
  }
  inline void
  Routine::local_size_set (unsigned local_size)
  {
    local_size_ = local_size;
  }

  inline const bool&
  Routine::uses_call_get () const
  {
    return uses_call_;
  }
  inline void
  Routine::uses_call_set (bool uses_call)
  {
    uses_call_ = uses_call;
  }

  inline const bool&
  Routine::has_imports_get () const
  {
    return has_imports_;
  }
  inline bool&
  Routine::has_imports_get ()
  {
    return has_imports_;
  }
  inline void
  Routine::has_imports_set (bool has_imports)
  {
    has_imports_ = has_imports;
  }


} // namespace ast

#endif // !AST_ROUTINE_HXX
