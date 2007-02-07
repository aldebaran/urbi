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

#include "parser/uparser.hh"

#ifdef ECHO
// libport/compiler.hh has a #define ECHO that conflicts with flex.
# undef ECHO
#endif
// Shorter access to the tokens.
typedef yy::parser::token token;

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

// Be bulletproof when memory allocation fails.
#define SET_VALP_STR(Value)			\
do {						\
  valp->str = new UString(Value);		\
  if (valp->str->str() == 0)			\
  {						\
    delete valp->str;				\
    valp->str = 0;				\
  }						\
} while (0)

// Store yytext in valp->str, and return Token.
#define RETURN_STR_TOKEN(Token)			\
do {						\
  SET_VALP_STR(yytext);				\
  return token::Token;				\
} while (0);

#define RETURN_VAL_TOKEN(Token, Value)		\
do {						\
  valp->val = new ufloat(Value);		\
  return token::Token;				\
} while (0)


#define RETURN_NUM(Value)      RETURN_VAL_TOKEN(NUM, Value)
#define RETURN_FLAG(Value)     RETURN_VAL_TOKEN(FLAG, Value)
#define RETURN_FLAGTIME(Value) RETURN_VAL_TOKEN(FLAGTIME, Value)


#define RETURN_TIMEVALUE(Scale)				\
do {							\
  yytext[yyleng-1]=0;					\
  valp->val = new ufloat(atof(yytext) * (Scale));	\
  return token::TIMEVALUE;				\
} while (0)

%}
%option noyywrap

%x SC_C_COMMENT

BLANKS   [ \t]+
EOL      \n|\r|\n\r|\r\n
ID       [a-zA-Z_][_A-Za-z0-9]*
NUMBER   (\.[0-9]+)|([0-9]+(\.[0-9]*)?)
STRING   \"([^\"]|"\\\"")*\"

%%
%{
  locp->step();
%}

  /*------------.
  | The rules.  |
  `------------*/

"inf"         RETURN_NUM(UINFINITY);
"pi"          RETURN_NUM(3.14159265358979323846264338327950288);
"off"         RETURN_NUM(0);
"switch"      RETURN_NUM(-1);
"on"          RETURN_NUM(1);


"+report"     RETURN_FLAG(1);
"+error"      RETURN_FLAG(2);
"+end"        RETURN_FLAG(3);
"+begin"      RETURN_FLAG(4);
"+top"        RETURN_FLAG(7);
"+bg"         RETURN_FLAG(10);

"+timeout"    RETURN_VAL_TOKEN(FLAGTIME, 5);

"+stop"       RETURN_VAL_TOKEN(FLAGTEST, 7);
"+freeze"     RETURN_VAL_TOKEN(FLAGTEST, 8);

"+connection" RETURN_VAL_TOKEN(FLAGID, 9);


{NUMBER}"d"   RETURN_TIMEVALUE(1000*60*60*24);
{NUMBER}"h"   RETURN_TIMEVALUE(1000*60*60);
{NUMBER}"m"   RETURN_TIMEVALUE(1000*60);
{NUMBER}"s"   RETURN_TIMEVALUE(1000);
{NUMBER}"ms"  yytext[yyleng-2]=0; RETURN_TIMEVALUE(1);

{NUMBER}      { valp->val = new ufloat(atof(yytext));
		return token::NUM; }

{STRING}      { /* Remove ending ". */
		yytext[yyleng - 1] = 0;
		SET_VALP_STR(yytext+1);
		if (valp->str)
		  valp->str->un_armor();
		return token::STRING;
	      }


"block"      |
"disconnect" |
"freeze"     |
"killall"    |
"stop"       |
"unblock"    |
"unfreeze"   RETURN_STR_TOKEN (OPERATOR_ID);

"delete"  |
"info"    |
"undef"   RETURN_STR_TOKEN (OPERATOR_VAR);

"add"     |
"cancel"  |
"discard" |
"mix"     |
"normal"  |
"queue"   RETURN_STR_TOKEN (STRING);

"external"  |
"internal"  RETURN_STR_TOKEN (BINDER);

"commands"    |
"connections" |
"debugoff"    |
"debugon"     |
"devices"     |
"events"      |
"motoroff"    |
"motoron"     |
"ping"        |
"quit"        |
"reboot"      |
"reset"       |
"shutdown"    |
"stopall"     |
"strict"      |
"undefall"    |
"unstrict"    |
"uservars"    |
"functions"   |
"vars"        RETURN_STR_TOKEN (OPERATOR);

