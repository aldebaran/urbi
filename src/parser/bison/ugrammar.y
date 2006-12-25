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
%parse-param {UParser& up}
%lex-param {UParser& up}
%debug

%{
#include "fwd.hh"
#include "utypes.hh"
%}

// Locations.
%locations
%define "filename_type" "const std::string"
%initial-action
{
  // Saved when exiting the start symbol.
  @$ = up.loc_;
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
  // Output in ugrammar.cc.
#include <string>
#include <iostream>
#define TRUE  ufloat(1)
#define FALSE ufloat(0)

#include "libport/ref-pt.hh"

#include "parser/uparser.hh"
#include "ubinary.hh"
#include "ucommand.hh"
#include "uasynccommand.hh"
#include "uconnection.hh"
#include "ugroup.hh"
#include "uobj.hh"
#include "uproperty.hh"
#include "userver.hh"

extern UString** globalDelete;

/* Memory checking macros, used in the command tree building process */

void
memcheck (UParser& up, const void* p)
{
  if (!p)
  {
    up.connection.server->isolate();
    up.connection.server->memoryOverflow = true;
  }
}

template <class T1>
void
memcheck(UParser& up, const void* p, T1*& p1)
{
  if (!p)
  {
    up.connection.server->isolate();
    up.connection.server->memoryOverflow = true;
    delete p1; p1 = 0;
  }
}

template <class T1, class T2>
void
memcheck(UParser& up, const void* p, T1*& p1, T2*& p2)
{
  if (!p)
  {
    up.connection.server->isolate();
    up.connection.server->memoryOverflow = true;
    delete p1; p1 = 0;
    delete p2; p2 = 0;
  }
}

template <class T1, class T2, class T3>
void
memcheck(UParser& up, const void* p, T1*& p1, T2*& p2, T3*& p3)
{
  if (!p)
  {
    up.connection.server->isolate();
    up.connection.server->memoryOverflow = true;
    delete p1; p1 = 0;
    delete p2; p2 = 0;
    delete p3; p3 = 0;
  }
}

template <class T1, class T2, class T3, class T4>
void
memcheck(UParser& up, const void* p, T1*& p1, T2*& p2, T3*& p3, T4*& p4)
{
  if (!p)
  {
    up.connection.server->isolate();
    up.connection.server->memoryOverflow = true;
    delete p1; p1 = 0;
    delete p2; p2 = 0;
    delete p3; p3 = 0;
    delete p4; p4 = 0;
  }
}

/// Direct the call from 'bison' to the scanner in the right UParser.
inline
yy::parser::token_type
yylex(yy::parser::semantic_type* val, yy::location* loc, UParser& up)
{
  return up.scanner_.yylex(val, loc, up);
}


/// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
UCommand*
new_bin(UParser& up,
	const yy::parser::location_type& l, enum UNodeType op,
	UCommand* lhs, UCommand* rhs)
{
  UCommand_TREE*res = new UCommand_TREE(l, op, lhs, rhs);
  if (res)
    res->setTag("__node__");
  memcheck(up, res, lhs, rhs);
  return res;
}

/// A new UExpression of type \c t and child \c t1.
template <class T1>
UExpression*
new_exp (UParser& up, UExpression::Type t, T1* t1)
{
  UExpression* res = new UExpression(t, t1);
  memcheck(up, res, t1);
  return res;
}

/// A new UExpression of type \c t and children \c t1, \c t2.
template <class T1, class T2>
UExpression*
new_exp (UParser& up, UExpression::Type t, T1* t1, T2* t2)
{
  UExpression* res = new UExpression(t, t1, t2);
  memcheck(up, res, t1, t2);
  return res;
}

/// Take the value, free the pointer.
template <class T>
inline
T
take (T* t)
{
  T res(*t);
  delete t;
  return res;
}
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
  TOK_DERIV        "derivation"
  TOK_DERIV2       "second-derivation"
  TOK_DIR          "->"
  TOK_DISINHERIT   "disinherit"
  TOK_DIV          "/"
  TOK_DOLLAR       "$"
  TOK_DOUBLECOLON  "::"
  TOK_ELSE         "else"
  TOK_EMIT         "emit"
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
  TOK_GROUP        "group"
  TOK_GROUPLIST    "group list"
  TOK_IF           "if"
  TOK_IN           "in"
  TOK_INFO         "info"
  TOK_INHERIT      "inherit"
  TOK_LBRACKET     "{"
  TOK_LOOP         "loop"
  TOK_LOOPN        "loopn"
  TOK_LPAREN       "("
  TOK_LSBRACKET    "["
  TOK_MINUS        "-"
  TOK_MINUSASSIGN  "-="
  TOK_MINUSMINUS   "--"
  TOK_MULT         "*"
  TOK_NEW          "new"
  TOK_NOOP         "noop"
  TOK_NORM         "normalized"
  TOK_OBJECT       "object"
  TOK_ONLEAVE      "onleave"
  TOK_ONLY         "only"
  TOK_OROPERATOR   "||"
  TOK_PERCENT      "%"
  TOK_PIPE         "|"
  TOK_PLUS         "+"
  TOK_PLUSASSIGN   "+="
  TOK_PLUSPLUS     "++"
  TOK_POINT        "."
  TOK_RBRACKET     "}"
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
  TOK_ECHO         "echo"
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

%token TOK_EOF 0 "end of command"

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

%type <expr>                expr            "expression"
%type <val>                 timeexpr        "time expression"
%type <command>             taggedcommands  "set of commands"
%type <command>             taggedcommand   "tagged command"
%type <command>             command         "command"
%type <command>             instruction     "instruction"
%type <namedparameters>     parameters      "parameters"
%type <namedparameters>     array           "array"
%type <namedparameters>     parameterlist   "list of parameters"
%type <namedparameters>     rawparameters   "list of attributes"
%type <namedparameters>     namedparameters "list of named parameters"
%type <namedparameters>     flags           "list of flags"
%type <namedparameters>     flags.0.1       "list of flags (0 or 1)"
%type <variablelist>        refvariables    "list of variables"
%type <expr>                softtest        "soft test"
%type <namedparameters>     identifiers     "list of identifiers"
%type <expr>                class_declaration "class declaration"
%type <namedparameters>     class_declaration_list "class declaration list"
%type <binary>              binary          "binary"
%type <property>            property        "property"
%type <variable>            variable        "variable"
%type <variable>            purevariable    "pure variable"
%type <variable>            refvariable     "ref-variable"
//%type  <namedparameters>     purevariables   "list of pure variables"


/*----------------------.
| Operator precedence.  |
`----------------------*/

// FIXME: This is sick!  Puke puke puke.
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

%start start;
start:
  root  { up.loc_ = @$; }
;

root:
    /* Minimal error recovery, so that all the tokens are read,
       especially the end-of-lines, to keep error messages in sync. */
  error
  {
    up.commandTree = 0;
  }

  | refvariable "=" binary ";" {

      libport::RefPt<UBinary> *ref = new libport::RefPt<UBinary>($3);
      memcheck(up, ref);
      UCommand* tmpcmd = new UCommand_ASSIGN_BINARY(@$, $1, ref);
      if (tmpcmd)
	tmpcmd->setTag("__node__");
      memcheck(up, tmpcmd, $1, ref);
      if (tmpcmd)
	up.binaryCommand = true;

      up.commandTree  = new UCommand_TREE(@$, USEMICOLON, tmpcmd, 0);
      if ( up.commandTree )
	up.commandTree->setTag("__node__");
      memcheck(up, up.commandTree);
    }

  | taggedcommands {

      up.commandTree = 0;
      if ($1 && $1->type == UCommand::CMD_TREE)
      {
	up.commandTree = dynamic_cast<UCommand_TREE*> ($1);
	assert (up.commandTree != 0);
      }
      else
	delete $1;
    }
;


/* TAGGEDCOMMANDS */
taggedcommands:
  taggedcommand
| taggedcommands "," taggedcommands { $$ = new_bin(up, @$, UCOMMA, $1, $3); }
| taggedcommands ";" taggedcommands { $$ = new_bin(up, @$, USEMICOLON, $1, $3); }
| taggedcommands "|" taggedcommands { $$ = new_bin(up, @$, UPIPE, $1, $3); }
| taggedcommands "&" taggedcommands { $$ = new_bin(up, @$, UAND, $1, $3);}
;

/* TAGGEDCOMMAND */

taggedcommand:

    command {
      if ($1)
	$1->setTag(UNKNOWN_TAG);

      $$ = $1;
    }

  | "identifier" flags ":" command {

      memcheck(up, $1);
      if ($4)
      {
	$4->setTag($1->str());
	$4->flags = $2;
      }
      $$ = $4;
    }

  | TAG flags ":" command {

      memcheck(up, $1);
      if ($4)
      {
	$4->setTag($1->str());
	$4->flags = $2;
      }
      $$ = $4;
    }

  | "identifier" ":" command {

      memcheck(up, $1);
      if ($3)
      {
	$3->setTag($1->str());
      }
      $$ = $3;
    }

  | TAG ":" command {

      memcheck(up, $1);
      if ($3)
      {
	$3->setTag($1->str());
      }
      $$ = $3;
    }


  | STRUCT ":" command {

      memcheck(up, $1.device);
      memcheck(up, $1.id);
      if ($3)
      {
	$3->setTag(UString($1.device, $1.id).str());
	delete $1.device;
	delete $1.id;
      }
      $$ = $3;
    }

  | STRUCT flags ":" command {

      memcheck(up, $1.device);
      memcheck(up, $1.id);

      if ($4)
      {
	$4->setTag(UString($1.device, $1.id).str());
	delete $1.device;
	delete $1.id;

	$4->flags = $2;
      }
      $$ = $4;
    }

  | flags ":" command {

      memcheck(up, $1);
      if ($3)
      {
	$3->setTag(UNKNOWN_TAG);
	$3->flags = $1;
      }
      $$ = $3;
    }
;


/*--------.
| flags.  |
`--------*/

// FIXME: Why don't we have real std::lists here???
// Right recursion.
flags :
     FLAG flags.0.1  {

      UExpression *flagval = new UExpression(UExpression::VALUE, *$1);
      delete $1;
      memcheck(up, flagval);

      $$ = new UNamedParameters(new UString("flag"), flagval, $2);
      memcheck(up, $$, flagval, $2);
    }

  | FLAGTIME "(" expr ")" flags.0.1 {

      $$ = new UNamedParameters(new UString("flagtimeout"), $3, $5);
      memcheck(up, $$, $3, $5);
    }

  | FLAGID "(" expr ")" flags.0.1 {

      $$ = new UNamedParameters(new UString("flagid"), $3, $5);
      memcheck(up, $$, $3, $5);
    }

  | FLAGTEST "(" softtest ")" flags.0.1 {

      if (*$1 == 6)
	$$ = new UNamedParameters(new UString("flagstop"), $3, $5);
      else
	$$ = new UNamedParameters(new UString("flagfreeze"), $3, $5);
      memcheck(up, $$, $3, $5);
    }

;

// 0 or 1 flags.
flags.0.1:
  /* empty. */ { $$ = 0;  }
| flags        { $$ = $1; }
;

/* COMMAND */

command:

    instruction

  | "{" taggedcommands "}" {

      $$ = new UCommand_TREE(@$, UPIPE, $2, new UCommand_NOOP(@$, true));
      $$->setTag("__UGrouped_set_of_commands__");
      ((UCommand_TREE*)$$)->command2->setTag("__system__");
    }
;

/* INSTRUCTION */

instruction:
  /* empty */ { $$ = 0; } /* FIXME: THIS IS BAD! REMOVE THIS!
			     FIXME: I WHOLEHEARTEDLY AGREE! */

  | "noop" {
    $$ = new UCommand_NOOP(@$);
    memcheck(up, $$);
    }

  | refvariable "=" expr namedparameters {
    $$ = new UCommand_ASSIGN_VALUE(@$, $1, $3, $4, false);
    memcheck(up, $$, $1, $3, $4);
    }

  | refvariable "+=" expr {

    $$ = new UCommand_AUTOASSIGN(@$, $1, $3, 0);
    memcheck(up, $$, $1, $3);
    }

  | refvariable "-=" expr {

    $$ = new UCommand_AUTOASSIGN(@$, $1, $3, 1);
    memcheck(up, $$, $1, $3);
    }


  | "var" refvariable "=" expr namedparameters {

      $2->local_scope = true;
      $$ = new UCommand_ASSIGN_VALUE(@$, $2, $4, $5);
      memcheck(up, $$, $2, $4, $5);
    }

  | property "=" expr {

    $$ = new UCommand_ASSIGN_PROPERTY(@$, $1->variablename, $1->property, $3);
      memcheck(up, $$, $1, $1, $3);
    }

  | expr {

    $$ = new UCommand_EXPR(@$, $1);
      memcheck(up, $$, $1);
    }

  | refvariable NUM {

      // FIXME: Leak
      $$ = new UCommand_DEVICE_CMD(@$, $1, $2);
      memcheck(up, $$, $1);
    }

  | "return" {

    $$ = new UCommand_RETURN(@$, 0);
      memcheck(up, $$);
    }

  | "return" expr {

    $$ = new UCommand_RETURN(@$, $2);
      memcheck(up, $$, $2);
    }

  | "echo" expr namedparameters {

    $$ = new UCommand_ECHO(@$, $2, $3, 0);
      memcheck(up, $$, $2, $3);
    }

  | refvariable "=" "new" "identifier" {

      memcheck(up, $4);
      $$ = new UCommand_NEW(@$, $1, $4, 0, true);
      memcheck(up, $$, $1, $4);
    }

  | refvariable "=" "new" "identifier" "(" parameterlist ")" {

      memcheck(up, $4);
      $$ = new UCommand_NEW(@$, $1, $4, $6);
      memcheck(up, $$, $1, $4, $6);
    }


  | "group" "identifier" "{" identifiers "}" {

    $$ = new UCommand_GROUP(@$, $2, $4);
      memcheck(up, $$, $4, $2);
    }

  | "addgroup" "identifier" "{" identifiers "}" {

    $$ = new UCommand_GROUP(@$, $2, $4, 1);
      memcheck(up, $$, $4, $2);
    }


  | "delgroup" "identifier" "{" identifiers "}" {

    $$ = new UCommand_GROUP(@$, $2, $4, 2);
      memcheck(up, $$, $4, $2);
    }

   /*
  | GROUP "identifier" {

      $$ = new UCommand_GROUP(@$, $2, 0);
      memcheck(up, $$, $2);
    }
*/
  | "group" {

    $$ = new UCommand_GROUP(@$, 0, 0);
      memcheck(up, $$);
    }

  | "alias" purevariable purevariable {

    $$ = new UCommand_ALIAS(@$, $2, $3);
      memcheck(up, $$, $2, $3);
    }

  | purevariable "inherit" purevariable {

    $$ = new UCommand_INHERIT(@$, $1, $3);
      memcheck(up, $$, $1, $3);
    }

  | purevariable "disinherit" purevariable {

    $$ = new UCommand_INHERIT(@$, $1, $3, true);
      memcheck(up, $$, $1, $3);
    }

  | "alias" purevariable {

    $$ = new UCommand_ALIAS(@$, $2, 0);
      memcheck(up, $$, $2);
    }

  | "unalias" purevariable {

    $$ = new UCommand_ALIAS(@$, $2, 0, true);
      memcheck(up, $$, $2);
    }

  | "alias" {

    $$ = new UCommand_ALIAS(@$, 0, 0);
      memcheck(up, $$);
  }

  | OPERATOR {

      memcheck(up, $1);
      $$ = new UCommand_OPERATOR(@$, $1);
      memcheck(up, $$, $1);
    }

  | OPERATOR_ID "identifier" {

      memcheck(up, $1);
      memcheck(up, $2);
      $$ = new UCommand_OPERATOR_ID(@$, $1, $2);
      memcheck(up, $$, $1, $2);
    }

  | OPERATOR_ID TAG {

      memcheck(up, $1);
      memcheck(up, $2);
      $$ = new UCommand_OPERATOR_ID(@$, $1, $2);
      memcheck(up, $$, $1, $2);
    }

  | OPERATOR_ID STRUCT {

      memcheck(up, $1);
      memcheck(up, $2.device);
      memcheck(up, $2.id);
      $$ = new UCommand_OPERATOR_ID(@$, $1, new UString($2.device, $2.id));
      delete $2.device;
      delete $2.id;
      memcheck(up, $$, $1);
    }

  | OPERATOR_VAR variable {

      memcheck(up, $1);
      $$ = new UCommand_OPERATOR_VAR(@$, $1, $2);
      memcheck(up, $$, $1, $2);
    }

  | BINDER TOK_OBJECT purevariable {

      memcheck(up, $1);
      $$ = new UCommand_BINDER(@$, 0, $1, 3, $3);
      memcheck(up, $$, $1, $3);
    }


  | BINDER "var" purevariable "from" purevariable {

      memcheck(up, $1);
      $$ = new UCommand_BINDER(@$, $5, $1, 1, $3);
      memcheck(up, $$, $1, $3, $5);
    }

  | BINDER "function" "(" NUM ")" purevariable "from" purevariable {

      memcheck(up, $1);
      $$ = new UCommand_BINDER(@$, $8, $1, 0, $6, (int)take($4));
      memcheck(up, $$, $1, $6, $8);
    }

  | BINDER "event" "(" NUM ")" purevariable "from" purevariable {

      memcheck(up, $1);
      $$ = new UCommand_BINDER(@$, $8, $1, 2, $6, (int)take ($4));
      memcheck(up, $$, $1, $6, $8);
    }

  | "wait" expr {

    $$ = new UCommand_WAIT(@$, $2);
      memcheck(up, $$, $2);
    }

  | "emit" purevariable {

      $2->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT(@$, $2, 0);
      memcheck(up, $$, $2);
    }

  | "emit" purevariable "(" parameterlist ")" {

      $2->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT(@$, $2, $4);
      memcheck(up, $$, $2, $4);
    }

  | "emit" "(" expr ")" purevariable {

      $5->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT(@$, $5, 0, $3);
      memcheck(up, $$, $5, $3);
    }

  | "emit" "(" expr ")" purevariable "(" parameterlist ")" {

      $5->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT(@$, $5, $7, $3);
      memcheck(up, $$, $5, $7, $3);
    }

  | "emit" "(" ")" purevariable {

      $4->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT(@$, $4, 0, new UExpression(UExpression::VALUE,
						    UINFINITY));
      memcheck(up, $$, $4);
    }

  | "emit" "(" ")" purevariable "(" parameterlist ")" {

      $4->id_type = UDEF_EVENT;
      $$ = new UCommand_EMIT(@$, $4, $6, new UExpression(UExpression::VALUE,
						     UINFINITY));
      memcheck(up, $$, $4, $6);
    }

  | "waituntil" softtest {

    $$ = new UCommand_WAIT_TEST(@$, $2);
      memcheck(up, $$, $2);
    }

  | refvariable TOK_MINUSMINUS {

    $$ = new UCommand_INCDECREMENT(@$, UCommand::CMD_DECREMENT, $1);
      memcheck(up, $$, $1);
    }

  | refvariable TOK_PLUSPLUS {

    $$ = new UCommand_INCDECREMENT(@$, UCommand::CMD_INCREMENT, $1);
      memcheck(up, $$, $1);
    }

  | TOK_DEF {

    $$ = new UCommand_DEF(@$, UDEF_QUERY, 0, 0, 0);
      memcheck(up, $$)
    }

  | "var" refvariable {

      $2->local_scope = true;
      $$ = new UCommand_DEF(@$, UDEF_VAR, $2, 0, 0);
      memcheck(up, $$, $2)
    }

  | TOK_DEF refvariable {

      $2->local_scope = true;
      $$ = new UCommand_DEF(@$, UDEF_VAR, $2, 0, 0);
      memcheck(up, $$, $2)
    }

  | "var" "{" refvariables "}" {

    $$ = new UCommand_DEF(@$, UDEF_VARS, $3);
      memcheck(up, $$, $3)
    }

  | TOK_CLASS "identifier" "{" class_declaration_list "}" {

    $$ = new UCommand_CLASS(@$, $2, $4);
      memcheck(up, $$, $2, $4)
    }

  | TOK_CLASS "identifier" {

    $$ = new UCommand_CLASS(@$, $2, 0);
      memcheck(up, $$, $2)
    }


  | "event" variable "(" identifiers ")" {

      $2->local_scope = true;
      $2->id_type = UDEF_EVENT;
      $$ = new UCommand_DEF(@$, UDEF_EVENT, $2, $4, 0);
      memcheck(up, $$, $2, $4);
    }

  | "event" variable {

      $2->local_scope = true;
      $2->id_type = UDEF_EVENT;
      $$ = new UCommand_DEF(@$, UDEF_EVENT, $2, 0, 0);
      memcheck(up, $$, $2);
    }

  | "function" variable "(" identifiers ")" {

      if (up.connection.functionTag)
      {
	delete $2;
	delete $4;
	$2 = 0;
	delete up.connection.functionTag;
	up.connection.functionTag = 0;
	error(@$, "Nested function def not allowed.");
	YYERROR;
      }
      else
      {
	up.connection.functionTag = new UString("__Funct__");
	globalDelete = &up.connection.functionTag;
      }

    } taggedcommand {

      $2->id_type = UDEF_FUNCTION;
      $$ = new UCommand_DEF(@$, UDEF_FUNCTION, $2, $4, $7);

      memcheck(up, $$, $2, $4);
      if (up.connection.functionTag)
      {
	delete up.connection.functionTag;
	up.connection.functionTag = 0;
	globalDelete = 0;
      }
    }

  | TOK_DEF variable "(" identifiers ")" {

      up.connection.server->debug("Warning: 'def' is deprecated, use"
				       "'function' instead\n");
      if (up.connection.functionTag)
      {
	delete $2;
	delete $4;
	$2 = 0;
	delete up.connection.functionTag;
	up.connection.functionTag = 0;
	error(@$, "Nested function def not allowed.");
	YYERROR;
      }
      else
      {
	up.connection.functionTag = new UString("__Funct__");
	globalDelete = &up.connection.functionTag;
      }

    } taggedcommand {

      $2->id_type = UDEF_FUNCTION;
      $$ = new UCommand_DEF(@$, UDEF_FUNCTION, $2, $4, $7);

      memcheck(up, $$, $2, $4);
      if (up.connection.functionTag)
      {
	delete up.connection.functionTag;
	up.connection.functionTag = 0;
	globalDelete = 0;
      }
    }

  | TOK_IF "(" expr ")" taggedcommand %prec CMDBLOCK {

      if (!$5)
      {
	delete $3;
	delete $5;
	error(@$, "Empty then-part within an if.");
	YYERROR;
      }
      $$ = new UCommand_IF(@$, $3, $5, 0);
      memcheck(up, $$, $3, $5);
    }

  | "if" "(" expr ")" taggedcommand "else" taggedcommand {

      if (!$5)
      {
	delete $3;
	delete $5;
	delete $7;
	error(@$, "Empty then-part within an if.");
	YYERROR;
      }
      $$ = new UCommand_IF(@$, $3, $5, $7);
      memcheck(up, $$, $3, $5, $7);
    }

  | TOK_EVERY "(" expr ")" taggedcommand {

    $$ = new UCommand_EVERY(@$, $3, $5);
      memcheck(up, $$, $3, $5);
    }

  | "timeout" "(" expr ")" taggedcommand {

    $$ = new UCommand_TIMEOUT(@$, $3, $5);
      memcheck(up, $$, $3, $5);
    }

  | "stopif" "(" softtest ")" taggedcommand {

    $$ = new UCommand_STOPIF(@$, $3, $5);
      memcheck(up, $$, $3, $5);
    }

  | TOK_FREEZEIF "(" softtest ")" taggedcommand {

    $$ = new UCommand_FREEZEIF(@$, $3, $5);
      memcheck(up, $$, $3, $5);
    }

  | "at" "(" softtest ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_AT(@$, UCommand::CMD_AT, $3, $5, 0);
      memcheck(up, $$, $3, $5);
    }

  | "at" "(" softtest ")" taggedcommand "onleave" taggedcommand {
      if(!$5)
      {
	delete $3;
	delete $5;
	delete $7;
	error(@$, "Empty body within an at command.");
	YYERROR;
      }
      $$ = new UCommand_AT(@$, UCommand::CMD_AT, $3, $5, $7);
      memcheck(up, $$, $3, $5, $7);
    }

  | "at" "&" "(" softtest ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_AT(@$, UCommand::CMD_AT_AND, $4, $6, 0);
      memcheck(up, $$, $4, $6);
    }

  | "at" "&" "(" softtest ")" taggedcommand "onleave" taggedcommand {
      if(!$6)
      {
	delete $4;
	delete $6;
	delete $8;
	error(@$, "Empty body within an at command.");
	YYERROR;
      }
      $$ = new UCommand_AT(@$, UCommand::CMD_AT_AND, $4, $6, $8);
      memcheck(up, $$, $4, $6, $8);
    }

  | "while" "(" expr ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_WHILE(@$, UCommand::CMD_WHILE, $3, $5);
      memcheck(up, $$, $3, $5);
    }

  | "while" "|" "(" expr ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_WHILE(@$, UCommand::CMD_WHILE_PIPE, $4, $6);
      memcheck(up, $$, $4, $6);
    }

  | "whenever" "(" softtest ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_WHENEVER(@$, $3, $5, 0);
      memcheck(up, $$, $3, $5);
    }

  | "whenever" "(" softtest ")" taggedcommand "else" taggedcommand {
      if(!$5)
      {
	delete $3;
	delete $5;
	delete $7;
	error(@$, "Empty body within a whenever command.");
	YYERROR;
      }
      $$ = new UCommand_WHENEVER(@$, $3, $5, $7);
      memcheck(up, $$, $3, $5, $7);
    }

  | "loop" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_LOOP(@$, $2);
      memcheck(up, $$, $2);
    }

  | "foreach" purevariable "in" expr "{" taggedcommands "}" %prec CMDBLOCK {

    $$ = new UCommand_FOREACH(@$, UCommand::CMD_FOREACH, $2, $4, $6);
      memcheck(up, $$, $2, $4, $6);
    }

  | "foreach" "&" purevariable "in" expr "{" taggedcommands "}" %prec CMDBLOCK {

    $$ = new UCommand_FOREACH(@$, UCommand::CMD_FOREACH_AND, $3, $5, $7);
      memcheck(up, $$, $3, $5, $7);
    }

  | "foreach" "|" purevariable "in" expr "{" taggedcommands "}" %prec CMDBLOCK {

    $$ = new UCommand_FOREACH(@$, UCommand::CMD_FOREACH_PIPE, $3, $5, $7);
      memcheck(up, $$, $3, $5, $7);
    }

  | "loopn" "(" expr ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_LOOPN(@$, UCommand::CMD_LOOPN, $3, $5);
      memcheck(up, $$, $3, $5);
    }

  | "loopn" "|" "(" expr ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_LOOPN(@$, UCommand::CMD_LOOPN_PIPE, $4, $6);
      memcheck(up, $$, $4, $6);
    }

  | "loopn" "&" "(" expr ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_LOOPN(@$, UCommand::CMD_LOOPN_AND, $4, $6);
      memcheck(up, $$, $4, $6);
    }

  | "for" "(" instruction ";"
	       expr ";"
	       instruction ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_FOR(@$, UCommand::CMD_FOR, $3, $5, $7, $9);
      memcheck(up, $$, $3, $5, $7, $9);
    }

  | "for" "|" "(" instruction ";"
		    expr ";"
		    instruction ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_FOR(@$, UCommand::CMD_FOR_PIPE, $4, $6, $8, $10);
      memcheck(up, $$, $4, $6, $8, $10);
    }

  | "for" "&" "(" instruction ";"
		   expr ";"
		   instruction ")" taggedcommand %prec CMDBLOCK {

    $$ = new UCommand_FOR(@$, UCommand::CMD_FOR_AND, $4, $6, $8, $10);
      memcheck(up, $$, $4, $6, $8, $10);
    }
