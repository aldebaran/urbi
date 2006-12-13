/* \file ugrammar.y
 *******************************************************************************

 File: ugrammar.y\n
 Definition of the parser used by the UParser object.
 This parser is defined with bison, using the option %pure_parser to make it
 reentrant. For more details about reentrancy issues, check the definition of
 the UServer class.

 This file is part of
 %URBI, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

%require "2.2"
%error-verbose
%defines
%skeleton "lalr1.cc"
%parse-param {UParser& uparser}
%lex-param {UParser& uparser}
%debug

%{
#include "utypes.hh"
#include "ucommand.hh"
class UParser;
%}

// Locations.
%locations
%define "filename_type" "std::string"
%initial-action
{
  @$.initialize (uparser.filename_.empty() ? 0 : &uparser.filename_);
}


/* Possible data type returned by the bison parsing mechanism */
%union
{
  UCommand                *command;
  UExpression             *expr;
  UBinary                 *binary;
  UNamedParameters        *namedparameters;
  UVariableName           *variable;
  UVariableList           *variablelist;
  UProperty               *property;

  ufloat                   *val;
  UString                  *str;
  struct {
    UString *device;
    UString *id;
    bool rooted;
  }                        structure;
}

%{
// Is included in ugrammar.cc
#ifdef _MSC_VER
# include <hash_map>
#else
# include <hash_map.h>
#endif
#include <string>
#include <iostream>
#define TRUE  ufloat(1)
#define FALSE ufloat(0)

#include "parser/uparser.hh"

#include "uconnection.hh"
#include "uobj.hh"
#include "ugroup.hh"
#include "userver.hh"

extern UString** globalDelete;

/* Memory checking macros, used in the command tree building process */

#define MEMCHECK(p)						\
 do {								\
   if (p==0)							\
     {								\
       uparser.connection.server->isolate();			\
       uparser.connection.server->memoryOverflow = true;	\
     }								\
 } while (0)

#define MEMCHECK1(p, p1)					\
  do {								\
    if (p==0)							\
      {								\
	uparser.connection.server->isolate();			\
	uparser.connection.server->memoryOverflow = true;	\
	delete p1; p1 = 0;					\
      }								\
  } while (0)

#define MEMCHECK2(p, p1, p2)					\
  do {								\
    if (p==0)							\
      {								\
	uparser.connection.server->isolate();			\
	uparser.connection.server->memoryOverflow = true;	\
	delete p1; p1 = 0;					\
	delete p2; p2 = 0;					\
      }								\
  } while (0)

#define MEMCHECK3(p, p1, p2, p3)				\
  do {								\
    if (p==0)							\
      {								\
	uparser.connection.server->isolate();			\
	uparser.connection.server->memoryOverflow = true;	\
	delete p1; p1 = 0;					\
	delete p2; p2 = 0;					\
	delete p3; p3 = 0;					\
      }								\
  } while (0)

#define MEMCHECK4(p, p1, p2, p3, p4)				\
  do {								\
    if (p==0)							\
      {								\
	uparser.connection.server->isolate();			\
	uparser.connection.server->memoryOverflow = true;	\
	delete p1; p1 = 0;					\
	delete p2; p2 = 0;					\
	delete p3; p3 = 0;					\
	delete p4; p4 = 0;					\
      }								\
  } while (0)

/// Direct the call from 'bison' to the scanner in the right UParser.
inline
yy::parser::token::yytokentype
yylex(yy::parser::semantic_type* val, yy::location* loc, UParser& p)
{
  return p.scanner_.yylex(val, loc, p);
}


/// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
# define NEW_BIN(Res, Op, Lhs, Rhs)		\
    do {					\
      Res = new UCommand_TREE(Op, Lhs, Rhs);	\
      if (Res)					\
	Res->setTag("__node__");		\
      MEMCHECK2(Res, Lhs, Rhs);			\
    } while (0)

/// Create a new UExpression node composing \c Child with \c Op.
# define NEW_EXP_1(Res, Op, Child)			\
  do {							\
      Res = new UExpression(Op, Child);			\
      MEMCHECK1(Res, Child);				\
    } while (0)

/// Create a new UExpression node composing \c Lhs and \c Rhs with \c Op.
# define NEW_EXP_2(Res, Op, Lhs, Rhs)			\
  do {							\
      Res = new UExpression(Op, Lhs, Rhs);		\
      MEMCHECK2(Res, Lhs, Rhs);				\
    } while (0)


%}

/* List of all tokens and terminal symbols, with their type */

