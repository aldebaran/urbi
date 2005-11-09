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
#define UPARSER_H_DEFINED


#include <iostream>
#include <string>

#include <strstream> 
#include <iostream.h>
#include <string.h>

#include "bison/FlexLexer.h"
#include "../utypes.h"
#include "../ustring.h"

using namespace std;

#undef  YY_DECL
#define YY_DECL int yyFlexLexer::yylex(YYSTYPE* lvalp)

// Parse function of 'bison' is defined externally
extern "C" int yyparse(void *);
extern char errorMessage[1024];
class UConnection;
class UCommand;

struct UDefine {
  UString *name;
  UString *value;
};

// The error function that 'bison' calls
inline void yyerror(char const *what_error) { 

  strcpy(errorMessage,"!!! "); 
  strncat(errorMessage,what_error,1019);
  errorMessage[1022] = 0; // Just make sure it ends...
  strcat(errorMessage,"\n");
}


//! Control class for a flex-scanner
/*! It has a pointer to the uparser in which it is contained
 */
class UFlexer 
  : public yyFlexLexer
{
public:
  UFlexer(void *_uparser) : uparser(_uparser) {}

  void * get_uparser() const { return uparser; }

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
  friend int yylex(YYSTYPE *lvalp, void *compiler);

  UParser() : uflexer(this) {}

  // Parse the command from a stream (this is how flex C++ handles it, 
  // no choice).
  int process(ubyte* command, int length, UConnection* connection_) 
  {    
    connection = connection_;
    commandTree = 0;
    result = 0;   

    istrstream * mem_buff = new istrstream((char*)command, length);    
    if (!mem_buff) return -1;
       
    istream* mem_input = new istream(mem_buff->rdbuf());
    if (!mem_input) {
      delete mem_buff;
      return -1;
    }
       
    uflexer.switch_streams(&(*mem_input), 0);// Tells flex the right stream
    binaryCommand = false;   
       
    result =  yyparse((void *) this);  

    delete mem_input;
    delete mem_buff;   

    return result;
  }

  UConnection   *connection;
  UCommand_TREE *commandTree;
  bool          binaryCommand;

private:
  // The scanner used in this parser (it is a flex-scanner)
  UFlexer uflexer;
  int result;

  int scan(YYSTYPE *lvalp) { return uflexer.yylex(lvalp); }
};


//! Directs the call from 'bison' to the scanner in the right parser
inline int yylex(YYSTYPE *lvalp, void *_uparser)
{
  UParser &the_parser = *static_cast<UParser *>(_uparser);
  return the_parser.scan(lvalp);
}


// Definitions for 'flex' and 'bison'

#define yywrap() 1
#define YY_SKIP_YYWRAP
#define YYPARSE_PARAM parm
#define YYLEX_PARAM parm

// Important! These are the "shortcuts" which you can use in your
// ".l"- and ".y"-files to access the corresponding uparser-object!
#define flex_uparser (*static_cast<UParser *> (static_cast<UFlexer *>(this)->get_uparser()))
#define bison_uparser (*static_cast<UParser *> (parm))

#endif