"BIN"           return token::TOK_BIN;
"addgroup"      return token::TOK_ADDGROUP;
"alias"         return token::TOK_ALIAS;
"at"            return token::TOK_AT;
"bin"           return token::TOK_BIN;
"class"         return token::TOK_CLASS;
"copy"          return token::TOK_COPY;
"def"           return token::TOK_DEF;
"delgroup"      return token::TOK_DELGROUP;
"disinherits"   return token::TOK_DISINHERITS;
"echo"          return token::TOK_ECHO;
"else"          return token::TOK_ELSE;
"emit"          return token::TOK_EMIT;
"event"         return token::TOK_EVENT;
"every"         return token::TOK_EVERY;
"false"         return token::TOK_FALSECONST;
"for"           return token::TOK_FOR;
"foreach"       return token::TOK_FOREACH;
"freezeif"      return token::TOK_FREEZEIF;
"from"          return token::TOK_FROM;
"function"      return token::TOK_FUNCTION;
"group"         return token::TOK_GROUP;
"if"            return token::TOK_IF;
"in"            return token::TOK_IN;
"inherits"      return token::TOK_INHERITS;
"list"          return token::TOK_GROUPLIST;
"loop"          return token::TOK_LOOP;
"loopn"         return token::TOK_LOOPN;
"new"           return token::TOK_NEW;
"noop"          return token::TOK_NOOP;
"object"        return token::TOK_OBJECT;
"onleave"       return token::TOK_ONLEAVE;
"only"          return token::TOK_ONLY;
"return"        return token::TOK_RETURN;
"static"        return token::TOK_STATIC;
"stopif"        return token::TOK_STOPIF;
"subclass"      return token::TOK_SUBCLASS;
"timeout"       return token::TOK_TIMEOUT;
"true"          return token::TOK_TRUECONST;
"unalias"       return token::TOK_UNALIAS;
"var"           return token::TOK_VAR;
"'in"           return token::TOK_VARIN;
"'out"          return token::TOK_VAROUT;
"wait"          return token::TOK_WAIT;
"waituntil"     return token::TOK_WAITUNTIL;
"whenever"      return token::TOK_WHENEVER;
"while"         return token::TOK_WHILE;

"!"             return token::TOK_BANG;
"!="            return token::TOK_NE;
"$"             return token::TOK_DOLLAR;
"%"             return token::TOK_PERCENT;
"%="            return token::TOK_PEQ;
"&"             return token::TOK_AND;
"&&"            return token::TOK_ANDOPERATOR;
"'"             return token::TOK_DERIV;
"''"            return token::TOK_DERIV2;
"'d"            return token::TOK_TRUEDERIV;
"'dd"           return token::TOK_TRUEDERIV2;
"'e"            return token::TOK_VARERROR;
"'n"            return token::TOK_NORM;
"("             return token::TOK_LPAREN;
")"             return token::TOK_RPAREN;
"*"             return token::TOK_MULT;
"+"             return token::TOK_PLUS;
"++"            return token::TOK_PLUSPLUS;
"+="            return token::TOK_PLUSASSIGN;
","             return token::TOK_COMMA;
"-"             return token::TOK_MINUS;
"--"            return token::TOK_MINUSMINUS;
"-="            return token::TOK_MINUSASSIGN;
"->"            return token::TOK_DIR;
"."             return token::TOK_POINT;
"/"             return token::TOK_DIV;
":"             return token::TOK_COLON;
"::"            return token::TOK_DOUBLECOLON;
";"             return token::TOK_SEMICOLON;
"<"             return token::TOK_LT;
"<="            return token::TOK_LE;
"="             return token::TOK_ASSIGN;
"=="            return token::TOK_EQ;
"=~="           return token::TOK_DEQ;
">"             return token::TOK_GT;
">="            return token::TOK_GE;
"@"             return token::TOK_AROBASE;
"["             return token::TOK_LSBRACKET;
"]"             return token::TOK_RSBRACKET;
"^"             return token::TOK_EXP;
"{"             return token::TOK_LBRACKET;
"|"             return token::TOK_PIPE;
"||"            return token::TOK_OROPERATOR;
"}"             return token::TOK_RBRACKET;
"~"             return token::TOK_TILDE;
"~="            return token::TOK_REQ;


{ID}          RETURN_STR_TOKEN (IDENTIFIER);


{ID}"."{ID}   { char* tmp_position = strchr(yytext,'.');
		tmp_position[0] = 0;
		valp->structure.device = new UString(yytext);
		if (valp->structure.device->str() == 0)
		{
		  delete valp->structure.device;
		  valp->structure.device = 0;
		}
		valp->structure.id = new UString(tmp_position+1);
		if (valp->structure.id->str() == 0)
		{
		  delete valp->structure.id;
		  valp->structure.id = 0;
		}
		valp->structure.rooted = false;

		return token::STRUCT; }


{ID}"."{ID}("."{ID})+   RETURN_STR_TOKEN(TAG);

    /*----------------------------.
    | White spaces and comments.  |
    `----------------------------*/

("//"|"#").*   locp->step (); // One-line comments.
"/*"           BEGIN SC_C_COMMENT;

{BLANKS}       locp->step();
{EOL}          LINES(locp); locp->step();
.              {
		 up.error(*locp, std::string ("invalid character: ")
			  + yytext);
		 locp->step ();
	       }

<SC_C_COMMENT>{ /* Comments. */
  "*/" { /* End of the comment. */
    locp->step ();
    BEGIN INITIAL;
  }

  [^*/\n\r]+  	;
  {EOL}	        LINES(locp);
  .             /* Stray `*' and `/'. */;

  <<EOF>> {
    up.error(*locp, std::string ("unexpected end of file in a comment"));
    locp->step ();
    BEGIN INITIAL;
  }
}

%%

// Local Variables:
// mode: C++
// End:
