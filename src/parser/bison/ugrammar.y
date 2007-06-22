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
%require "2.3"
%error-verbose
%defines
%skeleton "lalr1.cc"
%parse-param {UParser& up}
%lex-param   {UParser& up}
%debug

%code requires
{
// Output in ugrammar.hh.
#include "kernel/fwd.hh"
#include "kernel/utypes.hh"
#include "ast/fwd.hh"
}

// Locations.
%locations
// We use pointers to store the filename in the locations.  This saves
// space (pointers), time (no deep copy), but leaves the problem of
// deallocation.  The class Symbol provides this.
%define "filename_type" "libport::Symbol"
%initial-action
{
  // Saved when exiting the start symbol.
  @$ = up.loc_;
}


/* Possible data type returned by the bison parsing mechanism */
%union
{
  ast::Exp *expr;
}

// Old junk we should get rid of.
%union
{
  UBinary                 *binary;
  UVariableName           *variable;
  UVariableList           *variablelist;
  UProperty               *property;
}

%code // Output in ugrammar.cc.
{
#include <string>
#include <iostream>
#include <sstream>

#include "libport/ref-pt.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "ast/all.hh"

#include "parser/uparser.hh"
#include "ubinary.hh"
#include "ucommand.hh"
#include "uasynccommand.hh"
#include "ugroup.hh"
#include "uobj.hh"
#include "uproperty.hh"
#include "uvariablename.hh"
#include "uvariablelist.hh"

  extern UString** globalDelete;

  namespace
  {

#if 0 // FIXME: Not used yet.

    /// Whether the \a command was the empty command.
    static
    bool
    spontaneous (const UCommand& u)
    {
      const UCommand_NOOP* noop = dynamic_cast<const UCommand_NOOP*>(&u);
      return noop && noop->is_spontaneous();
    }

    /// Issue a warning.
    static
    void
    warn (UParser& up, const yy::parser::location_type& l, const std::string& m)
    {
      std::ostringstream o;
      o << "!!! " << l << ": " << m << "\n" << std::ends;
      up.connection.send(o.str().c_str(), "warning");
    }

    /// Complain if \a command is not spontaneous.
    static
    void
    warn_spontaneous(UParser& up,
  		   const yy::parser::location_type& l, const UCommand& u)
    {
      if (spontaneous(u))
	warn (up, l,
  	    "implicit empty statement.  "
  	    "Use 'noop' to make it explicit.");
    }
#endif
    /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
    static
    ast::Exp*
    new_bin(const yy::parser::location_type& l, Flavorable::UNodeType op,
	    ast::Exp* lhs, ast::Exp* rhs)
    {
      switch (op)
      {
	case Flavorable::UAND:
	  return new ast::AndExp (l, lhs, rhs);
	case Flavorable::UCOMMA:
	  return new ast::CommaExp (l, lhs, rhs);
	case Flavorable::UPIPE:
	  return new ast::PipeExp (l, lhs, rhs);
	case Flavorable::USEMICOLON:
	  return new ast::SemicolonExp (l, lhs, rhs);
	default:
	  pabort(op);
      }
    }

    /// A new UExpression of type \c t and child \c t1.
    template <class T1>
    static
    UExpression*
    new_exp (UParser& up, const yy::parser::location_type& l,
	     UExpression::Type t, T1* t1)
    {
      UExpression* res = new UExpression(l, t, t1);
      return res;
    }

    /// A new expression of operator \c o and children \c t1, \c t2.
    static
    ast::OpExp*
    new_exp (UParser&, const yy::parser::location_type& l,
  	   ast::OpExp::type o, ast::Exp* t1, ast::Exp* t2)
    {
      ast::OpExp* res = new ast::OpExp(l, t1, t2, o);
      return res;
    }

    /// Return the value pointed to be \a s, and delete it.
    template <typename T>
    static
    T
    take (T* s)
    {
      T res = *s;
      delete s;
      return res;
    }
  } // anon namespace

  /// Direct the call from 'bison' to the scanner in the right UParser.
  inline
  yy::parser::token_type
  yylex(yy::parser::semantic_type* val, yy::location* loc, UParser& up)
  {
    return up.scanner_.yylex(val, loc, up);
  }

} // %code requires.

