/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cassert>
#include <libport/indent.hh>

// FIXME: Understand why.
#include <ast/nary.hh>

// Template instanciation: ur doing it wrong
#ifdef _MSC_VER
# include <ast/all.hh>
#endif

#include <parser/parse.hh>
#include <parser/parse-result.hh>
#include <parser/uparser.hh>

namespace parser
{
  namespace
  {
    static
    parse_result_type
    parse_(const std::string& cmd, const yy::location& l,
           bool meta_p)
    {
      UParser p;
      p.meta(meta_p);
      parse_result_type res = p.parse(cmd, &l);
      passert(*res, !res->status);
      return res;
    }
  }

  parse_result_type
  parse_meta(const std::string& cmd, const yy::location& l)
  {
    parse_result_type res = parse_(cmd, l, true);
    // A parametric ast should never issue parse warnings/errors.
    passert(*res, res->perfect());
    return res;
  }

  parse_result_type
  parse(const std::string& cmd, const yy::location& l)
  {
    return parse_(cmd, l, false);
  }

  // Ugly code duplication here, not sure how to address it.
  parse_result_type
  parse_file(const std::string& file)
  {
    UParser p;
    parse_result_type res = p.parse_file(file);
    passert(*res, !res->status);
    return res;
  }

}
