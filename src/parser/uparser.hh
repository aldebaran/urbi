/*! \file uparser.h
 *******************************************************************************

 File: uparser.h\n
 Definition of the UParser class used to make flex/bison reentrant.

 This file is based on an example provided by Markus Mottl:
 See: "reent" at http://www.oefai.at/~markus

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UPARSER_HH
# define UPARSER_HH

# include <string>
# include "utypes.hh"
# include "ugrammar.hh"
# include "parser/bison/flex-lexer.hh"

class UCommand_TREE;

# undef  YY_DECL
# define YY_DECL                                                 \
  UParser::token_type						 \
  yyFlexLexer::yylex(UParser::semantic_type* valp,		 \
		     UParser::location_type* locp,		 \
		     UParser& uparser)

//! Control class for a flex-scanner
/*! It has a pointer to the uparser in which it is contained
 */
class UFlexer
  : public yyFlexLexer
{
public:
  UFlexer(void *_uparser);
  void* get_uparser() const;
private:
  void *uparser;
};


//! UParser uses 'flex' and 'bison' as scanner/parser
/*! The choice of flex/bison is detailed in the comment on the UServer class.
    The main concern is about reentrancy, which is not necessary in most
    cases but would become a problem with a multithreaded server.
*/
class UParser
{
public:
  typedef yy::parser::token_type token_type;
  typedef yy::parser::semantic_type semantic_type;
  typedef yy::parser::location_type location_type;

  UParser();

  /// Parse the command from a stream.
  /// (this is how flex C++ handles it, no choice).
  int process(ubyte* command, int length, UConnection* connection_);

  UConnection   *connection;
  UCommand_TREE *commandTree;
  bool          binaryCommand;

  token_type scan(semantic_type* val, location_type* loc);

  /// Declare an error at \a l about \a msg.
  void error (const location_type& l, const std::string& msg);

  /// The last error message from the parser.
  char errorMessage[1024];

private:
  // The scanner used in this parser (it is a flex-scanner)
  UFlexer uflexer;
};

#endif
