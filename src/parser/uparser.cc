/// \file uparser.cc

//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

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
| UParser.  |
`----------*/

UParser::UParser(UConnection& cn)
  : commandTree (0),
    binaryCommand (false),
    connection (cn),
    scanner_ (),
    files_(),
    filename_ (0),
    loc_()
{
  // The first column for locations is 1.
  loc_.begin.column = loc_.end.column = 1;
}

int
UParser::parse_ ()
{
  commandTree = 0;
  errorMessage[0] = 0;
  binaryCommand = false;

  parser_type p(*this);
  p.set_debug_level (!!getenv ("YYDEBUG"));
#ifdef ENABLE_DEBUG_TRACES
  p.set_debug_level(true);
#endif
  ECHO("====================== Parse begin");
  int res = p.parse();
  ECHO("====================== Parse end: " << res);
  return res;
}

int
UParser::process(const ubyte* command, int length)
{
  assert (!filename_);
  // It has been said Flex scanners cannot work with istrstream.
  std::istrstream mem_buff (reinterpret_cast<const char*> (command), length);
  std::istream mem_input (mem_buff.rdbuf());
  scanner_.switch_streams(&mem_input, 0);
  ECHO("Parsing string: ==================" << std::endl
       << loc_ << ':' << std::endl
       << std::string (reinterpret_cast<const char*>(command), length)
       << std::endl
       << "==================================");
  return parse_();
}

int
UParser::process(const std::string& fn)
{
  assert (!filename_);

  // Store the filename, and get a pointer to it.
  filename_ = &(*files_.insert (fn).first);

  // A location pointing to it.
  location_type loc;
  loc.initialize (filename_);
  // The convention for the first column changed: make sure the first
  // column is column 1.
  loc.begin.column = loc.end.column = 1;

  // Exchange with the current location so that we can restore it
  // afterwards (when reading the input flow, we want to be able to
  // restore the cursor after having handled a load command).
  std::swap(loc, loc_);
  std::ifstream f (fn.c_str());
  scanner_.switch_streams(&f, 0);
  ECHO("Parsing file: " << *filename_);
  int res = parse_();
  filename_ = 0;
  std::swap(loc, loc_);
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
