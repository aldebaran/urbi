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
 ** \file ast/match.hxx
 ** \brief Inline methods of ast::Match.
 */

#ifndef AST_MATCH_HXX
# define AST_MATCH_HXX

# include <ast/match.hh>

namespace ast
{


#if defined ENABLE_SERIALIZATION
 template <typename T>
  void
  Match::serialize(libport::serialize::OSerializer<T>& ser) const
  {
    LIBPORT_USE(ser);
    Ast::serialize(ser);
    ser.template serialize< rExp >("pattern", pattern_);
    ser.template serialize< rExp >("guard", guard_);
    ser.template serialize< rExp >("bindings", bindings_);
  }

  template <typename T>
  Match::Match(libport::serialize::ISerializer<T>& ser)
    : Ast(ser)
  {
    LIBPORT_USE(ser);
    pattern_ = ser.template unserialize< rExp >("pattern");
    guard_ = ser.template unserialize< rExp >("guard");
    bindings_ = ser.template unserialize< rExp >("bindings");
  }
#endif

  inline const rExp&
  Match::pattern_get () const
  {
    return pattern_;
  }
  inline rExp&
  Match::pattern_get ()
  {
    return pattern_;
  }
  inline void
  Match::pattern_set (const rExp& pattern)
  {
    pattern_ = pattern;
  }

  inline const rExp&
  Match::guard_get () const
  {
    return guard_;
  }
  inline rExp&
  Match::guard_get ()
  {
    return guard_;
  }
  inline void
  Match::guard_set (rExp guard)
  {
    guard_ = guard;
  }

  inline const rExp&
  Match::bindings_get () const
  {
    return bindings_;
  }
  inline rExp&
  Match::bindings_get ()
  {
    return bindings_;
  }
  inline void
  Match::bindings_set (rExp bindings)
  {
    bindings_ = bindings;
  }


} // namespace ast

#endif // !AST_MATCH_HXX
