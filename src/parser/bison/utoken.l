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
#define RETURN_FLAVOR(Symbol)					\
  RETURN_VALUE(TOK_ ## Symbol, flavor, Flavorable::U ## Symbol)

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

#define RETURN_TIME_VALUE(Scale)				\
  do {								\
    yytext[yyleng-1]=0;						\
    RETURN_VALUE(TIME_VALUE, fval, atof(yytext) * (Scale));	\
  } while (0)



%}
%option noyywrap

%x SC_C_COMMENT SC_STRING

BLANKS   [ \t]+
EOL      \n|\r|\n\r|\r\n
ID       [a-zA-Z_][_A-Za-z0-9]*
INTEGER  [0-9]+
FLOAT    \.[0-9]+|[0-9]+\.[0-9]*
NUMBER   {INTEGER}|{FLOAT}
STRING   \"([^\"]|"\\\"")*\"

%%
%{
  locp->step();
%}

  /*------------.
  | The rules.  |
  `------------*/

"inf"         RETURN_FVAL(FLOAT, UINFINITY);
"pi"          RETURN_FVAL(FLOAT, 3.14159265358979323846264338327950288);
"off"         RETURN_FVAL(FLOAT, 0);
"switch"      RETURN_FVAL(FLOAT, -1);
"on"          RETURN_FVAL(FLOAT, 1);

  /*  FIXME: This is really sick. */
"+report"     RETURN_IVAL(FLAG, 1);
"+error"      RETURN_IVAL(FLAG, 2);
"+end"        RETURN_IVAL(FLAG, 3);
"+begin"      RETURN_IVAL(FLAG, 4);
"+top"        RETURN_IVAL(FLAG, 7);
"+bg"         RETURN_IVAL(FLAG, 10);
"+freeze"     RETURN_IVAL(FLAG, 11);
"+stop"       RETURN_IVAL(FLAG, 12);

{NUMBER}"d"   RETURN_TIME_VALUE(1000*60*60*24);
{NUMBER}"h"   RETURN_TIME_VALUE(1000*60*60);
{NUMBER}"m"   RETURN_TIME_VALUE(1000*60);
{NUMBER}"s"   RETURN_TIME_VALUE(1000);
{NUMBER}"ms"  yytext[yyleng-2]=0; RETURN_TIME_VALUE(1);

  /* FIXME: atof and atoi are no good. */
{FLOAT}       { RETURN_FVAL(FLOAT,   atof(yytext)); }
{INTEGER}     { RETURN_IVAL(INTEGER, atoi(yytext)); }

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

"add"     |
"cancel"  |
"discard" |
"mix"     |
"normal"  |
"queue"   RETURN_STR (STRING);

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
"reboot"          |
"reset"           |
"runningcommands" |
"shutdown"        |
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
"false"         return token::TOK_FALSE;
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
"return"        return token::TOK_RETURN;
"static"        return token::TOK_STATIC;
"stopif"        return token::TOK_STOPIF;
"subclass"      return token::TOK_SUBCLASS;
"timeout"       return token::TOK_TIMEOUT;
"true"          return token::TOK_TRUE;
"unalias"       return token::TOK_UNALIAS;
"var"           return token::TOK_VAR;
"'in"           return token::TOK_VARIN;
"'out"          return token::TOK_VAROUT;
"wait"          return token::TOK_WAIT;
"waituntil"     return token::TOK_WAITUNTIL;
"whenever"      return token::TOK_WHENEVER;
"while"         return token::TOK_WHILE;

"!"             RETURN_OP(BANG);
"%"             RETURN_OP(PERCENT);
"*"             RETURN_OP(STAR);
"+"             RETURN_OP(PLUS);
"-"             RETURN_OP(MINUS);
"/"             RETURN_OP(DIV);
"<"             RETURN_OP(LTH);
"<="            RETURN_OP(LEQ);
"=="            RETURN_OP(EQU);
"=~="           RETURN_OP(DEQ);
">"             RETURN_OP(GTH);
">="            RETURN_OP(GEQ);
"^"             { up.warn(*locp, "The use of '^' for exponent operator"
			  " is deprecated. Use '**' instead.");
		  RETURN_OP(EXP); }
"**"            RETURN_OP(EXP);
"&&"            RETURN_OP(LAND);
"||"            RETURN_OP(LOR);

"="             return token::TOK_ASSIGN;
"!="            return token::TOK_NEQ;
"$"             return token::TOK_DOLLAR;
"%="            return token::TOK_PEQ;

"'"             return token::TOK_DERIV;
"''"            return token::TOK_DERIV2;
"'d"            return token::TOK_TRUEDERIV;
"'dd"           return token::TOK_TRUEDERIV2;
"'e"            return token::TOK_VARERROR;
"'n"            return token::TOK_NORM;
"("             return token::TOK_LPAREN;
")"             return token::TOK_RPAREN;
"++"            return token::TOK_PLUSPLUS;
"+="            return token::TOK_PLUSASSIGN;
"--"            return token::TOK_MINUSMINUS;
"-="            return token::TOK_MINUSASSIGN;
"->"            return token::TOK_DIR;
"."             return token::TOK_POINT;
":"             return token::TOK_COLON;
"::"            return token::TOK_DOUBLECOLON;
"@"             return token::TOK_AROBASE;
"["             return token::TOK_LBRACKET;
"]"             return token::TOK_RBRACKET;
"{"             return token::TOK_LBRACE;
"}"             return token::TOK_RBRACE;
"~"             return token::TOK_TILDE;
"~="            return token::TOK_REQ;

"&"             RETURN_FLAVOR(AND);
"|"             RETURN_FLAVOR(PIPE);
";"             RETURN_FLAVOR(SEMICOLON);
","             RETURN_FLAVOR(COMMA);


{ID}     RETURN_VALUE(IDENTIFIER, symbol, new libport::Symbol(yytext));


    /*----------.
    | Strings.  |
    `----------*/

<SC_STRING>{ /* Handling of the strings.  Initial " is eaten. */
     \" {
       BEGIN INITIAL;
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
     \\.	up.error (*locp, "unrecognized escape");


     [^\"\n\r\\]+      valp->str->append (yytext, yyleng);

     {EOL}+    locp->lines (yyleng); valp->str->append (yyleng, '\n');

     <<EOF>> {
       up.error (*locp, "unexpected end of file in a string");
       BEGIN INITIAL;
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

  [^*/\n\r]+	;
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
