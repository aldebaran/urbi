/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <sstream>

#include <parser/parser-utils.hh>

namespace parser
{

  std::string
  message_format(const yy::parser::location_type& l,
		 const std::string& msg)
  {
    std::ostringstream o;
    o << "!!! " << l << ": " << msg;
    return o.str();
  }

} // namespace parser