;


/* ARRAY */

array:

  /* empty */ { $$ = 0 }

  | "[" expr "]" array {

      $$ = new UNamedParameters($2, $4);
      memcheck(up, $$, $2, $4);
    }
;


/* VARID, PUREVARIABLE, VARIABLE, REFVARIABLE */

purevariable:

    "$" "(" expr ")" {

      $$ = new UVariableName($3);
      memcheck(up, $$, $3);
    }

  | "identifier" array TOK_POINT "identifier" array {

      memcheck(up, $1);
      memcheck(up, $4);
      $$ = new UVariableName($1, $2, $4, $5);
      memcheck(up, $$, $1, $2, $4, $5);
    }

  | "identifier" "::" "identifier" {

      memcheck(up, $1);
      memcheck(up, $3);
      $$ = new UVariableName($1, $3, true, 0);
      if ($$) $$->doublecolon = true;
      memcheck(up, $$, $1, $3);
    }

  | "identifier" array {

      memcheck(up, $1);
      if (up.connection.functionTag)
	// We are inside a function
	  $$ = new UVariableName(new UString(up.connection.functionTag),
				 $1, false, $2);
      else
	$$ = new UVariableName(new UString(up.connection.connectionTag),
			       $1, false, $2);
      memcheck(up, $$, $1, $2);
      $$->nostruct = true;
    }

  | STRUCT array {

      memcheck(up, $1.device);
      memcheck(up, $1.id);
      $$ = new UVariableName($1.device, $1.id, false, $2);
      memcheck(up, $$, $1.device, $1.id, $2);
    }

