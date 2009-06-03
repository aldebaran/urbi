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