%token
  TOK_ADDGROUP     "addgroup"
  TOK_ALIAS        "alias"
  TOK_AND          "&"
  TOK_ANDOPERATOR  "&&"
  TOK_AROBASE      "@"
  TOK_ASSIGN       "="
  TOK_AT           "at"
  TOK_BANG         "!"
  TOK_BIN          "bin"
  TOK_BLOCK        "block"
  TOK_CLASS        "class"
  TOK_CMDBLOCK     "command block"
  TOK_COLON        ":"
  TOK_COMMA        ","
  TOK_COPY         "copy"
  TOK_DEF          "def"
  TOK_DELGROUP     "delgroup"
  TOK_DEQ          "=~="
  TOK_DERIV        "derivation"
  TOK_DERIV2       "second-derivation"
  TOK_DIR          "->"
  TOK_DISINHERIT   "disinherit"
  TOK_DIV          "/"
  TOK_DOLLAR       "$"
  TOK_DOUBLECOLON  "::"
  TOK_ELSE         "else"
  TOK_EMIT         "emit"
  TOK_EQ           "=="
  TOK_EVENT        "event"
  TOK_EVERY        "every"
  TOK_EXP          "^"
  TOK_EXPRBLOCK    "expression block"
  TOK_FALSECONST   "false"
  TOK_FOR          "for"
  TOK_FOREACH      "foreach"
  TOK_FREEZEIF     "freezeif"
  TOK_FROM         "from"
  TOK_FUNCTION     "function"
  TOK_GE           ">="
  TOK_GROUP        "group"
  TOK_GROUPLIST    "group list"
  TOK_GT           ">"
  TOK_IF           "if"
  TOK_IN           "in"
  TOK_INFO         "info"
  TOK_INHERIT      "inherit"
  TOK_LBRACKET     "{"
  TOK_LE           "<="
  TOK_LOOP         "loop"
  TOK_LOOPN        "loopn"
  TOK_LPAREN       "("
  TOK_LSBRACKET    "["
  TOK_LT           "<"
  TOK_MINUS        "-"
  TOK_MINUSASSIGN  "-="
  TOK_MINUSMINUS   "--"
  TOK_MULT         "*"
  TOK_NE           "!="
  TOK_NEW          "new"
  TOK_NOOP         "noop"
  TOK_NORM         "normalized"
  TOK_OBJECT       "object"
  TOK_ONLEAVE      "onleave"
  TOK_ONLY         "only"
  TOK_OROPERATOR   "||"
  TOK_PEQ          "%="
  TOK_PERCENT      "%"
  TOK_PIPE         "|"
  TOK_PLUS         "+"
  TOK_PLUSASSIGN   "+="
  TOK_PLUSPLUS     "++"
  TOK_POINT        "."
  TOK_RBRACKET     "}"
  TOK_REQ          "~="
  TOK_RETURN       "return"
  TOK_RPAREN       ")"
  TOK_RSBRACKET    "]"
  TOK_SEMICOLON    ";"
  TOK_STATIC       "static"
  TOK_STOP         "stop"
  TOK_STOPIF       "stopif"
  TOK_SUBCLASS     "subclass"
  TOK_TILDE        "~"
  TOK_TIMEOUT      "timeout"
  TOK_TRUECONST    "true"
  TOK_TRUEDERIV    "command-derivation"
  TOK_TRUEDERIV2   "second-command-derivation"
  TOK_UECHO        "echo"    // Flex defines the ECHO macro
  TOK_UNALIAS      "unalias"
  TOK_UNBLOCK      "unblock"
  TOK_UNIT         "unit"
  TOK_VAR          "var"
  TOK_VARERROR     "'e"
  TOK_VARIN        "'in"
  TOK_VAROUT       "'out"
  TOK_WAIT         "wait"
  TOK_WAITUNTIL    "waituntil"
  TOK_WHENEVER     "whenever"
  TOK_WHILE        "while"

%token UEOF 0 "end of command"

/*------.
| Val.  |
`------*/

%token
  <val> NUM        "number"
  <val> TIMEVALUE  "time"
  <val> FLAG       "flag"
  <val> FLAGTEST   "flag test"
  <val> FLAGID     "flag identifier"
  <val> FLAGTIME   "flag time"
// FIXME: Simplify once Bison 2.4 is out.
%printer { debug_stream() << *$$; }
  NUM TIMEVALUE FLAG FLAGTEST FLAGID FLAGTIME;

 /*------.
 | Str.  |
 `------*/
%token
   <str>  IDENTIFIER         "identifier"
   <str>  TAG                "tag"
   <str>  STRING             "string"
   <str>  SWITCH             "switch"
   <str>  BINDER             "binder"
   <str>  OPERATOR           "operator command"
   <str>  OPERATOR_ID        "operator"
   <str>  OPERATOR_ID_PARAM  "param-operator"
   <str>  OPERATOR_VAR       "var-operator"
// FIXME: Simplify once Bison 2.4 is out.
%printer { debug_stream() << *$$; }
   "identifier" TAG STRING SWITCH BINDER OPERATOR OPERATOR_ID
   OPERATOR_ID_PARAM OPERATOR_VAR;

%token <structure>           STRUCT      "structured identifier"
%token <structure>           REFSTRUCT   "structured ref-identifier"

%type  <expr>                expr            "expression"
%type  <val>                 timeexpr        "time expression"
%type  <command>             taggedcommands  "set of commands"
%type  <command>             taggedcommand   "tagged command"
%type  <command>             command         "command"
%type  <command>             instruction     "instruction"
%type  <namedparameters>     parameters      "parameters"
%type  <namedparameters>     array           "array"
%type  <namedparameters>     parameterlist   "list of parameters"
%type  <namedparameters>     rawparameters   "list of attributes"
%type  <namedparameters>     namedparameters "list of named parameters"
%type  <namedparameters>     flags           "list of flags"
%type  <variablelist>        refvariables    "list of variables"
%type  <expr>                softtest        "soft test"
%type  <namedparameters>     identifiers     "list of identifiers"
%type  <expr>                class_declaration "class declaration"
%type  <namedparameters>     class_declaration_list "class declaration list"
%type  <binary>              binary          "binary"
%type  <property>            property        "property"
%type  <variable>            variable        "variable"
%type  <variable>            purevariable    "pure variable"
//%type  <namedparameters>     purevariables   "list of pure variables"
%type  <variable>            refvariable     "ref-variable"


/* Operator precedence. */

%left  "||" "&&" "!"
%left  "==" "~=" "%=" "=~=" "!=" ">" ">=" "<" "<="
%left  "-" "+"
%left  "*" "/" "%"
%left  NEG     /* Negation--unary minus */
%right "^"
%right TOK_NORM