;

variable:
  purevariable			{ $$ = $1;				}
| "static" purevariable		{ $$ = $2; $$->isstatic = true;		}
| purevariable TOK_NORM		{ $$ = $1; $$->isnormalized = true;	}
| purevariable TOK_VARERROR	{ $$ = $1; $$->varerror = true;		}
| purevariable TOK_VARIN	{ $$ = $1; $$->varin = true;		}
| purevariable TOK_VAROUT	{ $$ = $1;				}
| purevariable TOK_DERIV	{ $$ = $1; $$->deriv = UDERIV;		}
| purevariable TOK_DERIV2	{ $$ = $1; $$->deriv = UDERIV2;		}
| purevariable TOK_TRUEDERIV	{ $$ = $1; $$->deriv = UTRUEDERIV;	}
| purevariable TOK_TRUEDERIV2	{ $$ = $1; $$->deriv = UTRUEDERIV2;	}
;

refvariable:
    variable {

      $$ = $1;
    }

  | "only" variable {

      $$ = $2;
      $$->rooted = true;
  }
;


/* PROPERTY */

property:

    purevariable "->" "identifier" {

      $$ = new UProperty($1, $3);
      memcheck(up, $$, $1, $3);
    }
;


/* NAMEDPARAMETERS */

namedparameters:
  /* empty */ { $$ = 0 }

  | "identifier" ":" expr namedparameters {

      memcheck(up, $1);
      $$ = new UNamedParameters($1, $3, $4);
      memcheck(up, $$, $1, $4, $3);
    }
