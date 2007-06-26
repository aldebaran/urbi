/*! \file uparser.h
 *******************************************************************************

 File: uparser.h\n
 Definition of the UParser class used to make flex/bison reentrant.

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

# include <set>
# include <string>

# include "kernel/fwd.hh"
# include "kernel/utypes.hh"

# include "flavorable.hh"
# include "uvariablename.hh"
# include "ugrammar.hh"
# include "parser/bison/flex-lexer.hh"


# undef  YY_DECL
# define YY_DECL                                                 \
  UParser::token_type						 \
  yyFlexLexer::yylex(UParser::semantic_type* valp,		 \
		     UParser::location_type* locp,		 \
		     UParser& up)

//! UParser uses 'flex' and 'bison' as scanner/parser
/*! The choice of flex/bison is detailed in the comment on the UServer class.
    The main concern is about reentrancy, which is not necessary in most
    cases but would become a problem with a multithreaded server.
*/
class UParser
{
public:
  typedef yy::parser parser_type;
  typedef parser_type::token_type token_type;
  typedef parser_type::semantic_type semantic_type;
  typedef parser_type::location_type location_type;

  UParser(UConnection& cn);

  /// Parse the command from a buffer.
  int process(const ubyte* command, int length);

  /// Parse a file.
  int process (const std::string& fn);

  UCommand_TREE *commandTree;
  bool          binaryCommand;

  token_type scan(semantic_type* val, location_type* loc);

  /// Declare an error at \a l about \a msg.
  void error (const location_type& l, const std::string& msg);

  /// The latest parse error message.
  char errorMessage[1024];

  /// The connection we belong to.
  UConnection& connection;

private:
  // Give access to loc_ and scanner_.
  friend int parser_type::parse ();
  friend token_type yylex (semantic_type*, location_type*, UParser&);

  /// Run the parse.  Expects the scanner to be initialized.
  int parse_ ();

  /// The Flex scanner.
  yyFlexLexer scanner_;

  /// The current location.
  location_type loc_;
};

#endif