/* Tokens and nonterminal symbols, with their type */

%token
  TOK_ADDGROUP     "addgroup"
  TOK_ALIAS        "alias"
  TOK_LAND         "&&"
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
  TOK_FALSE        "false"
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
  TOK_LBRACE       "{"
  TOK_LOOP         "loop"
  TOK_LOOPN        "loopn"
  TOK_LPAREN       "("
  TOK_LBRACKET     "["
  TOK_MINUS        "-"
  TOK_MINUSASSIGN  "-="
  TOK_MINUSMINUS   "--"
  TOK_STAR         "*"
  TOK_NEW          "new"
  TOK_NOOP         "noop"
  TOK_NORM         "'n"
  TOK_OBJECT       "object"
  TOK_ONLEAVE      "onleave"
  TOK_LOR          "||"
  TOK_PERCENT      "%"
  TOK_PLUS         "+"
  TOK_PLUSASSIGN   "+="
  TOK_PLUSPLUS     "++"
  TOK_POINT        "."
  TOK_RBRACE       "}"
  TOK_RETURN       "return"
  TOK_RPAREN       ")"
  TOK_RBRACKET     "]"
  TOK_STATIC       "static"
  TOK_STOP         "stop"
  TOK_STOPIF       "stopif"
  TOK_SUBCLASS     "subclass"
  TOK_TILDE        "~"
  TOK_TIMEOUT      "timeout"
  TOK_TRUE         "true"
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



/*----------.
| Numbers.  |
`----------*/

%union { int ival; }
%token
  <ival> INTEGER    "integer"
  <ival> FLAG       "flag"
  <ival> FLAG_TEST  "flag test"
  <ival> FLAG_ID    "flag identifier"
  <ival> FLAG_TIME  "flag time"
%printer { debug_stream() << $$; } <ival>;

%union { float fval; }
%token
  <fval> FLOAT      "float"
  <fval> TIME_VALUE "time"
%type <fval> number;
%printer { debug_stream() << $$; } <fval>;


 /*----------.
 | Strings.  |
 `----------*/
%union
{
  std::string		   *str;
}

%token
   <str>  STRING             "string"
   <str>  BINDER             "binder"
   <str>  OPERATOR           "operator command"
   <str>  OPERATOR_ID        "operator"
   <str>  OPERATOR_ID_PARAM  "param-operator"
   <str>  OPERATOR_VAR       "var-operator"
%destructor { delete $$; } <str>;
%printer { debug_stream() << *$$; } <str>;


 /*----------.
 | Symbols.  |
 `----------*/
%union
{
  libport::Symbol* symbol;
}

%token <symbol> IDENTIFIER    "identifier"
%destructor { delete $$; } <symbol>;
%printer { debug_stream() << *$$; } <symbol>;


%type <expr>                expr            "expression"
%type <expr>                expr.opt        "optional expression"
%type <fval>                time_expr       "time expression"
%type <expr>                taggedcommands  "set of commands"
%type <expr>                taggedcommand   "tagged command"
%type <expr>                command         "command"
%type <expr>                statement       "statement"
%type <named_arguments>     exprs           "zero or more expressions"
%type <named_arguments>     exprs.1         "one or more expressions"
%type <named_arguments>     raw_arguments   "list of attributes"
%type <named_arguments>     namedarguments  "list of named arguments"
%type <named_arguments>     flag            "a flag"
%type <named_arguments>     flags.0         "zero or more flags"
%type <named_arguments>     flags.1         "one or more flags"
%type <variablelist>        variables       "list of variables"
%type <expr>                softtest        "soft test"
%type <named_arguments>     identifiers     "list of identifiers"
%type <expr>                class_declaration "class declaration"
%type <named_arguments>     class_declaration_list "class declaration list"
%type <binary>              binary          "binary"
%type <property>            property        "property"
%type <variable>            variable        "variable"
%type <variable>            purevariable    "pure variable"
//%type  <named_arguments>     purevariables   "list of pure variables"


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
%right "'n"

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
| variable "=" binary ";"  {}
| taggedcommands           {}
;

