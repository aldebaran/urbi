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

#ifndef UPARSER_H_DEFINED
# define UPARSER_H_DEFINED

# include <string>
# include "utypes.h"
# undef IN

# include "ugrammar.hh"
# include "parser/bison/flex-lexer.hh"

class UCommand_TREE;

# undef  YY_DECL
# define YY_DECL                                                 \
  yy::parser::token_type					 \
  yyFlexLexer::yylex(yy::parser::semantic_type* valp,		 \
		     yy::parser::location_type* locp, UParser& uparser)

//! Control class for a flex-scanner
/*! It has a pointer to the uparser in which it is contained
 */
class UFlexer
  : public yyFlexLexer
{
public:
  UFlexer(void *_uparser);
  void* UFlexer::get_uparser() const;
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
  // friend int yylex(yy::parser::semantic_type *lvalp, void *compiler);
  UParser();

  /// Parse the command from a stream.
  /// (this is how flex C++ handles it, no choice).
  int process(ubyte* command, int length, UConnection* connection_);

  UConnection   *connection;
  UCommand_TREE *commandTree;
  bool          binaryCommand;

  yy::parser::token_type scan(yy::parser::semantic_type* val,
			      yy::parser::location_type* loc);

  void error (const yy::parser::location_type& l, const std::string& msg);

private:
  // The scanner used in this parser (it is a flex-scanner)
  UFlexer uflexer;
  int result;
};


// Important! These are the "shortcuts" which you can use in your
// ".l"- and ".y"-files to access the corresponding uparser-object!
// # define flex_uparser (*static_cast<UParser *> (static_cast<UFlexer *>(this)->get_uparser()))

#endif

// Local Variables:
// mode: C++
// End:
