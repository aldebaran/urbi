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

#include <strstream> 
#include <sstream>
#include <algorithm>
#include <string>
#ifdef _MSC_VER
#ifdef min
#undef min
#endif
#endif
#include "utypes.h"
#undef IN
#include "bison/FlexLexer.h"
#include "bison/location.hh"

// FIXME: When Bison is fixed, replace yy::parser::token::yytokentype
// by yy::parser::token_type.
#undef  YY_DECL
#define YY_DECL                                                 \
  yy::parser::token::yytokentype 			 	\
  yyFlexLexer::yylex(yy::parser::semantic_type* valp,		\
                     yy::location* locp, UParser& uparser)

// Parse function of 'bison' is defined externally
extern char errorMessage[1024];
extern UString** globalDelete;

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
 // friend int yylex(yy::parser::semantic_type *lvalp, void *compiler);

  UParser() : uflexer(this) {}

  // Parse the command from a stream (this is how flex C++ handles it, 
  // no choice).
  int process(ubyte* command, int length, UConnection* connection_) 
  {    
    connection = connection_;
    commandTree = 0;
    result = 0;   

    std::istrstream * mem_buff = new std::istrstream((char*)command, length);    
    if (!mem_buff) return -1;
       
    std::istream* mem_input = new std::istream(mem_buff->rdbuf());
    if (!mem_input) {
      delete mem_buff;
      return -1;
    }
       
    uflexer.switch_streams(&(*mem_input), 0);// Tells flex the right stream
    binaryCommand = false;   
    
    yy::parser p(*this);   
    result = p.parse();      

    delete mem_input;
    delete mem_buff;   

    return result;
  }

  UConnection   *connection;
  UCommand_TREE *commandTree;
  bool          binaryCommand;

  yy::parser::token::yytokentype scan(yy::parser::semantic_type* val,
                                      yy::location* loc) { 
     return uflexer.yylex(val,loc,*this); 
  }

  void error (const yy::location& l, const std::string& msg)
  {
      std::ostringstream sstr;

      sstr << "!!! " << l << ": " << msg << "\n" << std::ends;
      strncpy(errorMessage, sstr.str().c_str(),
              std::min(sizeof (errorMessage), sstr.str().size()));  
	
  }

private:
  // The scanner used in this parser (it is a flex-scanner)
  UFlexer uflexer;
  int result;
};


// Important! These are the "shortcuts" which you can use in your
// ".l"- and ".y"-files to access the corresponding uparser-object!
// #define flex_uparser (*static_cast<UParser *> (static_cast<UFlexer *>(this)->get_uparser()))

#endif