binary:
  "bin" "integer" raw_arguments {}
;

raw_argument:
  number                    {}
| "identifier"              {}
;

// raw_argument*
raw_arguments:
  /* empty */
| raw_arguments raw_argument {}
;


/*-----------------.
| taggedcommands.  |
`-----------------*/

taggedcommands:
  taggedcommand
| taggedcommands "," taggedcommands { $$ = new_bin(@$, $2, $1, $3); }
| taggedcommands ";" taggedcommands { $$ = new_bin(@$, $2, $1, $3); }
| taggedcommands "|" taggedcommands { $$ = new_bin(@$, $2, $1, $3); }
| taggedcommands "&" taggedcommands { $$ = new_bin(@$, $2, $1, $3); }
;

/*----------------.
| taggedcommand.  |
`----------------*/

%type <expr> tag;
tag: expr;

taggedcommand:

  command {}

| tag flags.0 ":" command
  {
    $$ = new ast::TagExp (@$, $1, $4);
    // FIXME: $2 ignored.
  }

| flags.1 ":" command {}
;


/*--------.
| flags.  |
`--------*/

flag:
  FLAG
  {}

| FLAG_TIME "(" expr ")"
  {}

| FLAG_ID "(" expr ")"
  {}

| FLAG_TEST "(" softtest ")"
  {}
;

// One or more "flag"s.
flags.1:
  flag             {}
| flags.1 flag     {}
;

// Zero or more "flag"s.
flags.0:
  /* empty. */   {}
| flags.1        {}
;



/*----------.
| command.  |
`----------*/

command:
  statement
| "{" taggedcommands "}" {}
;


/*----------.
| flavors.  |
`----------*/
%code requires
{
#include "flavorable.hh"
};
%union { Flavorable::UNodeType flavor; };
%token
  <flavor> TOK_COMMA        ","
  <flavor> TOK_SEMICOLON    ";"
  <flavor> TOK_AND          "&"
  <flavor> TOK_PIPE         "|"
;

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


/*------------.
| Statement.  |
`------------*/

statement:
  /* empty */ {}
| "noop"      {}
| expr {}
| variable number {}
| "echo" expr namedarguments {}
| "group" "identifier" "{" identifiers "}" {}
| "addgroup" "identifier" "{" identifiers "}" {}
| "delgroup" "identifier" "{" identifiers "}" {}
| "group" {}
| "alias" purevariable purevariable {}
| purevariable "inherits" purevariable {}
| purevariable "disinherits" purevariable {}
| "alias" purevariable {}
| "unalias" purevariable {}
| "alias" {}
| OPERATOR {}
| OPERATOR_ID tag {}
| OPERATOR_VAR variable {}
| BINDER "object" purevariable {}
| BINDER "var" purevariable "from" purevariable {}
| BINDER "function" "(" "integer" ")" purevariable "from" purevariable {}
| BINDER "event" "(" "integer" ")" purevariable "from" purevariable {}
| "wait" expr {}
| "emit" purevariable {}
| "emit" purevariable "(" exprs ")" {}
| "emit" "(" expr ")" purevariable {}
| "emit" "(" expr ")" purevariable "(" exprs ")" {}
| "emit" "(" ")" purevariable {}
| "emit" "(" ")" purevariable "(" exprs ")" {}
| "waituntil" softtest {}
| "def" {}
| "var" variable {}
| "def" variable {}
| "var" "{" variables "}" {}
| "class" "identifier" "{" class_declaration_list "}" {}
| "class" "identifier" {}
| "event" name formal_arguments {}
| "function" name formal_arguments {} taggedcommand {}
;

