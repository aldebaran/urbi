/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <boost/serialization/optional.hpp>

#include <serialize/serialize.hh>

#include <ast/all.hh>
#include <ast/serialize.hh>

namespace ast
{
  void
  serialize(rConstAst ast, std::ostream& output)
  {
    libport::serialize::BinaryOSerializer s(output);
    s.serialize<ast::rConstAst>("ast", ast);
  }

  rAst
  unserialize(std::istream& input)
  {
    libport::serialize::BinaryISerializer s(input);
    return s.unserialize<rAst>("ast");
  }
}
