/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef PARSER_TRANSFORM_HH
# define PARSER_TRANSFORM_HH

# include <ast/fwd.hh>
# include <urbi/export.hh>

namespace parser
{
  template <typename T>
  URBI_SDK_API libport::intrusive_ptr<T>
  transform(libport::intrusive_ptr<const T> ast);
} // namespace parser

#endif // PARSER_TRANSFORM_HH
