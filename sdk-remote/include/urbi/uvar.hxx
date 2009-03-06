/// \file urbi/uvar.hxx

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008, 2009 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

#ifndef URBI_UVAR_HXX
# define URBI_UVAR_HXX

# include <urbi/uvar.hh>

namespace urbi
{
  inline
  UVar::operator bool() const
  {
    return static_cast<int>(*this) != 0;
  }

  inline
  UVar::UVar()
    : owned(false)
    , VAR_PROP_INIT
    , vardata(0)
    , name("noname")
  {}

  inline
  UVar::UVar(UVar&)
    : owned(false)
    , VAR_PROP_INIT
    , vardata(0)
    , name()
  {
    /// FIXME: This is really weird: a copy-ctor that does not use
    /// the lhs?
  }

} // end namespace urbi

#endif // ! URBI_UVAR_HXX
