/// \file uparser.cc

#include <cstdlib>

#include <strstream>
#include <sstream>
#include <algorithm>
#include <string>

#include "uparser.hh"

#include "ugrammar.hh"

/*----------.
| UFlexer.  |
`----------*/

UFlexer::UFlexer(void *_uparser)
  : uparser(_uparser)
{}

void *
UFlexer::get_uparser() const
{
  return uparser;
}

/*----------.
| UParser.  |
`----------*/

UParser::UParser(UConnection& cn)
  : connection (cn),
    uflexer_ (this)
{}

int
UParser::parse_ ()
{
  commandTree = 0;
  errorMessage[0] = 0;
  binaryCommand = false;

  yy::parser p(*this);
  p.set_debug_level (!!getenv ("YYDEBUG"));
  return p.parse();
}

int
UParser::process(ubyte* command, int length)
{
  // It has been said Flex scanner cannot work with istrstream.
  std::istrstream mem_buff ((char*)command, length);
  std::istream mem_input (mem_buff.rdbuf());
  uflexer_.switch_streams(&mem_input, 0);
  return parse_();
}

int
UParser::process(const char* fn)
{
  std::ifstream f (fn);
  uflexer_.switch_streams(&f, 0);
  return parse_();
}


yy::parser::token_type
UParser::scan(yy::parser::semantic_type* val, yy::parser::location_type* loc)
{
  return uflexer_.yylex(val, loc, *this);
}

void
UParser::error (const yy::parser::location_type& l, const std::string& msg)
{
  std::ostringstream sstr;
  sstr << "!!! " << l << ": " << msg << "\n" << std::ends;
  strncpy(errorMessage, sstr.str().c_str(),
	  std::min(sizeof (errorMessage), sstr.str().size()));
}
