/// \file uparser.cc

#include "uparser.h"

#include <cstdlib>

#include <strstream>
#include <sstream>
#include <algorithm>
#include <string>

#include "parser/bison/ugrammar.hh"

// Parse function of 'bison' is defined externally
extern char errorMessage[1024];
extern UString** globalDelete;

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
  result = 0;

  std::istrstream * mem_buff = new std::istrstream((char*)command, length);
  if (!mem_buff) return -1;

  std::istream* mem_input = new std::istream(mem_buff->rdbuf());
  if (!mem_input) 
    {
      delete mem_buff;
      return -1;
    }

  uflexer.switch_streams(&(*mem_input), 0);// Tells flex the right stream
  binaryCommand = false;

  yy::parser p(*this);
  p.set_debug_level (!!getenv ("YYDEBUG"));
  result = p.parse();

  delete mem_input;
  delete mem_buff;

  return result;
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
