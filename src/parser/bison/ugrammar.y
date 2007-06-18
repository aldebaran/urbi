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

%expect 0
%require "2.2"
%error-verbose
%defines
%skeleton "lalr1.cc"
%parse-param {UParser& up}
%lex-param {UParser& up}
%debug

%{
#include "kernel/fwd.hh"
#include "kernel/utypes.hh"
#include "flavorable.hh"
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

  Flavorable::UNodeType   flavor;
  ufloat                   *val;
  UString                  *ustr;
  std::string		   *str;
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
#include <sstream>

#include "libport/ref-pt.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "parser/uparser.hh"
#include "ubinary.hh"
#include "ucommand.hh"
#include "uasynccommand.hh"
#include "ugroup.hh"
#include "unamedparameters.hh"
#include "uobj.hh"
#include "uproperty.hh"
#include "uvariablename.hh"
#include "uvariablelist.hh"

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

/// Whether the \a command was the empty command.
bool
spontaneous (const UCommand& u)
{
  const UCommand_NOOP* noop = dynamic_cast<const UCommand_NOOP*>(&u);
  return noop && noop->is_spontaneous();
}

/// Issue a warning.
void
warn (UParser& up, const yy::parser::location_type& l, const std::string& m)
{
  std::ostringstream o;
  o << "!!! " << l << ": " << m << "\n" << std::ends;
  up.connection.send(o.str().c_str(), "warning");
}

/// Complain if \a command is not spontaneous.
void
warn_spontaneous(UParser& up,
		 const yy::parser::location_type& l, const UCommand& u)
{
  if (spontaneous(u))
    warn (up, l,
	  "implicit empty instruction.  "
	  "Use 'noop' to make it explicit.");
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
	const yy::parser::location_type& l, Flavorable::UNodeType op,
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
new_exp (UParser& up, const yy::parser::location_type& l,
	 UExpression::Type t, T1* t1)
{
  UExpression* res = new UExpression(l, t, t1);
  memcheck(up, res, t1);
  return res;
}

/// A new UExpression of type \c t and children \c t1, \c t2.
template <class T1, class T2>
UExpression*
new_exp (UParser& up, const yy::parser::location_type& l,
	 UExpression::Type t, T1* t1, T2* t2)
{
  UExpression* res = new UExpression(l, t, t1, t2);
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
  TOK_COPY         "copy"
  TOK_DEF          "def"
  TOK_DELGROUP     "delgroup"
  TOK_DERIV        "'"
  TOK_DERIV2       "''"
  TOK_DIR          "->"
  TOK_DISINHERITS  "disinherits"
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
  TOK_INHERITS     "inherits"
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
  TOK_NORM         "'n"
  TOK_OBJECT       "object"
  TOK_ONLEAVE      "onleave"
  TOK_OROPERATOR   "||"
  TOK_PERCENT      "%"
  TOK_PLUS         "+"
  TOK_PLUSASSIGN   "+="
  TOK_PLUSPLUS     "++"
  TOK_POINT        "."
  TOK_RBRACKET     "}"
  TOK_RETURN       "return"
  TOK_RPAREN       ")"
  TOK_RSBRACKET    "]"
  TOK_STATIC       "static"
  TOK_STOP         "stop"
  TOK_STOPIF       "stopif"
  TOK_SUBCLASS     "subclass"
  TOK_TILDE        "~"
  TOK_TIMEOUT      "timeout"
  TOK_TRUECONST    "true"
  TOK_TRUEDERIV    "'d"
  TOK_TRUEDERIV2   "''d"
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

%token
  <flavor> TOK_COMMA        ","
  <flavor> TOK_SEMICOLON    ";"
  <flavor> TOK_AND          "&"
  <flavor> TOK_PIPE         "|"
;

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
   <ustr>  IDENTIFIER         "identifier"
   <ustr>  TAG                "tag"
   <ustr>  STRING             "string"
   <ustr>  SWITCH             "switch"
   <ustr>  BINDER             "binder"
   <ustr>  OPERATOR           "operator command"
   <ustr>  OPERATOR_ID        "operator"
   <ustr>  OPERATOR_ID_PARAM  "param-operator"
   <ustr>  OPERATOR_VAR       "var-operator"
%type <ustr> tag "any kind of tag"
// FIXME: Simplify once Bison 2.4 is out.
%printer { debug_stream() << *$$; }
   "identifier" TAG STRING SWITCH BINDER OPERATOR OPERATOR_ID
   OPERATOR_ID_PARAM OPERATOR_VAR tag;

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
%type <namedparameters>     flag            "a flag"
%type <namedparameters>     flags.0         "zero or more flags"
%type <namedparameters>     flags.1         "one or more flags"
%type <variablelist>        variables       "list of variables"
%type <expr>                softtest        "soft test"
%type <namedparameters>     identifiers     "list of identifiers"
%type <expr>                class_declaration "class declaration"
%type <namedparameters>     class_declaration_list "class declaration list"
%type <binary>              binary          "binary"
%type <property>            property        "property"
%type <variable>            variable        "variable"
%type <variable>            purevariable    "pure variable"
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
    // FIXME: We should probably free it.
    up.commandTree = 0;
  }

  | variable "=" binary ";" {

      // FIXME: A pointer to a ref-pointer?  Sounds absurd.
      libport::RefPt<UBinary> *ref = new libport::RefPt<UBinary>($3);
      memcheck(up, ref);
      UCommand* c = new UCommand_ASSIGN_BINARY(@$, $1, ref);
      if (c)
	c->setTag("__node__");
      memcheck(up, c, $1, ref);
      if (c)
	up.binaryCommand = true;

      up.commandTree = new UCommand_TREE(@$, Flavorable::USEMICOLON, c, 0);
      if (up.commandTree)
	up.commandTree->setTag("__node__");
      memcheck(up, up.commandTree);
    }

  | taggedcommands {

      up.commandTree = 0;
      if ($1 && $1->type == UCommand::TREE)
      {
	up.commandTree = dynamic_cast<UCommand_TREE*> ($1);
	assert (up.commandTree != 0);
      }
      else
	delete $1;
    }
;



/*-----------------.
| taggedcommands.  |
`-----------------*/

taggedcommands:
  taggedcommand
| taggedcommands "," taggedcommands { $$ = new_bin(up, @$, $2, $1, $3); }
| taggedcommands ";" taggedcommands { $$ = new_bin(up, @$, $2, $1, $3); }
| taggedcommands "|" taggedcommands { $$ = new_bin(up, @$, $2, $1, $3); }
| taggedcommands "&" taggedcommands { $$ = new_bin(up, @$, $2, $1, $3); }
;

/*----------------.
| taggedcommand.  |
`----------------*/

// FIXME: I still use UString here, but that's really stupid, and
// leaking everywhere.  Should be fixed later.
tag:
  "identifier" { $$ = $1; }
| TAG          { $$ = $1; }
| STRUCT       {
		 memcheck(up, $1.device);
		 memcheck(up, $1.id);
		 // FIXME: This is stupid, args should not be given as pointers.
		 $$ = new UString(*$1.device, *$1.id);
		 delete $1.device;
		 delete $1.id;
		}
;

taggedcommand:

    command {
      if ($1)
	$1->setTag(UNKNOWN_TAG);
      $$ = $1;
    }

  | tag flags.0 ":" command {

      memcheck(up, $1);
      if ($4)
      {
	$4->setTag($1->c_str());
	delete $1;
	$4->flags = $2;
      }
      $$ = $4;
    }

  | flags.1 ":" command {

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

flag:
  FLAG
  {
    UExpression *flagval = new UExpression(@$, UExpression::VALUE, take($1));
    memcheck(up, flagval);
    $$ = new UNamedParameters(new UString("flag"), flagval);
    if (flagval->val == 1 || flagval->val == 3) // +report or +end flag
      $$->notifyEnd = true;
    if (flagval->val == 11)
      $$->notifyFreeze = true;
    memcheck(up, $$, flagval);
  }

| FLAGTIME "(" expr ")"
  {
    $$ = new UNamedParameters(new UString("flagtimeout"), $3);
    memcheck(up, $$, $3);
  }

| FLAGID "(" expr ")"
  {
    $$ = new UNamedParameters(new UString("flagid"), $3);
    memcheck(up, $$, $3);
  }

| FLAGTEST "(" softtest ")"
  {
    $$ = new UNamedParameters(new UString(*$1 == 6 ? "flagstop" : "flagfreeze"),
			      $3);
    memcheck(up, $$, $3);
  }
;

// One or more "flag"s.
flags.1:
  flag             { $$ = $1;       }
| flags.1 flag     { $1->next = $2;
		     if ($2->notifyEnd)
		       $1->notifyEnd = true; // propagate the +end flag optim
                     if ($2->notifyFreeze)
                       $1->notifyFreeze = true; // propagate the +freeze flag
		   }
;

// Zero or more "flag"s.
flags.0:
  /* empty. */   { $$ = 0; }
| flags.1        { $$ = $1; }
;



/*----------.
| command.  |
`----------*/

command:

    instruction

  | "{" taggedcommands "}" {

      UCommand_TREE* res =
	new UCommand_TREE(@$, Flavorable::UPIPE, $2,
			  new UCommand_NOOP(@$, UCommand_NOOP::zerotime));
      res->groupOfCommands = true;
      res->setTag("__UGrouped_set_of_commands__");
      res->command2->setTag("__system__");
      $$ = res;
    }
;


/*----------.
| flavors.  |
`----------*/
%type <flavor> and.opt flavor.opt pipe.opt;
%printer { debug_stream() << $$; } and.opt flavor.opt pipe.opt ";" "|" "&" ",";

// One or zero "&", defaulting to ";".
and.opt:
  /* empty. */  { $$ = Flavorable::USEMICOLON; }
| "&"
;

// One or zero "&" or "|", defaulting to ";".
flavor.opt:
  /* empty. */  { $$ = Flavorable::USEMICOLON; }
| "|"
| "&"
;

// One or zero "|", defaulting to ";".
pipe.opt:
  /* empty. */  { $$ = Flavorable::USEMICOLON; }
| "|"
;


/*--------------.
| Instruction.  |
`--------------*/

instruction:
  /* empty */
  {
    $$ = new UCommand_NOOP(@$, UCommand_NOOP::spontaneous);
  }

  | "noop"
  {
    $$ = new UCommand_NOOP(@$);
    memcheck(up, $$);
  }

  | variable "=" expr namedparameters {
    $$ = new UCommand_ASSIGN_VALUE(@$, $1, $3, $4, false);
    memcheck(up, $$, $1, $3, $4);
    }

  | variable "+=" expr {

    $$ = new UCommand_AUTOASSIGN(@$, $1, $3, 0);
    memcheck(up, $$, $1, $3);
    }

  | variable "-=" expr {

    $$ = new UCommand_AUTOASSIGN(@$, $1, $3, 1);
    memcheck(up, $$, $1, $3);
    }


  | "var" variable "=" expr namedparameters {

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

  | variable NUM {

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

  | variable "=" "new" "identifier" {

      memcheck(up, $4);
      $$ = new UCommand_NEW(@$, $1, $4, 0, true);
      memcheck(up, $$, $1, $4);
    }

  | variable "=" "new" "identifier" "(" parameterlist ")" {

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

  | purevariable "inherits" purevariable {

    $$ = new UCommand_INHERIT(@$, $1, $3);
      memcheck(up, $$, $1, $3);
    }

  | purevariable "disinherits" purevariable {

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

  | OPERATOR_ID tag {

      memcheck(up, $1);
      memcheck(up, $2);
      $$ = new UCommand_OPERATOR_ID(@$, $1, $2);
      memcheck(up, $$, $1, $2);
    }

  | OPERATOR_VAR variable {

      memcheck(up, $1);
      $$ = new UCommand_OPERATOR_VAR(@$, $1, $2);
      memcheck(up, $$, $1, $2);
    }

  | BINDER "object" purevariable {

      memcheck(up, $1);
      $$ = new UCommand_BINDER(@$, 0, $1, UBIND_OBJECT, $3);
      memcheck(up, $$, $1, $3);
    }


  | BINDER "var" purevariable "from" purevariable {

      memcheck(up, $1);
      $$ = new UCommand_BINDER(@$, $5, $1, UBIND_VAR, $3);
      memcheck(up, $$, $1, $3, $5);
    }

  | BINDER "function" "(" NUM ")" purevariable "from" purevariable {

      memcheck(up, $1);
      $$ = new UCommand_BINDER(@$, $8, $1, UBIND_FUNCTION, $6, (int)take($4));
      memcheck(up, $$, $1, $6, $8);
    }

  | BINDER "event" "(" NUM ")" purevariable "from" purevariable {

      memcheck(up, $1);
      $$ = new UCommand_BINDER(@$, $8, $1, UBIND_EVENT, $6, (int)take ($4));
      memcheck(up, $$, $1, $6, $8);
    }

  | "wait" expr {

    $$ = new UCommand_WAIT(@$, $2);
      memcheck(up, $$, $2);
    }

  | "emit" purevariable {

      $$ = new UCommand_EMIT(@$, $2, 0);
      memcheck(up, $$, $2);
    }

  | "emit" purevariable "(" parameterlist ")" {

      $$ = new UCommand_EMIT(@$, $2, $4);
      memcheck(up, $$, $2, $4);
    }

  | "emit" "(" expr ")" purevariable {

      $$ = new UCommand_EMIT(@$, $5, 0, $3);
      memcheck(up, $$, $5, $3);
    }

  | "emit" "(" expr ")" purevariable "(" parameterlist ")" {

      $$ = new UCommand_EMIT(@$, $5, $7, $3);
      memcheck(up, $$, $5, $7, $3);
    }

  | "emit" "(" ")" purevariable {

      $$ = new UCommand_EMIT(@$, $4, 0, new UExpression(@$, UExpression::VALUE,
							UINFINITY));
      memcheck(up, $$, $4);
    }

  | "emit" "(" ")" purevariable "(" parameterlist ")" {

      $$ = new UCommand_EMIT(@$, $4, $6, new UExpression(@$, UExpression::VALUE,
							 UINFINITY));
      memcheck(up, $$, $4, $6);
    }

  | "waituntil" softtest {

      $$ = new UCommand_WAIT_TEST(@$, $2);
      memcheck(up, $$, $2);
    }

  | variable "--" {

      $$ = new UCommand_INCDECREMENT(@$, UCommand::DECREMENT, $1);
      memcheck(up, $$, $1);
    }

  | variable "++" {

      $$ = new UCommand_INCDECREMENT(@$, UCommand::INCREMENT, $1);
      memcheck(up, $$, $1);
    }

  | "def" {

      $$ = new UCommand_DEF(@$, UCommand_DEF::UDEF_QUERY, 0, 0, 0);
      memcheck(up, $$);
    }

  | "var" variable {

      $2->local_scope = true;
      $$ = new UCommand_DEF(@$, UCommand_DEF::UDEF_VAR, $2, 0, 0);
      memcheck(up, $$, $2);
    }

  | "def" variable {

      $2->local_scope = true;
      $$ = new UCommand_DEF(@$, UCommand_DEF::UDEF_VAR, $2, 0, 0);
      memcheck(up, $$, $2);
    }

  | "var" "{" variables "}" {

    $$ = new UCommand_DEF(@$, UCommand_DEF::UDEF_VARS, $3);
    memcheck(up, $$, $3);
    }

  | "class" "identifier" "{" class_declaration_list "}" {

    $$ = new UCommand_CLASS(@$, $2, $4);
    memcheck(up, $$, $2, $4);
    }

  | "class" "identifier" {

    $$ = new UCommand_CLASS(@$, $2, 0);
    memcheck(up, $$, $2);
    }


  | "event" variable "(" identifiers ")" {

      $2->local_scope = true;
      $$ = new UCommand_DEF(@$, UCommand_DEF::UDEF_EVENT, $2, $4, 0);
      memcheck(up, $$, $2, $4);
    }

  | "event" variable {

      $2->local_scope = true;
      $$ = new UCommand_DEF(@$, UCommand_DEF::UDEF_EVENT, $2, 0, 0);
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

      $$ = new UCommand_DEF(@$, UCommand_DEF::UDEF_FUNCTION, $2, $4, $7);

      memcheck(up, $$, $2, $4);
      if (up.connection.functionTag)
      {
	delete up.connection.functionTag;
	up.connection.functionTag = 0;
	globalDelete = 0;
      }
    }

  | "def" variable "(" identifiers ")" {

      warn (up, @$, "'def' is deprecated, use 'function' instead");
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

      $$ = new UCommand_DEF(@$, UCommand_DEF::UDEF_FUNCTION, $2, $4, $7);

      memcheck(up, $$, $2, $4);
      if (up.connection.functionTag)
      {
	delete up.connection.functionTag;
	up.connection.functionTag = 0;
	globalDelete = 0;
      }
    }

  | "if" "(" expr ")" taggedcommand %prec CMDBLOCK
    {
      warn_spontaneous(up, @5, *$5);
      $$ = new UCommand_IF(@$, $3, $5, 0);
      memcheck(up, $$, $3, $5);
    }

  | "if" "(" expr ")" taggedcommand "else" taggedcommand
    {
      warn_spontaneous(up, @5, *$5);
      $$ = new UCommand_IF(@$, $3, $5, $7);
      memcheck(up, $$, $3, $5, $7);
    }

  | "every" "(" expr ")" taggedcommand {

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

  | "freezeif" "(" softtest ")" taggedcommand {

    $$ = new UCommand_FREEZEIF(@$, $3, $5);
      memcheck(up, $$, $3, $5);
    }

  | "at" and.opt "(" softtest ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_AT(@$,  $2, $4, $6, 0);
      memcheck(up, $$, $4, $6);
    }

  | "at" and.opt "(" softtest ")" taggedcommand "onleave" taggedcommand {
     warn_spontaneous(up, @6, *$6);
     $$ = new UCommand_AT(@$, $2, $4, $6, $8);
     memcheck(up, $$, $4, $6, $8);
    }

  | "while" pipe.opt "(" expr ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_WHILE(@$, $2, $4, $6);
      memcheck(up, $$, $4, $6);
    }

  | "whenever" "(" softtest ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_WHENEVER(@$, $3, $5, 0);
      memcheck(up, $$, $3, $5);
    }

  | "whenever" "(" softtest ")" taggedcommand "else" taggedcommand {
      warn_spontaneous(up, @5, *$5);
      $$ = new UCommand_WHENEVER(@$, $3, $5, $7);
      memcheck(up, $$, $3, $5, $7);
    }

  | "loop" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOP(@$, $2);
      memcheck(up, $$, $2);
    }

  | "foreach" flavor.opt purevariable "in" expr "{" taggedcommands "}"
     %prec CMDBLOCK {

      $$ = new UCommand_FOREACH(@$, $2, $3, $5, $7);
      memcheck(up, $$, $3, $5, $7);
    }

  | "loopn" flavor.opt "(" expr ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_LOOPN(@$, $2, $4, $6);
      memcheck(up, $$, $4, $6);
    }

  | "for" flavor.opt "(" instruction ";"
			 expr ";"
			 instruction ")" taggedcommand %prec CMDBLOCK {

      $$ = new UCommand_FOR(@$, $2, $4, $6, $8, $10);
      memcheck(up, $$, $4, $6, $8, $10);
    }
;


/* ARRAY */

array:

  /* empty */ { $$ = 0; }

  | "[" expr "]" array {

      $$ = new UNamedParameters($2, $4);
      memcheck(up, $$, $2, $4);
    }
;


/* VARID, PUREVARIABLE, VARIABLE, VARIABLE */

purevariable:

    "$" "(" expr ")" {

      $$ = new UVariableName($3);
      memcheck(up, $$, $3);
    }

  | "identifier" array "." "identifier" array {

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
      $$ = new UVariableName(new UString(up.connection.functionTag
					 ? *up.connection.functionTag
					 : *up.connection.connectionTag),
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
  purevariable		{ $$ = $1;				}
| "static" purevariable	{ $$ = $2; $$->isstatic = true;		}
| purevariable "'n"	{ $$ = $1; $$->isnormalized = true;	}
| purevariable "'e"	{ $$ = $1; $$->varerror = true;		}
| purevariable "'in"	{ $$ = $1; $$->varin = true;		}
| purevariable "'out"	{ $$ = $1;				}
| purevariable "'"	{ $$ = $1; $$->deriv = UVariableName::UDERIV;	  }
| purevariable "''"	{ $$ = $1; $$->deriv = UVariableName::UDERIV2;	  }
| purevariable "'d"	{ $$ = $1; $$->deriv = UVariableName::UTRUEDERIV; }
| purevariable "'dd"	{ $$ = $1; $$->deriv = UVariableName::UTRUEDERIV2;}
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
  /* empty */ { $$ = 0; }

  | "identifier" ":" expr namedparameters {

      memcheck(up, $1);
      $$ = new UNamedParameters($1, $3, $4);
      memcheck(up, $$, $1, $4, $3);
    }
;


/* BINARY */

binary:
    "bin" NUM {
      $$ = new UBinary((int)take($2), 0);
      memcheck(up, $$);
      if ($$ != 0)
	memcheck(up, $$->buffer, $$);
    }

  | "bin" NUM rawparameters {

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
    $$ = new UExpression(@$, UExpression::VALUE, take($1));
    memcheck(up, $$);
  }

| timeexpr {
    $$ = new UExpression(@$, UExpression::VALUE, take ($1));
    memcheck(up, $$);
  }

| STRING {
    memcheck(up, $1);
    $$ = new UExpression(@$, UExpression::VALUE, $1);
    memcheck(up, $$, $1);
  }

| "[" parameterlist "]" {

    $$ = new UExpression(@2, UExpression::LIST, $2);
    memcheck(up, $$, $2);
  }

| property {

    $$ = new UExpression(@$, UExpression::PROPERTY,
			 $1->property, $1->variablename);
     memcheck(up, $$, $1);
  }

| variable "(" parameterlist ")"  {

    //if (($1) && ($1->device) &&
    //    ($1->device->equal(up.connection.functionTag)))
    //  $1->nameUpdate(up.connection.connectionTag->c_str(),
    //                 $1->id->c_str());

    $$ = new_exp(up, @$, UExpression::FUNCTION, $1, $3);
  }

| "%" variable         { $$ = new_exp(up, @$, UExpression::ADDR_VARIABLE, $2); }
| variable             { $$ = new_exp(up, @$, UExpression::VARIABLE, $1);      }
| "group" "identifier" { $$ = new_exp(up, @$, UExpression::GROUP, $2);         }
;


  /* num expr */
expr:
    expr "+" expr	{ $$ = new_exp(up, @$, UExpression::PLUS,  $1, $3); }
  | expr "-" expr	{ $$ = new_exp(up, @$, UExpression::MINUS, $1, $3); }
  | expr "*" expr	{ $$ = new_exp(up, @$, UExpression::MULT,  $1, $3); }
  | expr "/" expr	{ $$ = new_exp(up, @$, UExpression::DIV,   $1, $3); }
  | expr "%" expr	{ $$ = new_exp(up, @$, UExpression::MOD,   $1, $3); }
  | expr "^" expr	{ $$ = new_exp(up, @$, UExpression::EXP,   $1, $3); }

  | "copy" expr  %prec NEG {

      $$ = new UExpression(@$, UExpression::COPY, $2, 0);
      memcheck(up, $$, $2);
    }

  | "-" expr %prec NEG {

      $$ = new UExpression(@$, UExpression::NEG, $2, 0);
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
  "true"  { $$ = new UExpression(@$, UExpression::VALUE, ufloat(1)); }
| "false" { $$ = new UExpression(@$, UExpression::VALUE, ufloat(0)); }

| expr "=="  expr { $$ = new_exp(up, @$, UExpression::TEST_EQ,  $1, $3); }
| expr "~="  expr { $$ = new_exp(up, @$, UExpression::TEST_REQ, $1, $3); }
| expr "=~=" expr { $$ = new_exp(up, @$, UExpression::TEST_DEQ, $1, $3); }
| expr "%="  expr { $$ = new_exp(up, @$, UExpression::TEST_PEQ, $1, $3); }
| expr "!="  expr { $$ = new_exp(up, @$, UExpression::TEST_NE,  $1, $3); }
| expr ">"   expr { $$ = new_exp(up, @$, UExpression::TEST_GT,  $1, $3); }
| expr ">="  expr { $$ = new_exp(up, @$, UExpression::TEST_GE,  $1, $3); }
| expr "<"   expr { $$ = new_exp(up, @$, UExpression::TEST_LT,  $1, $3); }
| expr "<="  expr { $$ = new_exp(up, @$, UExpression::TEST_LE,  $1, $3); }

| "!" expr {
    $$ = new UExpression(@$, UExpression::TEST_BANG, $2, 0);
    memcheck(up, $$, $2);
  }

| expr "&&" expr { $$ = new_exp(up, @$, UExpression::TEST_AND, $1, $3); }
| expr "||" expr { $$ = new_exp(up, @$, UExpression::TEST_OR,  $1, $3); }
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
      UExpression *expr = new UExpression(@$, UExpression::VALUE, take($1));
      $$ = new UNamedParameters(expr);
      memcheck(up, $$, expr);
    }

  | "identifier" {

      UExpression *expr = new UExpression(@$, UExpression::VALUE, $1);
      $$ = new UNamedParameters(expr);
      memcheck(up, $$, expr);
    }

  |  NUM rawparameters {

      UExpression *expr = new UExpression(@$, UExpression::VALUE, take($1));
      $$ = new UNamedParameters(expr, $2);
      memcheck(up, $$, $2, expr);
    }

  |  "identifier" rawparameters {

      UExpression *expr = new UExpression(@$, UExpression::VALUE, $1);
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
      $$ = new UExpression(@$, UExpression::VALUE, $2);
      memcheck(up, $$, $2);
    }

  | "function" variable "(" identifiers ")" {
      $$ = new_exp(up, @$, UExpression::FUNCTION, $2, $4);
    }

  | "function" variable {
      $$ = new UExpression(@$, UExpression::FUNCTION, $2,
			   static_cast<UNamedParameters*> (0));
      memcheck(up, $$, $2);
    }

  | "event" variable "(" identifiers ")" {
      $$ = new_exp(up, @$, UExpression::EVENT, $2, $4);
    }

  | "event" variable {
      $$ = new UExpression(@$, UExpression::EVENT, $2,
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

/* VARIABLES */

variables:
  /* empty */  { $$ = 0; }

  | variable {
      memcheck(up, $1);
      $$ = new UVariableList($1);
      memcheck(up, $$, $1);
    }

  | variable ";" variables {
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
