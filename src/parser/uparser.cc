/// \file uparser.cc

#include <cstdlib>

#include <strstream>
#include <sstream>
#include <algorithm>
#include <string>

#include "uparser.hh"

#include "ugrammar.hh"

UFlexer::UFlexer(void *_uparser)
  : uparser(_uparser)
{}

void *
UFlexer::get_uparser() const
{
  return uparser;
}

UParser::UParser()
  : uflexer(this)
{}

int
UParser::process(ubyte* command, int length, UConnection* connection_)
{
  connection = connection_;
  commandTree = 0;
  errorMessage[0] = 0;
  binaryCommand = false;

  // Pass the stream to scan.
  std::istrstream mem_buff ((char*)command, length);
  std::istream mem_input (mem_buff.rdbuf());
  uflexer.switch_streams(&mem_input, 0);

  yy::parser p(*this);
  p.set_debug_level (!!getenv ("YYDEBUG"));
  return p.parse();
}


yy::parser::token_type
UParser::scan(yy::parser::semantic_type* val, yy::parser::location_type* loc)
{
  return uflexer.yylex(val,loc,*this);
}

void
UParser::error (const yy::parser::location_type& l, const std::string& msg)
{
  std::ostringstream sstr;
  sstr << "!!! " << l << ": " << msg << "\n" << std::ends;
  strncpy(errorMessage, sstr.str().c_str(),
	  std::min(sizeof (errorMessage), sstr.str().size()));
}