;


/* BINARY */

binary:
    TOK_BIN NUM {
      $$ = new UBinary((int)take($2), 0);
      memcheck(up, $$);
      if ($$ != 0)
	memcheck(up, $$->buffer, $$);
    }

  | TOK_BIN NUM rawparameters {

      $$ = new UBinary((int)take($2), $3);
      memcheck(up, $$, $3);
      if ($$ != 0)
	memcheck(up, $$->buffer, $$, $3);
    }
;


/* TIMEEXPR */

timeexpr:
 TIMEVALUE           { $$ = $1;  }
| timeexpr TIMEVALUE { $$ = new ufloat(take($1)+take($2));  }
;


/* EXPR */

expr:
  NUM {
    $$ = new UExpression(UExpression::VALUE, take($1));
    memcheck(up, $$);
  }

| timeexpr {
    $$ = new UExpression(UExpression::VALUE, take ($1));
    memcheck(up, $$);
  }

| STRING {
    memcheck(up, $1);
    $$ = new UExpression(UExpression::VALUE, $1);
    memcheck(up, $$, $1);
  }

| "[" parameterlist "]" {

    $$ = new UExpression(UExpression::LIST, $2);
    memcheck(up, $$, $2);
  }

| property {

     $$ = new UExpression(UExpression::PROPERTY,
			    $1->property, $1->variablename);
     memcheck(up, $$, $1);
  }

