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

/*----------.
| UParser.  |
`----------*/

UParser::UParser(UConnection& cn)
  : command_tree_ (0),
    binaryCommand (false),
    connection (cn),
    hasError_(false),
    error_ (),
    hasWarning_(false),
    warning_ (),
    scanner_ (),
    loc_()
{
  // The first column for locations is 1.
  loc_.begin.column = loc_.end.column = 1;
}

int
UParser::parse_ ()
{
  command_tree_ = 0;
  binaryCommand = false;
  hasError_ = false;
  hasWarning_ = false;

  parser_type p(*this);
#ifdef ENABLE_DEBUG_TRACES
  p.set_debug_level(true);
#else
  p.set_debug_level (!!getenv ("YYDEBUG"));
#endif
  ECHO("====================== Parse begin");
  int res = p.parse();
  ECHO("====================== Parse end: " << res);
  return res;
}

int
UParser::process (const std::string& command)
{
  // It has been said Flex scanners cannot work with istrstream.
  std::istrstream mem_buff (command.c_str ());
  std::istream mem_input (mem_buff.rdbuf());
  scanner_.switch_streams(&mem_input, 0);
  ECHO("Parsing string: ==================\n"
       << loc_ << ":\n" << command
       << "\n==================================");
  return parse_ ();
}

ast::Ast*
UParser::command_tree_get ()
{
  if (hasError ())
    return 0;
  return command_tree_;
}

const ast::Ast*
UParser::command_tree_get () const
{
  if (hasError ())
    return 0;
  return command_tree_;
}

void
UParser::command_tree_set (ast::Ast* ast)
{
  command_tree_ = ast;
}

int
UParser::process_file (const std::string& fn)
{
  // A location pointing to it.
  location_type loc;
  loc.initialize (new libport::Symbol(fn));
  // The convention for the first column changed: make sure the first
  // column is column 1.
  loc.begin.column = loc.end.column = 1;

  // Exchange with the current location so that we can restore it
  // afterwards (when reading the input flow, we want to be able to
  // restore the cursor after having handled a load command).
  std::swap(loc, loc_);
  std::ifstream f (fn.c_str());
  scanner_.switch_streams(&f, 0);
  ECHO("Parsing file: " << fn);
  int res = parse_();
  std::swap(loc, loc_);
  return res;
}

namespace
{
  /// Helper to format error and warning messages.
  std::string errorFormat(const yy::parser::location_type& l,
                          const std::string& msg)
  {
    std::ostringstream o;
    o << "!!! " << l << ": " << msg << '\n' << std::ends;
    return o.str();
  }
}

void
UParser::error (const yy::parser::location_type& l, const std::string& msg)
{
  hasError_ = true;
  error_ = errorFormat(l, msg);
}

void
UParser::warn (const yy::parser::location_type& l, const std::string& msg)
{
  hasWarning_ = true;
  warning_ = errorFormat(l, msg);
}

bool
UParser::hasError () const
{
  return hasError_;
}

std::string
UParser::error_get () const
{
  // precondition
  assert(hasError());

  return error_;
}

bool
UParser::hasWarning () const
{
  return hasWarning_;
}

std::string
UParser::warning_get () const
{
  // precondition
  assert(hasWarning());
  return warning_;
}
