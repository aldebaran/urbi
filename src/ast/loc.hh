/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file ast/loc.hh
 ** \brief Definition of ast::loc.
 */

#ifndef AST_LOC_HH
# define AST_LOC_HH

# include <libport/symbol.hh>

# include <urbi/parser/location.hh>

// If you have an error from here, it probably means that you used
// LOCATION_HERE without first calling DECLARE_LOCATION_FILE (alone,
// visible from the scopes using LOCATION_HERE).
# define DECLARE_LOCATION_FILE                          \
  static /* const */ ::libport::Symbol                  \
  _DECLARE_LOCATION_FILE_is_missing =                   \
    ::ast::declare_location_file(__SRCDIR__, __FILE__);

# define LOCATION_HERE                                          \
  ::ast::loc(&_DECLARE_LOCATION_FILE_is_missing, __LINE__)

namespace ast
{
  typedef yy::location loc;

  /// A Symbol that represents the current source file.
  /// Also add it to the urbi::system_files_get.
  ::libport::Symbol
  declare_location_file(const std::string& srcdir, std::string file);
}

#endif // !AST_LOC_HH