| refvariable "(" parameterlist ")"  {

    //if (($1) && ($1->device) &&
    //    ($1->device->equal(up.connection.functionTag)))
    //  $1->nameUpdate(up.connection.connectionTag->str(),
    //                 $1->id->str());

    $1->id_type = UDEF_FUNCTION;
    $$ = new_exp(up, UExpression::FUNCTION, $1, $3);
  }

| "%" variable		{ $$ = new_exp(up, UExpression::ADDR_VARIABLE, $2); }
| variable		{ $$ = new_exp(up, UExpression::VARIABLE, $1);      }
| "group" "identifier"	{ $$ = new_exp(up, UExpression::GROUP, $2);         }
;


  /* num expr */
expr:
    expr "+" expr	{ $$ = new_exp(up, UExpression::PLUS, $1, $3); }
  | expr "-" expr	{ $$ = new_exp(up, UExpression::MINUS, $1, $3); }
  | expr "*" expr	{ $$ = new_exp(up, UExpression::MULT, $1, $3); }
  | expr "/" expr	{ $$ = new_exp(up, UExpression::DIV,  $1, $3); }
  | expr "%" expr	{ $$ = new_exp(up, UExpression::MOD,  $1, $3); }
  | expr "^" expr	{ $$ = new_exp(up, UExpression::EXP,  $1, $3); }

  | TOK_COPY expr  %prec NEG {

      $$ = new UExpression(UExpression::COPY, $2, 0);
      memcheck(up, $$, $2);
    }

  | "-" expr %prec NEG {

      $$ = new UExpression(UExpression::NEG, $2, 0);
      memcheck(up, $$, $2);
    }

  | "(" expr ")" {

      $$ = $2;
    }
