/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file parser/parse.hh

#ifndef PARSER_PARSE_HH
# define PARSER_PARSE_HH

# include <memory>

# include <parser/fwd.hh>
# include <urbi/parser/location.hh>
# include <urbi/export.hh>

namespace parser
{

  /// Parse \a cmd, and return an object that holds the result and the
  /// errors.
  parse_result_type URBI_SDK_API
  parse(const std::string& cmd, const yy::location& loc);

  /// Parse \a cmd, and return an object that holds the result and the
  /// errors.  Meta variables are activated.
  parse_result_type URBI_SDK_API
  parse_meta(const std::string& cmd, const yy::location& loc);

  /// Parse a file.
  /// \see parse().
  parse_result_type URBI_SDK_API
  parse_file(const std::string& file);

}

#endif // !PARSER_PARSE_HH