/*------------------------.
| Statement: Assignment.  |
`------------------------*/
statement:
        variable "=" expr namedarguments {}
| "var" variable "=" expr namedarguments {}
| variable "+=" expr {}
| variable "-=" expr {}
| property "=" expr {}
| variable "=" "new" "identifier" {}
| variable "=" "new" "identifier" "(" exprs ")" {}
| variable "--" {}
| variable "++" {}
;

/*--------------------------.
| Statement: Control flow.  |
`--------------------------*/
statement:
  "at" and.opt "(" softtest ")" taggedcommand %prec CMDBLOCK {}
| "at" and.opt "(" softtest ")" taggedcommand "onleave" taggedcommand {}
| "every" "(" expr ")" taggedcommand {}
| "if" "(" expr ")" taggedcommand %prec CMDBLOCK    {}
| "if" "(" expr ")" taggedcommand "else" taggedcommand  {}
| "for" flavor.opt "(" statement ";"
			 expr ";"
			 statement ")" taggedcommand %prec CMDBLOCK {}
| "foreach" flavor.opt purevariable "in" expr "{" taggedcommands "}"
     %prec CMDBLOCK {}
| "freezeif" "(" softtest ")" taggedcommand {}
| "loop" taggedcommand %prec CMDBLOCK {}
| "loopn" flavor.opt "(" expr ")" taggedcommand %prec CMDBLOCK {}
| "stopif" "(" softtest ")" taggedcommand {}
| "timeout" "(" expr ")" taggedcommand {}
| "return" expr.opt   { $$ = new ast::ReturnExp(@$, $2, false); }
| "whenever" "(" softtest ")" taggedcommand %prec CMDBLOCK {}
| "whenever" "(" softtest ")" taggedcommand "else" taggedcommand {}
| "while" pipe.opt "(" expr ")" taggedcommand %prec CMDBLOCK {}
;



/*-------------------------------.
| Name, Purevariable, Variable.  |
`-------------------------------*/

name:
  "identifier"
| name "." "identifier"
| name "[" expr "]"
;

/*
names.1:
  name
| names "," name
;

names:
  // nothing.
| names.1
;
*/

purevariable:

    "$" "(" expr ")" {}

| purevariable "[" expr "]" {}

| purevariable "." "identifier" {}

| "identifier" "::" "identifier" {}

;

variable:
  purevariable		{}
| "static" purevariable	{}
| purevariable "'n"	{}
| purevariable "'e"	{}
| purevariable "'in"	{}
| purevariable "'out"	{}
| purevariable "'"	{}
| purevariable "''"	{}
| purevariable "'d"	{}
| purevariable "'dd"	{}
;


/*---------.
| PROPERTY |
`---------*/

property:

    purevariable "->" "identifier" {}
;


/*---------------.
| NAMEDARGUMENTS |
`---------------*/

namedarguments:
  /* empty */ {}

| "identifier" ":" expr namedarguments {}
;


/*------------.
| time_expr.  |
`------------*/

time_expr:
  TIME_VALUE
| time_expr TIME_VALUE { $$ += $2; }
;


number:
  "integer"  { $$ = $1; }
| "float"
;


/*-------.
| expr.  |
`-------*/

expr:
  number    { $$ = new ast::FloatExp(@$, $1);        }
| time_expr { $$ = new ast::FloatExp(@$, $1);        }
| "string"  { $$ = new ast::StringExp(@$, take($1)); }

| "[" exprs "]" {}

| property {}

| variable "(" exprs ")"  {}

| "%" variable         {}
| variable             {}
| "group" "identifier" {}
;


  /*---------.
  | num expr |
  `---------*/
expr:
  expr "+" expr	{ $$ = new_exp(up, @$, ast::OpExp::add, $1, $3); }