;


  /*--------.
  | Tests.  |
  `--------*/
%token
  TOK_EQ   "=="
  TOK_GT   ">"
  TOK_LE   "<="
  TOK_LT   "<"
  TOK_PEQ  "%="
  TOK_NE   "!="
  TOK_GE   ">="
  TOK_DEQ  "=~="
  TOK_REQ  "~="
;

expr:
    "true"  { $$ = new UExpression(UExpression::VALUE, TRUE);  }
  | "false" { $$ = new UExpression(UExpression::VALUE, FALSE); }

  | expr "=="  expr { $$ = new_exp(up, UExpression::TEST_EQ,  $1, $3); }
  | expr "~="  expr { $$ = new_exp(up, UExpression::TEST_REQ, $1, $3); }
  | expr "=~=" expr { $$ = new_exp(up, UExpression::TEST_DEQ, $1, $3); }
  | expr "%="  expr { $$ = new_exp(up, UExpression::TEST_PEQ, $1, $3); }
  | expr "!="  expr { $$ = new_exp(up, UExpression::TEST_NE,  $1, $3); }
  | expr ">"   expr { $$ = new_exp(up, UExpression::TEST_GT,  $1, $3); }
  | expr ">="  expr { $$ = new_exp(up, UExpression::TEST_GE,  $1, $3); }
  | expr "<"   expr { $$ = new_exp(up, UExpression::TEST_LT,  $1, $3); }
  | expr "<="  expr { $$ = new_exp(up, UExpression::TEST_LE,  $1, $3); }

  | "!" expr {
      $$ = new UExpression(UExpression::TEST_BANG, $2, 0);
      memcheck(up, $$, $2);
    }

  | expr "&&" expr { $$ = new_exp(up, UExpression::TEST_AND, $1, $3); }
  | expr "||" expr { $$ = new_exp(up, UExpression::TEST_OR,  $1, $3); }

