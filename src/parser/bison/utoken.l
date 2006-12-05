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
// Shorter access to the tokens.
typedef yy::parser::token token;

#define YY_USER_ACTION locp->columns(yyleng);

// Otherwise Flex returns an int instead of an enum.
#define yyterminate() return token::UEOF

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


{NUMBER}"d"   RETURN_TIMEVALUE(60000*60*24);
{NUMBER}"h"   RETURN_TIMEVALUE(60000*60);
{NUMBER}"m"   RETURN_TIMEVALUE(60000);
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


"stop"       |
"unblock"    |
"block"      |
"unfreeze"   |
"freeze"     |
"killall"    |
"disconnect"   RETURN_STR_TOKEN (OPERATOR_ID);

"info"  |
"undef" |
"delete"       RETURN_STR_TOKEN (OPERATOR_VAR);

"mix"     |
"add"     |
"normal"  |
"cancel"  |
"discard" |
"queue"       RETURN_STR_TOKEN (STRING);

"external" |
"internal"     RETURN_STR_TOKEN (BINDER);

"motoron"     |
"motoroff"    |
"strict"      |
"unstrict"    |
"ping"        |
"devices"     |
"connections" |
"vars"        |
"events"      |
"uservars"    |
"undefall"    |
"reset"       |
"commands"    |
"debugon"     |
"debugoff"    |
"quit"        |
"stopall"     |
"reboot"      |
"shutdown"      RETURN_STR_TOKEN (OPERATOR);

"if"            return token::IF;
"else"          return token::ELSE;
"from"          return token::FROM;
"var"           return token::VAR;
"object"        return token::OBJECT;
"function"      return token::FUNCTION;
"class"         return token::CLASS;
"subclass"      return token::SUBCLASS;
"new"           return token::NEW;
"event"         return token::EVENT;
"while"         return token::WHILE;
"timeout"       return token::TIMEOUT;
"stopif"        return token::STOPIF;
"freezeif"      return token::FREEZEIF;
"every"         return token::EVERY;
"at"            return token::AT;
"static"        return token::STATIC;
"whenever"      return token::WHENEVER;
"onleave"       return token::ONLEAVE;
"for"           return token::FOR;
"foreach"       return token::FOREACH;
"in"            return token::IN;
"loop"          return token::LOOP;
"loopn"         return token::LOOPN;
"group"         return token::GROUP;
"addgroup"      return token::ADDGROUP;
"delgroup"      return token::DELGROUP;
"alias"         return token::ALIAS;
"inherit"       return token::INHERIT;
"disinherit"    return token::DISINHERIT;
"unalias"       return token::UNALIAS;
"echo"          return token::UECHO;
"wait"          return token::WAIT;
"waituntil"     return token::WAITUNTIL;
"emit"          return token::EMIT;
"def"           return token::DEF;
"return"        return token::RETURN;
"BIN"           return token::BIN;
"bin"           return token::BIN;
"noop"          return token::NOOP;
"true"          return token::TRUECONST;
"false"         return token::FALSECONST;
"copy"          return token::COPY;
"only"          return token::ONLY;
"list"          return token::GROUPLIST;

"'n"          return token::NORM;
"'e"          return token::VARERROR;
"'d"          return token::TRUEDERIV;
"'dd"         return token::TRUEDERIV2;
"''"          return token::DERIV2;
"'"           return token::DERIV;
"&&"          return token::ANDOPERATOR;
"||"          return token::OROPERATOR;
";"           return token::SEMICOLON;
"::"          return token::DOUBLECOLON;
":"           return token::COLON;
","           return token::COMMA;
"&"           return token::AND;
"|"           return token::PIPE;
"!"           return token::BANG;
"--"          return token::MINUSMINUS;
"++"          return token::PLUSPLUS;
"+="          return token::PLUSASSIGN;
"-="          return token::MINUSASSIGN;
"*"           return token::MULT;
"/"           return token::DIV;
"+"           return token::PLUS;
"^"           return token::EXP;
"-"           return token::MINUS;
"="           return token::ASSIGN;
"=="          return token::EQ;
"~="          return token::REQ;
"=~="         return token::DEQ;
"%="          return token::PEQ;
"!="          return token::NE;
">"           return token::GT;
"<"           return token::LT;
"<="          return token::LE;
">="          return token::GE;
"->"          return token::DIR;
"~"           return token::TILDE;
"%"           return token::PERCENT;
"@"           return token::AROBASE;
"("           return token::LPAREN;
")"           return token::RPAREN;
"["           return token::LSBRACKET;
"]"           return token::RSBRACKET;
"{"           return token::LBRACKET;
"}"           return token::RBRACKET;
"$"           return token::DOLLAR;
"."           return token::POINT;




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
{EOL}+         locp->lines(yyleng); locp->step();
.              {
		 uparser.error(*locp, std::string ("invalid character: ")
				      + yytext);
		 locp->step ();
	       }

<SC_C_COMMENT>{ /* Comments. */
  "*/" { /* End of the comment. */
    locp->step ();
    BEGIN INITIAL;
  }

  [^*/\n\r]+  	;
  {EOL}+	locp->lines (yyleng);
  .             /* Stray `*' and `/'. */;

  <<EOF>> {
    uparser.error(*locp, std::string ("unexpected end of file in a comment"));
    locp->step ();
    BEGIN INITIAL;
  }
}

%%

// Local Variables:
// mode: C++
// End:
