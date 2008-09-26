#include <libport/assert.hh>
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
  // Ugly code duplication here, not sure how to address it.
  parse_result_type
  parse(const std::string& cmd, const yy::location& l)
  {
    UParser p;
    parse_result_type res = p.parse(cmd, &l);
    res->dump_errors();
    passert(*res, !res->status);
    return res;
  }

  parse_result_type
  parse_file(const std::string& file)
  {
    UParser p;
    parse_result_type res = p.parse_file(file);
    res->dump_errors();
    passert(*res, !res->status);
    return res;
  }

}