;


/* PARAMETERLIST, PARAMETERS, PARAMETERSERIES */

parameterlist:
  /* empty */ { $$ = 0; }

  | parameters
;

parameters:
    expr {

      $$ = new UNamedParameters($1);
      memcheck(up, $$, $1);
    }

  | expr "," parameters {

      $$ = new UNamedParameters($1, $3);
      memcheck(up, $$, $1, $3);
    }
;

rawparameters:
    NUM {
      UExpression *expr = new UExpression(UExpression::VALUE, take($1));
      $$ = new UNamedParameters(expr);
      memcheck(up, $$, expr);
    }

  | "identifier" {

      UExpression *expr = new UExpression(UExpression::VALUE, $1);
      $$ = new UNamedParameters(expr);
      memcheck(up, $$, expr);
    }

  |  NUM rawparameters {

      UExpression *expr = new UExpression(UExpression::VALUE, take($1));
      $$ = new UNamedParameters(expr, $2);
      memcheck(up, $$, $2, expr);
    }

  |  "identifier" rawparameters {

      UExpression *expr = new UExpression(UExpression::VALUE, $1);
      $$ = new UNamedParameters(expr, $2);
      memcheck(up, $$, $2, expr);
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

      memcheck(up, $1);
      $$ = new UNamedParameters($1, 0);
      memcheck(up, $$, $1);
    }

  | "var" "identifier" {

      memcheck(up, $2);
      $$ = new UNamedParameters($2, 0);
      memcheck(up, $$, $2);
    }


  | "identifier" "," identifiers {

      memcheck(up, $1);
      $$ = new UNamedParameters($1, 0, $3);
      memcheck(up, $$, $3, $1);
    }

  | "var" "identifier" "," identifiers {

      memcheck(up, $2);
      $$ = new UNamedParameters($2, 0, $4);
      memcheck(up, $$, $4, $2);
    }

