/// \file uparser.cc

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include "ast/nary.hh"

#include "parser/uparser.hh"
#include "parser/parser-impl.hh"

namespace parser
{

  /*----------.
  | UParser.  |
  `----------*/

  UParser::UParser()
    : pimpl_(new ParserImpl)
  {
  }

  UParser::UParser(const UParser& rhs)
    : pimpl_(new ParserImpl(*rhs.pimpl_))
  {
  }

  UParser::~UParser()
  {
  }

  int
  UParser::parse(const std::string& command)
  {
    return pimpl_->parse(command);
  }

  int
  UParser::parse(Tweast& t)
  {
    return pimpl_->parse(t);
  }

  void
  UParser::dump_errors() const
  {
    return pimpl_->dump_errors();
  }

  int
  UParser::parse_file(const std::string& fn)
  {
    return pimpl_->parse_file(fn);
  }

  void
  UParser::process_errors(ast::Nary* target)
  {
    return pimpl_->process_errors(target);
  }

  std::auto_ptr<UParser::ast_type>
  UParser::ast_take()
  {
    return pimpl_->ast_take();
  }

  std::auto_ptr<UParser::ast_type>
  UParser::ast_xtake()
  {
    return pimpl_->ast_xtake();
  }

  /*--------------------------.
  | Free-standing functions.  |
  `--------------------------*/

  UParser
  parse(const std::string& cmd)
  {
    UParser res;
    int err = res.parse (cmd);
    res.dump_errors();
    passert (err, !err);
    return res;
  }

  UParser::ast_type*
  parse(Tweast& t)
  {
    UParser p;
    int err = p.parse (t);
    p.dump_errors();
    passert (err, !err);
    return p.ast_xtake().release();
  }

  UParser
  parse_file(const std::string& file)
  {
    UParser res;
    int err = res.parse_file (file);
    res.dump_errors();
    passert (err, !err);
    return res;
  }

}