%right "," ";"
%left  "&" "|"
%left  CMDBLOCK EXPRBLOCK
%left  "else" "onleave"
%nonassoc "="


/* URBI Grammar */
%%

%start root;
root:

    refvariable "=" binary ";" {

      URefPt<UBinary> *ref = new URefPt<UBinary>($3);
      MEMCHECK(ref);
      UCommand* tmpcmd = new UCommand_ASSIGN_BINARY($1,ref);
      if (tmpcmd) tmpcmd->setTag("__node__");
      MEMCHECK2(tmpcmd,$1,ref);
      if (tmpcmd) uparser.binaryCommand = true;

      uparser.commandTree  = new UCommand_TREE(USEMICOLON,tmpcmd,0);
      if ( uparser.commandTree )
	uparser.commandTree->setTag("__node__");
      MEMCHECK(uparser.commandTree);
    }

  | taggedcommands {

      uparser.commandTree = 0;
      if ($1) {
	if ($1->type == CMD_TREE)
	  uparser.commandTree = (UCommand_TREE*)$1;
	else
	  delete $1;
      }
    }
;


/* TAGGEDCOMMANDS */
taggedcommands:
  taggedcommand
| taggedcommands "," taggedcommands { NEW_BIN ($$, UCOMMA,$1,$3); }
| taggedcommands ";" taggedcommands { NEW_BIN ($$, USEMICOLON,$1,$3); }
| taggedcommands "|" taggedcommands { NEW_BIN ($$, UPIPE,$1,$3); }
| taggedcommands "&" taggedcommands { NEW_BIN ($$, UAND,$1,$3);}
;

/* TAGGEDCOMMAND */

taggedcommand:

    command {
      if ($1)
	$1->setTag(UNKNOWN_TAG);

      $$ = $1;
    }

  | "identifier" flags ":" command {

      MEMCHECK($1);
      if ($4) {
	$4->setTag($1->str());
	$4->flags = $2;
      }
      $$ = $4;
    }

  | TAG flags ":" command {

      MEMCHECK($1);
      if ($4) {
	$4->setTag($1->str());
	$4->flags = $2;
      }
      $$ = $4;
    }

  | "identifier" ":" command {

      MEMCHECK($1);
      if ($3) {
	$3->setTag($1->str());
      }
      $$ = $3;
    }

  | TAG ":" command {

      MEMCHECK($1);
      if ($3) {
	$3->setTag($1->str());
      }
      $$ = $3;
    }


  | STRUCT ":" command {

      MEMCHECK($1.device);
      MEMCHECK($1.id);
      if ($3) {
	$3->setTag(UString($1.device,$1.id).str());
	delete $1.device;
	delete $1.id;
      }
      $$ = $3;
    }

  | STRUCT flags ":" command {

      MEMCHECK($1.device);
      MEMCHECK($1.id);

      if ($4) {
       	$4->setTag(UString($1.device,$1.id).str());

	delete $1.device;
	delete $1.id;

	$4->flags = $2;
      }
      $$ = $4;
    }

  | flags ":" command {

      MEMCHECK($1);
      if ($3) {
	$3->setTag(UNKNOWN_TAG);
	$3->flags = $1;
      }
      $$ = $3;
    }
;

/* FLAGS */