;

/* CLASS_DELCARATION & CLASS_DECLARATION_LIST */

class_declaration:

    "var" "identifier" {

      memcheck(up, $2);
      $$ = new UExpression(UExpression::VALUE, $2);
      memcheck(up, $$, $2);
    }

  | "function" variable "(" identifiers ")" {
      $2->id_type = UDEF_FUNCTION;
      $$ = new_exp(up, UExpression::FUNCTION, $2, $4);
    }

  | "function" variable {
      $2->id_type = UDEF_FUNCTION;
      $$ = new UExpression(UExpression::FUNCTION, $2,
			   static_cast<UNamedParameters*> (0));
      memcheck(up, $$, $2);
    }

  | "event" variable "(" identifiers ")" {
      $2->id_type = UDEF_EVENT;
      $$ = new_exp(up, UExpression::EVENT, $2, $4);
    }

  | "event" variable {
      $2->id_type = UDEF_EVENT;
      $$ = new UExpression(UExpression::EVENT, $2,
			   static_cast<UNamedParameters*> (0));
      memcheck(up, $$, $2);
    }
;


class_declaration_list:
  /* empty */  { $$ = 0; }

  | class_declaration {
      $$ = new UNamedParameters($1, 0);
      memcheck(up, $$, $1);
    }

  | class_declaration ";" class_declaration_list {
      $$ = new UNamedParameters($1, $3);
      memcheck(up, $$, $3, $1);
    }
;

/* REFVARIABLES */

refvariables:
  /* empty */  { $$ = 0; }

  | refvariable {
      memcheck(up, $1);
      $$ = new UVariableList($1);
      memcheck(up, $$, $1);
    }

  | refvariable ";" refvariables {
      memcheck(up, $1);
      $$ = new UVariableList($1, $3);
      memcheck(up, $$, $3, $1);
    }
;

/* End of grammar */

%%

// The error function that 'bison' calls
void yy::parser::error(const location_type& l, const std::string& m)
{
  up.error (l, m);
  if (globalDelete)
  {
    delete *globalDelete;
    *globalDelete = 0;
  }
}

// Local Variables:
// mode: c++
// End:
