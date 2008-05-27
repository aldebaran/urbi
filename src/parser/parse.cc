#include <libport/assert.hh>
#include <libport/indent.hh>

// FIXME: Understand why.
#include <ast/nary.hh>

#include <parser/parse.hh>
#include <parser/parse-result.hh>
#include <parser/tweast.hh>
#include <parser/uparser.hh>

namespace parser
{
  // Ugly code duplication here, not sure how to address it.
  parse_result_type
  parse(const std::string& cmd)
  {
    UParser p;
    parse_result_type res = p.parse(cmd);
    res->dump_errors();
    passert(*res, !res->status);
    return res;
  }

  parse_result_type
  parse(Tweast& t)
  {
    UParser p;
    parse_result_type res = p.parse(t);
    if (!res->good())
    {
      std::cerr << "Tweast parsing failed:"
                << libport::incendl << t << libport::decendl;
      res->dump_errors();
      abort();
    }
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