| expr "-" expr	{ $$ = new_exp(up, @$, ast::OpExp::sub, $1, $3); }
| expr "*" expr	{ $$ = new_exp(up, @$, ast::OpExp::mul, $1, $3); }
| expr "/" expr	{ $$ = new_exp(up, @$, ast::OpExp::div, $1, $3); }
| expr "%" expr	{ $$ = new_exp(up, @$, ast::OpExp::mod, $1, $3); }
| expr "^" expr	{ $$ = new_exp(up, @$, ast::OpExp::exp, $1, $3); }
| "-" expr %prec NEG { $$ = new ast::NegOpExp(@$, $2); }
| "(" expr ")"  { $$ = $2; }

| "copy" expr  %prec NEG {}
;

expr.opt:
  /* nothing */ { $$ = 0; }
| expr          { $$ = $1; }
;


/*--------.
| Tests.  |
`--------*/
%token
  TOK_EQU   "=="
  TOK_GTH   ">"
  TOK_LEQ   "<="
  TOK_LTH   "<"
  TOK_PEQ  "%="
  TOK_NEQ   "!="
  TOK_GEQ   ">="
  TOK_DEQ  "=~="
  TOK_REQ  "~="
;

expr:
  "true"  {}
| "false" {}

| expr "=="  expr { $$ = new_exp(up, @$, ast::OpExp::equ, $1, $3); }
| expr "~="  expr { $$ = new_exp(up, @$, ast::OpExp::req, $1, $3); }
| expr "!="  expr { $$ = new_exp(up, @$, ast::OpExp::neq, $1, $3); }
| expr ">"   expr { $$ = new_exp(up, @$, ast::OpExp::gth, $1, $3); }
| expr ">="  expr { $$ = new_exp(up, @$, ast::OpExp::geq, $1, $3); }
| expr "<"   expr { $$ = new_exp(up, @$, ast::OpExp::lth, $1, $3); }
| expr "<="  expr { $$ = new_exp(up, @$, ast::OpExp::leq, $1, $3); }

| expr "=~=" expr { /* $$ = new_exp(up, @$, ???, $1, $3); */ }
| expr "%="  expr { /* $$ = new_exp(up, @$, ???, $1, $3); */ }

| "!" expr {}

| expr "&&" expr { $$ = new_exp(up, @$, ast::OpExp::land, $1, $3); }
| expr "||" expr { $$ = new_exp(up, @$, ast::OpExp::lor,  $1, $3); }
;


/*--------------.
| Expressions.  |
`--------------*/

exprs:
  /* empty */ {}
| exprs.1     {}
;

exprs.1:
  expr             {}
| exprs.1 "," expr {}
;


/*---------.
| softtest |
`---------*/

softtest:
  expr
| expr "~" expr  {}
| "(" expr "~" expr ")" {}
;


/*--------------.
| identifiers.  |
`--------------*/

// "var"?
var.opt:
  /* empty. */
| "var"
;

// One or several comma-separated identifiers.
identifiers.1:
  var.opt "identifier"
| identifiers.1 "," var.opt "identifier"
;

// Zero or several comma-separated identifiers.
identifiers:
  /* empty */
| identifiers.1
;


/*-------------------------------------------.
| class_declaration & class_declaration_list |
`-------------------------------------------*/

class_declaration:
  "var"      name
| "function" name formal_arguments
| "event"    name formal_arguments
;

/* It used to be possible to not have the parens for empty identifiers.
   For the time being, this is disabled because it goes against
   factoring.  Might be reintroduced later. */
formal_arguments:
  "(" identifiers ")"
;

class_declaration_list:
  /* empty */  {}
| class_declaration {}
| class_declaration ";" class_declaration_list {}
;

/*----------.
| variables |
`----------*/

variables:
  /* empty */  {}
| variable {}
| variable ";" variables {}
;

/* End of grammar */

%%

// The error function that 'bison' calls.
void
yy::parser::error(const location_type& l, const std::string& m)
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
