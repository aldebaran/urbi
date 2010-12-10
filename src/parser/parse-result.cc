/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file parser/parse-result.cc

#include <algorithm>
#include <iostream>
#include <iterator>

#include <libport/deref.hh>
#include <libport/indent.hh>
#include <libport/foreach.hh>
#include <libport/separate.hh>

#include <ast/nary.hh>
#include <ast/print.hh>

#include <parser/parse-result.hh>

namespace parser
{

  ParseResult::ParseResult()
    : status(-1)
    , ast_(0)
  {
  }

  ParseResult::ParseResult(ParseResult& rhs)
    : status(rhs.status)
    , ast_(rhs.ast_)
  {
  }

  ParseResult::~ParseResult()
  {
  }

  bool
  ParseResult::good() const
  {
    return (!status
            && ast_.get());
  }

  bool
  ParseResult::perfect() const
  {
    return good();
  }

  std::ostream&
  ParseResult::dump(std::ostream& o) const
  {
    return o
      << "Status:"
      << libport::incendl << status << libport ::decendl
      << "Ast:"
      << libport::incendl << libport::deref << ast_ << libport ::decendl
      ;
  }

  /*------.
  | Ast.  |
  `------*/

  // Not in the *.hxx to avoid the #include <ast/nary.hh>.
  void
  ParseResult::ast_set(ast_type ast)
  {
    ast_ = ast;
  }

  // Not in the *.hxx to avoid the #include <ast/nary.hh>.
  ParseResult::ast_type
  ParseResult::ast_get()
  {
    return ast_;
  }

  // Not in the *.hxx to avoid the #include <ast/nary.hh>.
  ParseResult::ast_type
  ParseResult::ast_xget()
  {
    return assert_exp(ast_get());
  }

}
