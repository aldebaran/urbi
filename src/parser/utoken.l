%{
/* \file utoken.l
 *******************************************************************************

 File: utoken.l\n
 Definition of scanner used by the UParser object.
 This scanned is defined with flex, using option c++.

 This file is part of
 %URBI, version 1.0\n
 (c) Jean-Christophe Baillie, 2004.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <cstdlib>
#include <cstring>
#ifndef NDEBUG
# include <cstdlib> // for getenv
#endif

#include <stack>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "parser/uparser.hh"
// Shorter access to the tokens.
typedef yy::parser::token token;


// Safe conversion to a numeric value.
template <typename Out, typename In>
Out
lexical_cast (UParser& up, const yy::location& loc, In t)
{
  try
  {
    return boost::lexical_cast<Out>(t);
  }
  catch (boost::bad_lexical_cast)
  {
    up.error(loc,
	     (boost::format ("invalid numerical litteral: %1%") % t).str());
    return 0;
  }
}


// At each match, adjust the end cursor.
#define YY_USER_ACTION locp->columns(yyleng);

// To avoid differences between versions of Bison, set by hand the
// initial columns when advancing lines.
#define LINES(Loc)				\
 do {						\
   (Loc)->lines();				\
   (Loc)->end.column = 1;			\
 } while (0)

// Otherwise Flex returns an int instead of an enum.
#define yyterminate() return token::TOK_EOF

/*----------------------------------.
| Sugar to return semantic values.  |
`----------------------------------*/

#define RETURN_VALUE(Token, Field, Value)	\
 do {						\
   valp->Field = Value;				\
   return token::Token;				\
 } while (0)



// For ",", ";", "|", and "&" (i.e., COMMA, SEMICOLON, PIPE, and AND).
#define RETURN_FLAVOR(Symbol, Flavor)					\
  RETURN_VALUE(TOK_ ## Symbol, flavor, ast::flavor_ ## Flavor)

/// Return an fval of type Token with Value.
#define RETURN_FVAL(Token, Value)		\
  RETURN_VALUE(Token, fval, Value)

/// Return an ival of type Token with Value.
#define RETURN_IVAL(Token, Value)		\
  RETURN_VALUE(Token, ival, Value)

// Store yytext in valp->str, and return Token.
#define RETURN_STR(Token)				\
  RETURN_VALUE(Token, str, new std::string(yytext))

// Store yytext in valp->symbol, and return Token.
#define RETURN_SYMBOL(Token)					\
  RETURN_VALUE(Token, symbol, new libport::Symbol(yytext))

// Return an operator whose string representation is kept as a Symbol.
#define RETURN_OP(Token)			\
  RETURN_SYMBOL(TOK_ ## Token)

// UnitSize is the number of letters used for the unit.
#define RETURN_TIME_VALUE(UnitSize, Scale)				\
  do {									\
    yytext[yyleng - UnitSize] = 0;					\
    RETURN_VALUE(TIME_VALUE, fval,					\
		 boost::lexical_cast<float>(yytext) * (Scale));		\
  } while (0)

#define RETURN_FLAVORED_KEYWORD(Token)					\
  switch (yytext[strlen(yytext) - 1])					\
  {									\
    case '|': RETURN_FLAVOR(Token, pipe);				\
    case ',': RETURN_FLAVOR(Token, comma);				\
    case '&': RETURN_FLAVOR(Token, and);				\
    default: RETURN_FLAVOR(Token, semicolon);				\
  }

// We matched an obsolete token, New is now preferered.  Unput it so
// that the regular scanning rule of New is run.
#define DEPRECATED(New)						\
  do {								\
    up.warn(*locp,						\
	    std::string("`") + yytext + "' is deprecated. "	\
	    "Use `" New "' instead.");				\
    for (int i = strlen(New) - 1; 0 <= i; --i)			\
      unput(New[i]);						\
  } while(0);

%}
%option noyywrap debug

%x SC_C_COMMENT SC_STRING
%s SC_UCONSOLE

BLANKS   [ \t]+
EOL      \n|\r|\n\r|\r\n

ID       [a-zA-Z_][_A-Za-z0-9]*

NATURAL  [0-9]+
FRACTION (\.{NATURAL})|({NATURAL}(\.[0-9]*)?)
EXPONENT [eE][-+]?[0-9]+
FLOAT    {FRACTION}{EXPONENT}?

NUMBER   {NATURAL}|{FLOAT}

FLAVOR   [;|,&]?

%%
%{
#ifndef NDEBUG
  static bool first_run = true;
  if (first_run)
    yy_flex_debug = !!getenv("FLEX_DEBUG");
#endif

  locp->step();

#ifndef NDEBUG
  // The default start condition: the one replacing INITIAL.
  static int sc_default = -1;
  if (sc_default == -1)
  {
    if (getenv ("UCONSOLE_MODE"))
    {
      std::cerr << "Message from the scanner: UCONSOLE_MODE enabled.\n";
      sc_default = SC_UCONSOLE;
    }
    else
      sc_default = INITIAL;
    BEGIN sc_default;
  }
#endif

  static std::stack<yy::location> synclines;
%}

  /*-----------------------------------.
  | Internal rules for generated code. |
  `-----------------------------------*/
^"#"{BLANKS}?("line"|"push"){BLANKS}{NATURAL}({BLANKS}"\"".*"\"")?{BLANKS}?{EOL}	{
# define SKIP_SPACES()				\
  for (++yytext; isspace (*yytext); ++yytext)	\
    continue;

  SKIP_SPACES();
  bool push = yytext[0] == 'p';
  yytext += 4; // line|push.

  SKIP_SPACES();
  char* line = yytext;
  yytext += strspn(yytext, "0123456789");
  *yytext = 0;
  ++yytext;

  char* file = strchr (yytext, '"');
  if (file)
  {
    ++file;
    *strchr (file, '"') = 0;
  }

  if (push)
    synclines.push(*locp);

  locp->initialize (!file || STREQ(file, "-") ? 0 : new libport::Symbol (file));
  locp->lines (lexical_cast<int>(up, *locp, line) - 1);
  locp->step ();
}

^"#"{BLANKS}?"pop"{BLANKS}?{EOL}     *locp = synclines.top(); synclines.pop();


  /*--------------------------------------------------------------.
  | Console mode: skip the [01234567] lines for expected output.  |
  `--------------------------------------------------------------*/

<SC_UCONSOLE>{
  ^\[[0-9X]{8,}.*\]\ .* /* ignore the "[timestamp:tags] whatever" messages */;
  ^\[:].*               /* ignore the "[:] whatever" messages */;
}


  /*------------.
  | The rules.  |
  `------------*/

  /*  FIXME: This is really sick. */
"+report"     RETURN_IVAL(FLAG, 1);
"+error"      RETURN_IVAL(FLAG, 2);
"+end"        RETURN_IVAL(FLAG, 3);
"+begin"      RETURN_IVAL(FLAG, 4);
"+top"        RETURN_IVAL(FLAG, 7);
"+bg"         RETURN_IVAL(FLAG, 10);
"+freeze"     RETURN_IVAL(FLAG, 11);
"+stop"       RETURN_IVAL(FLAG, 12);

{NUMBER}"d"   RETURN_TIME_VALUE(1, 1000*60*60*24);
{NUMBER}"h"   RETURN_TIME_VALUE(1, 1000*60*60);
{NUMBER}"m"   RETURN_TIME_VALUE(1, 1000*60);
{NUMBER}"s"   RETURN_TIME_VALUE(1, 1000);
{NUMBER}"ms"  RETURN_TIME_VALUE(2, 1);

{NATURAL}     RETURN_IVAL(INTEGER, lexical_cast<int>  (up, *locp, yytext));
{FLOAT}       RETURN_FVAL(FLOAT,   lexical_cast<float>(up, *locp, yytext));

\"            valp->str = new std::string; BEGIN SC_STRING;


"block"      |
"disconnect" |
"freeze"     |
"killall"    |
"stop"       |
"unblock"    |
"unfreeze"   RETURN_SYMBOL (OPERATOR_ID);

"delete"  |
"info"    |
"undef"   RETURN_SYMBOL (OPERATOR_VAR);

 /*
   FIXME: Surprisingly enough, these are strings, not symbols,
   i.e., they pretend to have quotes around them.  This is because
   they are used as properties, which are "generalized" as strings.
   It is some sort of lexical sugar that allows to write << add >>,
   instead of << "add" >>.  Needless to say that it conflicts badly
   with the rest of the language.

   "add"     |
   "cancel"  |
   "discard" |
   "mix"     |
   "normal"  |
   "queue"   RETURN_STR (STRING);
 */

"external"  |
"internal"  RETURN_SYMBOL (BINDER);

"commands"        |
"connections"     |
"debugoff"        |
"debugon"         |
"devices"         |
"events"          |
"functions"       |
"motoroff"        |
"motoron"         |
"ping"            |
"quit"            |
"reset"           |
"runningcommands" |
"stopall"         |
"strict"          |
"taglist"         |
"undefall"        |
"unstrict"        |
"uservars"        |
"vars"            RETURN_SYMBOL (OPERATOR);

"BIN"           return token::TOK_BIN;
"addgroup"      return token::TOK_ADDGROUP;
"alias"         return token::TOK_ALIAS;
"bin"           return token::TOK_BIN;
"break"         return token::TOK_BREAK;
"class"         return token::TOK_CLASS;
"def"           return token::TOK_DEF;
"delgroup"      return token::TOK_DELGROUP;
"do"		return token::TOK_DO;
"else"          return token::TOK_ELSE;
"emit"          return token::TOK_EMIT;
"event"         return token::TOK_EVENT;
"every"         return token::TOK_EVERY;
"foreach"       DEPRECATED("for");
"freezeif"      return token::TOK_FREEZEIF;
"from"          return token::TOK_FROM;
"function"      return token::TOK_FUNCTION;
"group"         return token::TOK_GROUP;
"if"            return token::TOK_IF;
"in"            return token::TOK_IN;

"loopn"         DEPRECATED("for");
"new"           return token::TOK_NEW;
"noop"          return token::TOK_NOOP;
"object"        return token::TOK_OBJECT;
"onleave"       return token::TOK_ONLEAVE;
"return"        return token::TOK_RETURN;
"static"        return token::TOK_STATIC;
"stopif"        return token::TOK_STOPIF;
"timeout"       return token::TOK_TIMEOUT;
"unalias"       return token::TOK_UNALIAS;
"var"           return token::TOK_VAR;
"'in"           return token::TOK_VARIN;
"'out"          return token::TOK_VAROUT;
"waituntil"     return token::TOK_WAITUNTIL;
"whenever"      return token::TOK_WHENEVER;

"for"{FLAVOR}   RETURN_FLAVORED_KEYWORD(FOR);
"loop"{FLAVOR}  RETURN_FLAVORED_KEYWORD(LOOP);
"while"{FLAVOR} RETURN_FLAVORED_KEYWORD(WHILE);
"at"{FLAVOR}    RETURN_FLAVORED_KEYWORD(AT);

"!"             RETURN_OP(BANG);
"%"             RETURN_OP(PERCENT);
"*"             RETURN_OP(STAR);
"*="            RETURN_OP(STAR_ASSIGN);
"+"             RETURN_OP(PLUS);
"+="            RETURN_OP(PLUS_ASSIGN);
"++"            RETURN_OP(PLUS_PLUS);
"-"             RETURN_OP(MINUS);
"-="            RETURN_OP(MINUS_ASSIGN);
"--"            RETURN_OP(MINUS_MINUS);
"/"             RETURN_OP(SLASH);
"/="            RETURN_OP(SLASH_ASSIGN);

"<"             RETURN_OP(LTH);
"<="            RETURN_OP(LEQ);
"<<"            RETURN_OP(LT_LT);
"=="            RETURN_OP(EQU);
"=~="           RETURN_OP(DEQ);
">"             RETURN_OP(GTH);
">="            RETURN_OP(GEQ);
">>"            RETURN_OP(GT_GT);
"^"             RETURN_OP(CARET);
"**"            RETURN_OP(STAR_STAR);
"&&"            RETURN_OP(LAND);
"||"            RETURN_OP(LOR);

"="             return token::TOK_ASSIGN;
"!="            RETURN_OP(NEQ);
"$"             return token::TOK_DOLLAR;
"%="            return token::TOK_PEQ;

  /*
   "'"             return token::TOK_DERIV;
   "''"            return token::TOK_DERIV2;
   "'d"            return token::TOK_TRUEDERIV;
   "'dd"           return token::TOK_TRUEDERIV2;
   "'e"            return token::TOK_VARERROR;
   "'n"            return token::TOK_NORM;
   */
"("             return token::TOK_LPAREN;
")"             return token::TOK_RPAREN;
"->"            return token::TOK_DIR;
"."             return token::TOK_POINT;
":"             return token::TOK_COLON;
"::"            return token::TOK_DOUBLECOLON;
"["             return token::TOK_LBRACKET;
"]"             return token::TOK_RBRACKET;
"{"             return token::TOK_LBRACE;
"}"             return token::TOK_RBRACE;
"~"             return token::TOK_TILDE;
"~="            return token::TOK_REQ;

"&"             RETURN_FLAVOR(AND, and);
"|"             RETURN_FLAVOR(PIPE, pipe);
";"             RETURN_FLAVOR(SEMICOLON, semicolon);
","             RETURN_FLAVOR(COMMA, comma);


{ID}     RETURN_VALUE(IDENTIFIER, symbol, new libport::Symbol(yytext));


    /*----------.
    | Strings.  |
    `----------*/

<SC_STRING>{ /* Handling of the strings.  Initial " is eaten. */
     \" {
       BEGIN sc_default;
       return token::STRING;
     }

     \\[0-7]{3}		{
       long c = strtol (yytext + 1, 0, 8);
       if (c > 255)
	 up.error(*locp, "invalid escape");
       else
	 *valp->str += c;
     }

     \\x[0-9a-fA-F]{2}	{
       *valp->str += strtol (yytext + 2, 0, 16);
     }

     \\a	*valp->str += '\a';
     \\b	*valp->str += '\b';
     \\f	*valp->str += '\f';
     \\n	*valp->str += '\n';
     \\r	*valp->str += '\r';
     \\t	*valp->str += '\t';
     \\v	*valp->str += '\v';
     \\[\\""]   *valp->str += yytext[1];
     \\.	up.error (*locp, "invalid escape");


     [^\"\n\r\\]+      valp->str->append (yytext, yyleng);

     {EOL}+    locp->lines (yyleng); valp->str->append (yyleng, '\n');

     <<EOF>> {
       up.error (*locp, "unexpected end of file in a string");
       BEGIN sc_default;
       return token::STRING;
     }
}



    /*----------------------------.
    | White spaces and comments.  |
    `----------------------------*/

("//"|"#").*   locp->step (); // One-line comments.
"/*"           BEGIN SC_C_COMMENT;

{BLANKS}       locp->step();
{EOL}          LINES(locp); locp->step();

<SC_C_COMMENT>{ /* Comments. */
  "*/" { /* End of the comment. */
    locp->step ();
    BEGIN sc_default;
  }

  [^*/\n\r]+	;
  {EOL}	        LINES(locp);
  .             /* Stray `*' and `/'. */;

  <<EOF>> {
    up.error(*locp, std::string ("unexpected end of file in a comment"));
    locp->step ();
    BEGIN sc_default;
  }
}

<*>{
   .      {
	     up.error(*locp, std::string ("invalid character: ") + yytext);
	     locp->step ();
	  }
}

%%

// Local Variables:
// mode: C++
// End:
