/// \file uparser.cc

#include <cstdlib>
#include <cassert>

#include <sstream>
#include <strstream>
#include <fstream>
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
    scanner_ (this)
{}

int
UParser::parse_ ()
{
  commandTree = 0;
  errorMessage[0] = 0;
  binaryCommand = false;

  parser_type p(*this);
  p.set_debug_level (!!getenv ("YYDEBUG"));
  return p.parse();
}

int
UParser::process(ubyte* command, int length)
{
  assert (filename_.empty());
  // It has been said Flex scanner cannot work with istrstream.
  std::istrstream mem_buff ((char*)command, length);
  std::istream mem_input (mem_buff.rdbuf());
  scanner_.switch_streams(&mem_input, 0);
  return parse_();
}

int
UParser::process(const char* fn)
{
  // Store in this object the name of the file, and let location point
  // to it.  Once the parsing finish, clean it.
  assert (filename_.empty ());
  filename_ = fn;
  loc_.initialize (&filename_);
  std::ifstream f (fn);
  scanner_.switch_streams(&f, 0);
  int res = parse_();
  filename_.clear ();
  loc_.initialize (0);
  return res;
}


void
UParser::error (const yy::parser::location_type& l, const std::string& msg)
{
  std::ostringstream sstr;
  sstr << "!!! " << l << ": " << msg << "\n" << std::ends;
  strncpy(errorMessage, sstr.str().c_str(),
	  std::min(sizeof (errorMessage), sstr.str().size()));
}