flags :
     FLAG  {

      UExpression *flagval = new UExpression(EXPR_VALUE,$1);
      MEMCHECK(flagval);

      $$ = new UNamedParameters(new UString("flag"),flagval,0);
      MEMCHECK1($$,flagval);
    }

  |  FLAG flags  {

      UExpression *flagval = new UExpression(EXPR_VALUE,$1);
      MEMCHECK(flagval);

      $$ = new UNamedParameters(new UString("flag"),flagval,$2);
      MEMCHECK2($$,flagval,$2);
    }

  | FLAGTIME "(" expr ")" flags {

      $$ = new UNamedParameters(new UString("flagtimeout"),$3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | FLAGTIME "(" expr ")" {

      $$ = new UNamedParameters(new UString("flagtimeout"),$3,0);
      MEMCHECK1($$,$3);
    }

  | FLAGID "(" expr ")" {

      $$ = new UNamedParameters(new UString("flagid"),$3,0);
      MEMCHECK1($$,$3);
    }

  | FLAGID "(" expr ")" flags {

      $$ = new UNamedParameters(new UString("flagid"),$3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | FLAGTEST "(" softtest ")" flags {

      if (*$1 == 6)
	$$ = new UNamedParameters(new UString("flagstop"),$3,$5);
      else
	$$ = new UNamedParameters(new UString("flagfreeze"),$3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | FLAGTEST "(" softtest ")" {

      if (*$1 == 6)
	$$ = new UNamedParameters(new UString("flagstop"),$3,0);
      else
	$$ = new UNamedParameters(new UString("flagfreeze"),$3,0);
      MEMCHECK1($$,$3);
    }
;

/* COMMAND */

command:

    instruction

  | "{" taggedcommands "}" {

      $$ = (UCommand*)
	new UCommand_TREE(UPIPE,
			  $2,
			  new UCommand_NOOP(true));
      $$->setTag("__UGrouped_set_of_commands__");
      ((UCommand_TREE*)$$)->command2->setTag("__system__");
    }
;

/* INSTRUCTION */

instruction:
  /* empty */ { $$ = 0; }

  | TOK_NOOP {
      $$ = new UCommand_NOOP();
      MEMCHECK($$);
    }

  | refvariable "=" expr namedparameters {

    $$ = new UCommand_ASSIGN_VALUE($1,$3,$4, false);
      MEMCHECK3($$,$1,$3,$4);
    }

  | refvariable "+=" expr {

      $$ = new UCommand_AUTOASSIGN($1,$3,0);
      MEMCHECK2($$,$1,$3);
    }

  | refvariable "-=" expr {

      $$ = new UCommand_AUTOASSIGN($1,$3,1);
      MEMCHECK2($$,$1,$3);
    }


  | "var" refvariable "=" expr namedparameters {

      $2->local_scope = true;
      $$ = new UCommand_ASSIGN_VALUE($2,$4,$5);
      MEMCHECK3($$,$2,$4,$5);
    }

  | property "=" expr {

      $$ = new UCommand_ASSIGN_PROPERTY($1->variablename,$1->property,$3);
      MEMCHECK3($$,$1,$1,$3);
    }

  | expr {

      $$ = new UCommand_EXPR($1);
      MEMCHECK1($$,$1);
    }

  | refvariable NUM {

      $$ = new UCommand_DEVICE_CMD($1,$2);
      MEMCHECK1($$,$1);
    }

  | "return" {

      $$ = new UCommand_RETURN((UExpression*)0);
      MEMCHECK($$);
    }

  | "return" expr {

      $$ = new UCommand_RETURN($2);
      MEMCHECK1($$,$2);
    }

  | "echo" expr namedparameters {

      $$ = new UCommand_ECHO($2,$3,(UString*)0);
      MEMCHECK2($$,$2,$3);
    }

  | refvariable "=" "new" "identifier" {

      MEMCHECK($4);
      $$ = new UCommand_NEW($1,$4,(UNamedParameters*)0,true);
      MEMCHECK2($$,$1,$4);
    }

  | refvariable "=" "new" "identifier" "(" parameterlist ")" {

      MEMCHECK($4);
      $$ = new UCommand_NEW($1,$4,$6);
      MEMCHECK3($$,$1,$4,$6);
    }


  | "group" "identifier" "{" identifiers "}" {

      $$ = new UCommand_GROUP($2,$4);
      MEMCHECK2($$,$4,$2);
    }

  | "addgroup" "identifier" "{" identifiers "}" {

      $$ = new UCommand_GROUP($2,$4,1);
      MEMCHECK2($$,$4,$2);
    }


  | "delgroup" "identifier" "{" identifiers "}" {

      $$ = new UCommand_GROUP($2,$4,2);
      MEMCHECK2($$,$4,$2);
    }

   /*
  | GROUP "identifier" {

      $$ = new UCommand_GROUP($2,(UNamedParameters*)0);
      MEMCHECK1($$,$2);
    }
*/
  | "group" {

      $$ = new UCommand_GROUP((UString*)0,(UNamedParameters*)0);
      MEMCHECK($$);
    }


  | "alias" purevariable purevariable {

      $$ = new UCommand_ALIAS($2,$3);
      MEMCHECK2($$,$2,$3);
    }

  | purevariable "inherit" purevariable {

      $$ = new UCommand_INHERIT($1,$3);
      MEMCHECK2($$,$1,$3);
    }

  | purevariable "disinherit" purevariable {

      $$ = new UCommand_INHERIT($1,$3,true);
      MEMCHECK2($$,$1,$3);
    }

  | "alias" purevariable {

      $$ = new UCommand_ALIAS($2,(UVariableName*)0);
      MEMCHECK1($$,$2);
    }

  | "unalias" purevariable {

      $$ = new UCommand_ALIAS($2,(UVariableName*)0,true);
      MEMCHECK1($$,$2);
    }

  | "alias" {

      $$ = new UCommand_ALIAS((UVariableName*)0,(UVariableName*)0);
      MEMCHECK($$);
  }

  | OPERATOR {

      MEMCHECK($1);
      $$ = new UCommand_OPERATOR($1);
      MEMCHECK1($$,$1);
    }

  | OPERATOR_ID "identifier" {

      MEMCHECK($1);
      MEMCHECK($2);
      $$ = new UCommand_OPERATOR_ID($1,$2);
      MEMCHECK2($$,$1,$2);
    }

  | OPERATOR_ID TAG {

      MEMCHECK($1);
      MEMCHECK($2);
      $$ = new UCommand_OPERATOR_ID($1,$2);
      MEMCHECK2($$,$1,$2);
    }

  | OPERATOR_ID STRUCT {

      MEMCHECK($1);
      MEMCHECK($2.device);
      MEMCHECK($2.id);
      $$ = new UCommand_OPERATOR_ID($1,new UString($2.device,$2.id));
      delete $2.device;
      delete $2.id;
      MEMCHECK1($$,$1);
    }

  | OPERATOR_VAR variable {

      MEMCHECK($1);
      $$ = new UCommand_OPERATOR_VAR($1,$2);
      MEMCHECK2($$,$1,$2);
    }

  | BINDER TOK_OBJECT purevariable {

      MEMCHECK($1);
      $$ = new UCommand_BINDER((UVariableName*)0,$1,3,$3);
      MEMCHECK2($$,$1,$3);
    }


  | BINDER "var" purevariable "from" purevariable {

      MEMCHECK($1);
      $$ = new UCommand_BINDER($5,$1,1,$3);
      MEMCHECK3($$,$1,$3,$5);
    }

  | BINDER "function" "(" NUM ")" purevariable "from" purevariable {

      MEMCHECK($1);
      $$ = new UCommand_BINDER($8,$1,0,$6,(int)(*$4));
      MEMCHECK3($$,$1,$6,$8);
    }

  | BINDER "event" "(" NUM ")" purevariable "from" purevariable {

      MEMCHECK($1);
      $$ = new UCommand_BINDER($8,$1,2,$6,(int)(*$4));
      MEMCHECK3($$,$1,$6,$8);
    }

  | TOK_WAIT expr {

      $$ = new UCommand_WAIT($2);
      MEMCHECK1($$,$2);
    }

  | "emit" purevariable {

      $2->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT($2,(UNamedParameters*)0);
      MEMCHECK1($$,$2);
    }

  | "emit" purevariable "(" parameterlist ")" {

      $2->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT($2,$4);
      MEMCHECK2($$,$2,$4);
    }

  | "emit" "(" expr ")" purevariable {

      $5->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT($5,(UNamedParameters*)0,$3);
      MEMCHECK2($$,$5,$3);
    }

  | "emit" "(" expr ")" purevariable "(" parameterlist ")" {

      $5->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT($5,$7,$3);
      MEMCHECK3($$,$5,$7,$3);
    }

  | "emit" "(" ")" purevariable {

      $4->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT($4,(UNamedParameters*)0,
      	new UExpression(EXPR_VALUE,UINFINITY));
      MEMCHECK1($$,$4);
    }

  | "emit" "(" ")" purevariable "(" parameterlist ")" {

      $4->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT($4,$6,
      	new UExpression(EXPR_VALUE,UINFINITY));
      MEMCHECK2($$,$4,$6);
    }

  | "waituntil" softtest {

      $$ = new UCommand_WAIT_TEST($2);
      MEMCHECK1($$,$2);
    }

  | refvariable TOK_MINUSMINUS {

      $$ = new UCommand_INCDECREMENT(CMD_DECREMENT,$1);
      MEMCHECK1($$,$1);
    }

  | refvariable TOK_PLUSPLUS {

      $$ = new UCommand_INCDECREMENT(CMD_INCREMENT,$1);
      MEMCHECK1($$,$1);
    }

  | TOK_DEF {

      $$ = new UCommand_DEF(UDEF_QUERY,
      	                    (UVariableName*)0,
			    (UNamedParameters*)0,
			    (UCommand*)0);
      MEMCHECK($$)
    }

  | "var" refvariable {

      $2->local_scope = true;
      $$ = new UCommand_DEF(UDEF_VAR,$2,
			    (UNamedParameters*)0,
			    (UCommand*)0);

      MEMCHECK1($$,$2)
    }

  | TOK_DEF refvariable {

      $2->local_scope = true;
      $$ = new UCommand_DEF(UDEF_VAR,$2,
			    (UNamedParameters*)0,
			    (UCommand*)0);

      MEMCHECK1($$,$2)
    }

  | "var" "{" refvariables "}" {

      $$ = new UCommand_DEF(UDEF_VARS,$3);
      MEMCHECK1($$,$3)
    }

  | TOK_CLASS "identifier" "{" class_declaration_list "}" {

      $$ = new UCommand_CLASS($2,$4);
      MEMCHECK2($$,$2,$4)
    }

  | TOK_CLASS "identifier" {

      $$ = new UCommand_CLASS($2,(UNamedParameters*)0);
      MEMCHECK1($$,$2)
    }


  | "event" variable "(" identifiers ")" {

      $2->local_scope = true;
      $2->id_type = UDEF_EVENT;
      $$ = new UCommand_DEF(UDEF_EVENT,$2,$4,(UCommand*)0);
      MEMCHECK2($$,$2,$4);
    }

  | "event" variable {

      $2->local_scope = true;
      $2->id_type = UDEF_EVENT;
      $$ = new UCommand_DEF(UDEF_EVENT,$2,(UNamedParameters*)0,(UCommand*)0);
      MEMCHECK1($$,$2);
    }
/**/
  | "function" variable "(" identifiers ")" {

      if (uparser.connection.functionTag) {
	delete $2;
	delete $4;
	$2 = 0;
	delete uparser.connection.functionTag;
	uparser.connection.functionTag = 0;
	error(@$,"Nested function def not allowed.");
	YYERROR;
      }
      else {
	uparser.connection.functionTag = new UString("__Funct__");
	globalDelete = &uparser.connection.functionTag;
      }

    } taggedcommand {

      $2->id_type = UDEF_FUNCTION;
      $$ = new UCommand_DEF(UDEF_FUNCTION,$2,$4,$7);

      MEMCHECK2($$,$2,$4);
      if (uparser.connection.functionTag) {
	delete uparser.connection.functionTag;
	uparser.connection.functionTag = 0;
	globalDelete = 0;
      }
    }

  | TOK_DEF variable "(" identifiers ")" {

      uparser.connection.server->debug("Warning: 'def' is deprecated, use 'function' instead\n");
      if (uparser.connection.functionTag) {
	delete $2;
	delete $4;
	$2 = 0;
	delete uparser.connection.functionTag;
	uparser.connection.functionTag = 0;
	error(@$,"Nested function def not allowed.");
	YYERROR;
      }
      else {
	uparser.connection.functionTag = new UString("__Funct__");
	globalDelete = &uparser.connection.functionTag;
      }

    } taggedcommand {

      $2->id_type = UDEF_FUNCTION;
      $$ = new UCommand_DEF(UDEF_FUNCTION,$2,$4,$7);

      MEMCHECK2($$,$2,$4);
      if (uparser.connection.functionTag) {
	delete uparser.connection.functionTag;
	uparser.connection.functionTag = 0;
	globalDelete = 0;
      }
    }

  | TOK_IF "(" expr ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_IF($3,$5,(UCommand*)0);
      MEMCHECK2($$,$3,$5);
    }

  | "if" "(" expr ")" taggedcommand "else" taggedcommand {

      if (!$5)
      {
	delete $3;
	delete $5;
	delete $7;
	error(@$, "Empty then-part within a if.");
	YYERROR;
      }
      $$ = new UCommand_IF($3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | TOK_EVERY "(" expr ")" taggedcommand {

      $$ = new UCommand_EVERY($3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | "timeout" "(" expr ")" taggedcommand {

      $$ = new UCommand_TIMEOUT($3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | "stopif" "(" softtest ")" taggedcommand {

      $$ = new UCommand_STOPIF($3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | TOK_FREEZEIF "(" softtest ")" taggedcommand {

      $$ = new UCommand_FREEZEIF($3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | "at" "(" softtest ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_AT(CMD_AT,$3,$5,(UCommand*)0);
      MEMCHECK2($$,$3,$5);
    }

  | "at" "(" softtest ")" taggedcommand "onleave" taggedcommand {
      if(!$5)
      {
	delete $3;
	delete $5;
	delete $7;
	error(@$,"Empty body within a at command.");
	YYERROR;
      }
      $$ = new UCommand_AT(CMD_AT,$3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | "at" "&" "(" softtest ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_AT(CMD_AT_AND,$4,$6,(UCommand*)0);
      MEMCHECK2($$,$4,$6);
    }

  | "at" "&" "(" softtest ")" taggedcommand "onleave" taggedcommand {

      $$ = new UCommand_AT(CMD_AT_AND,$4,$6,$8);
      MEMCHECK3($$,$4,$6,$8);
    }

  | "while" "(" expr ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_WHILE(CMD_WHILE,$3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | "while" "|" "(" expr ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_WHILE(CMD_WHILE_PIPE,$4,$6);
      MEMCHECK2($$,$4,$6);
    }

  | "whenever" "(" softtest ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_WHENEVER($3,$5,(UCommand*)0);
      MEMCHECK2($$,$3,$5);
    }

  | "whenever" "(" softtest ")" taggedcommand "else" taggedcommand {

      $$ = new UCommand_WHENEVER($3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | "loop" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOP($2);
      MEMCHECK1($$,$2);
    }

  | "foreach" purevariable "in" expr "{" taggedcommands "}" %prec CMDBLOCK {

      $$ = new UCommand_FOREACH(CMD_FOREACH,$2,$4,$6);
      MEMCHECK3($$,$2,$4,$6);
    }

  | "foreach" "&" purevariable "in" expr "{" taggedcommands "}" %prec CMDBLOCK {

      $$ = new UCommand_FOREACH(CMD_FOREACH_AND,$3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | "foreach" "|" purevariable "in" expr "{" taggedcommands "}" %prec CMDBLOCK {

      $$ = new UCommand_FOREACH(CMD_FOREACH_PIPE,$3,$5,$7);
      MEMCHECK3($$,$3,$5,$7);
    }

  | "loopn" "(" expr ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOPN(CMD_LOOPN,$3,$5);
      MEMCHECK2($$,$3,$5);
    }

  | "loopn" "|" "(" expr ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOPN(CMD_LOOPN_PIPE,$4,$6);
      MEMCHECK2($$,$4,$6);
    }

  | "loopn" "&" "(" expr ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOPN(CMD_LOOPN_AND,$4,$6);
      MEMCHECK2($$,$4,$6);
    }

  | "for" "(" instruction ";"
	       expr ";"
	       instruction ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_FOR(CMD_FOR,$3,$5,$7,$9);
      MEMCHECK4($$,$3,$5,$7,$9);
    }

  | "for" "|" "(" instruction ";"
		    expr ";"
		    instruction ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_FOR(CMD_FOR_PIPE,$4,$6,$8,$10);
      MEMCHECK4($$,$4,$6,$8,$10);
    }

  | "for" "&" "(" instruction ";"
		   expr ";"
		   instruction ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_FOR(CMD_FOR_AND,$4,$6,$8,$10);
      MEMCHECK4($$,$4,$6,$8,$10);
    }
;


/* ARRAY */

array:

  /* empty */ { $$ = 0 }

  | "[" expr "]" array {

      $$ = new UNamedParameters($2,$4);
      MEMCHECK2($$,$2,$4);
    }
;


/* VARID, PUREVARIABLE, VARIABLE, REFVARIABLE */

purevariable:

    "$" "(" expr ")" {

      $$ = new UVariableName($3);
      MEMCHECK1($$,$3);
    }

  | "identifier" array TOK_POINT "identifier" array {

      MEMCHECK($1);
      MEMCHECK($4);
      $$ = new UVariableName($1,$2,$4,$5);
      MEMCHECK4($$,$1,$2,$4,$5);
    }

  | "identifier" "::" "identifier" {

      MEMCHECK($1);
      MEMCHECK($3);
      $$ = new UVariableName($1,$3,true,
			     (UNamedParameters*)0);
      if ($$) $$->doublecolon = true;
      MEMCHECK2($$,$1,$3);
    }

  | "identifier" array {

      MEMCHECK($1);
      if (uparser.connection.functionTag) {
	// We are inside a function
	  $$ = new UVariableName(new UString(uparser.connection.functionTag),$1,false,$2);
      }
      else
	$$ = new UVariableName(new UString(uparser.connection.connectionTag),
	    $1,false,$2);
      MEMCHECK2($$,$1,$2);
      $$->nostruct = true;
    }

  | STRUCT array {

      MEMCHECK($1.device);
      MEMCHECK($1.id);
      $$ = new UVariableName($1.device,$1.id,false,$2);
      MEMCHECK3($$,$1.device,$1.id,$2);
    }

;

variable:

     purevariable {

       $$ = $1;
    }

  | "static" purevariable {

      $$ = $2;
      $$->isstatic = true;
    }

  | purevariable TOK_NORM {

      $$ = $1;
      $$->isnormalized = true;
    }

  | purevariable TOK_VARERROR {

      $$ = $1;
      $$->varerror = true;
    }

  | purevariable TOK_VARIN {

      $$ = $1;
      $$->varin = true;
    }

  | purevariable TOK_VAROUT {

      $$ = $1;
    }


  | purevariable TOK_DERIV {

      $$ = $1;
      $$->deriv = UDERIV;
    }

  | purevariable TOK_DERIV2 {

      $$ = $1;
      $$->deriv = UDERIV2;
    }

  | purevariable TOK_TRUEDERIV {

      $$ = $1;
      $$->deriv = UTRUEDERIV;
    }

  | purevariable TOK_TRUEDERIV2 {

      $$ = $1;
      $$->deriv = UTRUEDERIV2;
    }
;

refvariable:
    variable {

      $$ = $1;
    }

  | TOK_ONLY variable {

      $$ = $2;
      $$->rooted = true;
  }
;


/* PROPERTY */

property:

    purevariable "->" "identifier" {

      $$ = new UProperty($1,$3);
      MEMCHECK2($$,$1,$3);
    }
;


/* NAMEDPARAMETERS */

namedparameters:
  /* empty */ { $$ = 0 }

  | "identifier" ":" expr namedparameters {

      MEMCHECK($1);
      $$ = new UNamedParameters($1,$3,$4);
      MEMCHECK3($$,$1,$4,$3);
    }
;


/* BINARY */

binary:
    TOK_BIN NUM {

      $$ = new UBinary((int)(*$2),0);
      MEMCHECK($$);
      if ($$ != 0)
	MEMCHECK1($$->buffer,$$);
    }

  | TOK_BIN NUM rawparameters {

      $$ = new UBinary((int)(*$2),$3);
      MEMCHECK1($$,$3);
      if ($$ != 0)
	MEMCHECK2($$->buffer,$$,$3);
    }
;


/* TIMEEXPR */

timeexpr:
    TIMEVALUE {
      $$ = $1;
    }

  | timeexpr TIMEVALUE {
       $$ = new ufloat(*$1+*$2);
       delete $1;
       delete $2;
    }
;


/* EXPR */

expr:
    NUM {

      $$ = new UExpression(EXPR_VALUE,$1);
      MEMCHECK($$);
    }

  | timeexpr {

      $$ = new UExpression(EXPR_VALUE,$1);
      MEMCHECK($$);
    }

  | STRING {

      MEMCHECK($1);
      $$ = new UExpression(EXPR_VALUE,$1);
      MEMCHECK1($$,$1);
    }

  | "[" parameterlist "]" {

      $$ = new UExpression(EXPR_LIST,$2);
      MEMCHECK1($$,$2);
    }

  | "%" variable { NEW_EXP_1 ($$, EXPR_ADDR_VARIABLE, $2); }

  | property {

       $$ = new UExpression(EXPR_PROPERTY,$1->property,$1->variablename);
       MEMCHECK1($$,$1);
    }

  | refvariable "(" parameterlist ")"  {

      //if (($1) && ($1->device) &&
      //    ($1->device->equal(uparser.connection.functionTag)))
      //  $1->nameUpdate(uparser.connection.connectionTag->str(),
      //                 $1->id->str());

      $1->id_type = UDEF_FUNCTION;
      NEW_EXP_2($$, EXPR_FUNCTION,$1,$3);
    }

  | variable         { NEW_EXP_1 ($$, EXPR_VARIABLE,$1); }
  | "group" "identifier" { NEW_EXP_1 ($$, EXPR_GROUP,$2); }
;


  /* num expr */
expr:
    expr "+" expr	{ NEW_EXP_2 ($$, EXPR_PLUS, $1,$3); }
  | expr "-" expr	{ NEW_EXP_2 ($$, EXPR_MINUS,$1,$3); }
  | expr "*" expr	{ NEW_EXP_2 ($$, EXPR_MULT, $1,$3); }
  | expr "/" expr	{ NEW_EXP_2 ($$, EXPR_DIV,  $1,$3); }
  | expr "%" expr 	{ NEW_EXP_2 ($$, EXPR_MOD,  $1,$3); }
  | expr "^" expr	{ NEW_EXP_2 ($$, EXPR_EXP,  $1,$3); }

  | TOK_COPY expr  %prec NEG {

      $$ = new UExpression(EXPR_COPY,$2,(UExpression*)0);
      MEMCHECK1($$,$2);
    }

  | "-" expr %prec NEG {

      $$ = new UExpression(EXPR_NEG,$2,(UExpression*)0);
      MEMCHECK1($$,$2);
    }

  | "(" expr ")" {

      $$ = $2;
    }
;

  /* Tests */
expr:
    TOK_TRUECONST {

      $$ = new UExpression(EXPR_VALUE,TRUE);
    }

  | TOK_FALSECONST {

      $$ = new UExpression(EXPR_VALUE,FALSE);
    }

  | expr TOK_EQ expr  { NEW_EXP_2 ($$, EXPR_TEST_EQ,  $1, $3); }
  | expr TOK_REQ expr { NEW_EXP_2 ($$, EXPR_TEST_REQ, $1, $3); }
  | expr TOK_DEQ expr { NEW_EXP_2 ($$, EXPR_TEST_DEQ, $1, $3); }
  | expr TOK_PEQ expr { NEW_EXP_2 ($$, EXPR_TEST_PEQ, $1, $3); }
  | expr TOK_NE expr  { NEW_EXP_2 ($$, EXPR_TEST_NE,  $1, $3); }
  | expr TOK_GT expr  { NEW_EXP_2 ($$, EXPR_TEST_GT,  $1, $3); }
  | expr TOK_GE expr  { NEW_EXP_2 ($$, EXPR_TEST_GE,  $1, $3); }
  | expr TOK_LT expr  { NEW_EXP_2 ($$, EXPR_TEST_LT,  $1, $3); }
  | expr TOK_LE expr  { NEW_EXP_2 ($$, EXPR_TEST_LE,  $1, $3); }

  | TOK_BANG expr {

      $$ = new UExpression(EXPR_TEST_BANG,$2,(UExpression*)0);
      MEMCHECK1($$,$2);
    }

  | expr "&&" expr { NEW_EXP_2 ($$, EXPR_TEST_AND, $1, $3); }
  | expr "||" expr { NEW_EXP_2 ($$, EXPR_TEST_OR,  $1, $3); }

    /*
    // Not needed anymore => will be handled nicely by aliases
  |
  GROUPLIST purevariable {
    $$ = new UExpression(EXPR_GROUPLIST,$2);
      MEMCHECK1($$,$2);
  }
  */
;


/* PARAMETERLIST, PARAMETERS, PARAMETERSERIES */

parameterlist:
  /* empty */ { $$ = 0; }

  | parameters
;

parameters:
    expr {

      $$ = new UNamedParameters($1);
      MEMCHECK1($$,$1);
    }

  | expr "," parameters {

      $$ = new UNamedParameters($1,$3);
      MEMCHECK2($$,$1,$3);
    }
;

rawparameters:
    NUM {

      UExpression *expr = new UExpression(EXPR_VALUE,$1);
      $$ = new UNamedParameters(expr);
      MEMCHECK1($$,expr);
    }

  | "identifier" {

      UExpression *expr = new UExpression(EXPR_VALUE,$1);
      $$ = new UNamedParameters(expr);
      MEMCHECK1($$,expr);
    }

  |  NUM rawparameters {

      UExpression *expr = new UExpression(EXPR_VALUE,$1);
      $$ = new UNamedParameters(expr,$2);
      MEMCHECK2($$,$2,expr);
    }

  |  "identifier" rawparameters {

      UExpression *expr = new UExpression(EXPR_VALUE,$1);
      $$ = new UNamedParameters(expr,$2);
      MEMCHECK2($$,$2,expr);
    }
;

/* SOFTTEST */

softtest:
    expr
  | expr "~" expr  {

      $$ = $1;
      $$->issofttest = true;
      $$->softtest_time = $3;
    }
  | "(" expr "~" expr ")" {

      $$ = $2;
      $$->issofttest = true;
      $$->softtest_time = $4;
    }
;

/* "identifier"S */

identifiers:
  /* empty */  { $$ = 0; }

  | "identifier" {

      MEMCHECK($1);
      $$ = new UNamedParameters($1,0);
      MEMCHECK1($$,$1);
    }

  | "var" "identifier" {

      MEMCHECK($2);
      $$ = new UNamedParameters($2,0);
      MEMCHECK1($$,$2);
    }


  | "identifier" "," identifiers {

      MEMCHECK($1);
      $$ = new UNamedParameters($1,0,$3);
      MEMCHECK2($$,$3,$1);
    }

  | "var" "identifier" "," identifiers {

      MEMCHECK($2);
      $$ = new UNamedParameters($2,0,$4);
      MEMCHECK2($$,$4,$2);
    }

;

/* CLASS_DELCARATION & CLASS_DECLARATION_LIST */

class_declaration:

    "var" "identifier" {

      MEMCHECK($2);
      $$ = new UExpression(EXPR_VALUE,$2);
      MEMCHECK1($$,$2);
    }

  | "function" variable "(" identifiers ")" {
      $2->id_type = UDEF_FUNCTION;
      NEW_EXP_2 ($$, EXPR_FUNCTION,$2,$4);
    }

  | "function" variable {
      $2->id_type = UDEF_FUNCTION;
      $$ = new UExpression(EXPR_FUNCTION,$2,(UNamedParameters*)0);
      MEMCHECK1($$,$2);
    }

  | "event" variable "(" identifiers ")" {
      $2->id_type = UDEF_EVENT;
      NEW_EXP_2 ($$, EXPR_EVENT,$2,$4);
    }

  | "event" variable {
      $2->id_type = UDEF_EVENT;
      $$ = new UExpression(EXPR_EVENT,$2,(UNamedParameters*)0);
      MEMCHECK1($$,$2);
    }
;


class_declaration_list:
  /* empty */  { $$ = 0; }

  | class_declaration {
      $$ = new UNamedParameters($1,0);
      MEMCHECK1($$,$1);
    }

  | class_declaration ";" class_declaration_list {
      $$ = new UNamedParameters($1,$3);
      MEMCHECK2($$,$3,$1);
    }
;

/* REFVARIABLES */

refvariables:
  /* empty */  { $$ = 0; }

  | refvariable {
      MEMCHECK($1);
      $$ = new UVariableList($1);
      MEMCHECK1($$,$1);
    }

  | refvariable ";" refvariables {
      MEMCHECK($1);
      $$ = new UVariableList($1,$3);
      MEMCHECK2($$,$3,$1);
    }
;

/* End of grammar */

%%

// The error function that 'bison' calls
void yy::parser::error(const location_type& l, const std::string& what_error)
{
  uparser.error (l, what_error);
  if (globalDelete)
  {
    delete *globalDelete;
    *globalDelete = 0;
  }
}

// Local Variables:
// mode: c++
// End:
