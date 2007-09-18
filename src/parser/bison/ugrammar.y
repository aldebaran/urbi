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


%code // Output in ugrammar.cc.
{
#include <string>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include "ast/all.hh"
#include "runner/runner.hh"

#include "parser/uparser.hh"

#define EVALUATE(Tree)						\
 do {								\
    std::cerr << "Command: " << Tree << std::endl;		\
    runner::Runner r;						\
    r(Tree);							\
    std::cerr << "Result: "					\
	      << libport::deref << r.result() << std::endl;	\
 } while (0)

  namespace
  {

    /// Return the value pointed to be \a s, and delete it.
    template <typename T>
    static
    T
    take (T* s)
    {
      assert (s);
      T res = *s;
      delete s;
      return res;
    }

    /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
    static
    ast::Exp*
    new_bin(const yy::parser::location_type& l, Flavorable::UNodeType op,
	    ast::Exp* lhs, ast::Exp* rhs)
    {
      ast::Exp* res = 0;
      assert (lhs);
      assert (rhs);
      switch (op)
      {
	case Flavorable::UAND:
	  res = new ast::And (l, lhs, rhs);
	  break;
	case Flavorable::UPIPE:
	  res = new ast::Pipe (l, lhs, rhs);
	  break;
	default:
	  pabort(op);
      }
      return res;
    }

    /// "<target> . <method> (args)".
    static
    ast::Call*
    call (const yy::parser::location_type& l,
	  ast::Exp* target, libport::Symbol method, ast::exps_type* args)
    {
      args->push_front (target);
      ast::Call* res = new ast::Call(l, method, args);
      return res;
    }

    /// "<target> . <method> ()".
    static
    ast::Call*
    call (const yy::parser::location_type& l,
	  ast::Exp* target, libport::Symbol method)
    {
      return call (l, target, method, new ast::exps_type);
    }

    /// "<target> . <method> ()".
    static
    ast::Call*
    call (const yy::parser::location_type& l,
	  ast::Exp* target, libport::Symbol* method)
    {
      assert (method);
      return call (l, target, take(method));
    }

    /// "<target> . <method> (<arg1>)".
    static
    ast::Call*
    call (const yy::parser::location_type& l,
	  ast::Exp* target, libport::Symbol* method, ast::Exp* arg1)
    {
      assert (method);
      ast::Call* res = call (l, target, method);
      res->args_get().push_back(arg1);
      return res;
    }

    /// Return \a e in a ast::Scope unless it is already one.
    static
    ast::Scope*
    scope (const yy::parser::location_type& l, ast::Exp* e)
    {
      if (ast::Scope* res = dynamic_cast<ast::Scope*>(e))
	return res;
      else
	return new ast::Scope(l, e);
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
	TOK_ASSIGN       "="
	TOK_AT           "at"
	TOK_BIN          "bin"
	TOK_CLASS        "class"
	TOK_COLON        ":"
	TOK_DEF          "def"
	TOK_DELGROUP     "delgroup"
	TOK_DIR          "->"
	TOK_DISINHERITS  "disinherits"
	TOK_DOLLAR       "$"
	TOK_DOUBLECOLON  "::"
	TOK_ELSE         "else"
	TOK_EMIT         "emit"
	TOK_EVENT        "event"
	TOK_EVERY        "every"
	TOK_FALSE        "false"
	TOK_FOR          "for"
	TOK_FOREACH      "foreach"
	TOK_FREEZEIF     "freezeif"
	TOK_FROM         "from"
	TOK_FUNCTION     "function"
	TOK_GROUP        "group"
	TOK_IF           "if"
	TOK_IN           "in"
	TOK_INHERITS     "inherits"
	TOK_LBRACE       "{"
	TOK_LBRACKET     "["
	TOK_LOOP         "loop"
	TOK_LOOPN        "loopn"
	TOK_LPAREN       "("
	TOK_MINUSASSIGN  "-="
	TOK_MINUSMINUS   "--"
	TOK_NEW          "new"
	TOK_NOOP         "noop"
	TOK_NORM         "'n"
	TOK_OBJECT       "object"
	TOK_ONLEAVE      "onleave"
	TOK_PLUSASSIGN   "+="
	TOK_PLUSPLUS     "++"
	TOK_POINT        "."
	TOK_RBRACE       "}"
	TOK_RBRACKET     "]"
	TOK_RETURN       "return"
	TOK_RPAREN       ")"
	TOK_STATIC       "static"
	TOK_STOPIF       "stopif"
	TOK_TILDE        "~"
	TOK_TIMEOUT      "timeout"
	TOK_TRUE         "true"
	TOK_UNALIAS      "unalias"
	TOK_VAR          "var"
	TOK_VARERROR     "'e"
	TOK_VARIN        "'in"
	TOK_VAROUT       "'out"
	TOK_WAITUNTIL    "waituntil"
	TOK_WHENEVER     "whenever"
	TOK_WHILE        "while"

 // Tokens that have a symbol semantic value.
%token <symbol>
	TOK_COPY         "copy"
	TOK_ECHO         "echo"
	TOK_WAIT         "wait"

%token TOK_EOF 0 "end of command"



/*----------.
| Numbers.  |
`----------*/

%union { int ival; }
%token <ival>
	INTEGER    "integer"
	FLAG       "flag"
	FLAG_TEST  "flag test"
	FLAG_ID    "flag identifier"
	FLAG_TIME  "flag time"
%printer { debug_stream() << $$; } <ival>;

%union { float fval; }
%token <fval>
	FLOAT      "float"
	TIME_VALUE "time"
%type <fval> number;
%type <fval> time_expr;
%printer { debug_stream() << $$; } <fval>;


 /*----------.
 | Strings.  |
 `----------*/
%union
{
  std::string* str;
}

// FIXME: Arguably, could be a Symbol too.
%token
   <str>  STRING             "string"
%destructor { delete $$; } <str>;
%printer { debug_stream() << libport::deref << $$; } <str>;


 /*----------.
 | Symbols.  |
 `----------*/
%union
{
  libport::Symbol* symbol;
  ast::symbols_type* symbols;
}

%token <symbol>
	IDENTIFIER         "identifier"
	BINDER             "binder"
	OPERATOR           "operator command"
	OPERATOR_ID        "operator"
	OPERATOR_VAR       "var-operator"
%type <symbols> identifiers identifiers.1 formal_args
// FIXME: this destructor entails double frees and invalid pointer
// frees.
// %destructor { delete $$; } <symbol>;
%printer { debug_stream() << libport::deref << $$; } <symbol>;
%printer { debug_stream() << libport::separate (*$$, ", "); } <symbols>;


/*--------------.
| Expressions.  |
`--------------*/

%union
{
  ast::Exp*    expr;
  ast::Call*   call;
  ast::Nary*   nary;
}

%printer { debug_stream() << libport::deref << $$; } <expr> <call> <nary>;

%type <call>  lvalue

%type <expr>  class_declaration
%type <expr>  class_declaration_list
%type <expr>  expr
%type <expr>  expr.opt
%type <expr>  flag
%type <expr>  flags.0
%type <expr>  flags.1
%type <expr>  namedarguments
%type <expr>  raw_arguments
%type <expr>  softtest
%type <expr>  stmt

/*----------------------.
| Operator precedence.  |
`----------------------*/

 /*
   ! < ( so that !m(x) be read as !(m(x)).
 */

%left  "||"
%left  "&&"
%left  "==" "~=" "%=" "=~=" "!=" ">" ">=" "<" "<="
%left  "-" "+"
%left  "*" "/" "%"
%left  "!" UNARY     /* Negation--unary minus */
%left  "("
%right "^"
%right "'n"

// a^b.c reads "a ^ (b.c)".
%left  "."

%right "," ";"
%left  "&" "|"
%left  CMDBLOCK
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
    up.command_tree_set (0);
  }
| lvalue "=" binary ";"  { /* FIXME: */ up.command_tree_set (0); }
| stmts ";"
  {
    $1->back().second = ast::execution_foreground;
    up.command_tree_set ($1);
  }
| stmts ","
  {
    $1->back().second = ast::execution_background;
    up.command_tree_set ($1);
  }
;

binary:
  "bin" "integer" raw_arguments { /* FIXME: Fill. */ }
;

raw_argument:
  number                    { /* FIXME: Fill. */ }
| "identifier"              { /* FIXME: Fill. */ }
;

// raw_argument*
raw_arguments:
  /* empty */                { $$ = 0; }
| raw_arguments raw_argument { $$ = 0; }
;


/*--------.
| stmts.  |
`--------*/

%type <nary>  stmts;

// Statements: with ";" and ",".
stmts:
  cstmt
  {
    // FIXME: Adjust the locations.
    $$ = new ast::Nary();
    $$->push_back ($1);
  }
| stmts ";" cstmt
  {
    // The ";" qualifies the previous command.
    $$->push_back(ast::execution_foreground, $3);
  }
| stmts "," cstmt
  {
    // The "," qualifies the previous command.
    $$->push_back(ast::execution_background, $3);
  }
;

%type <expr> cstmt;
// Composite statement: with "|" and "&".
cstmt:
  stmt
  {
    // XXX FIXME: Used as a temporary workaround until all actions are
    // filled in this parser
    if (!$1)
      $$ = new ast::Noop (@$);
    else
      $$ = $1;
  }
| cstmt "|" cstmt { $$ = new_bin(@$, $2, $1, $3); }
| cstmt "&" cstmt { $$ = new_bin(@$, $2, $1, $3); }
;


/*--------------------------.
| tagged and flagged stmt.  |
`--------------------------*/

%type <expr> tag;
tag: expr;

stmt:
  tag flags.0 ":" stmt
  {
    $$ = new ast::Tag (@$, $1, $4);
  }
| flags.1 ":" stmt { $$ = $3; }
;


/*--------.
| flags.  |
`--------*/

flag:
  FLAG                        { $$ = 0; }
| FLAG_TIME "(" expr ")"      { $$ = 0; }
| FLAG_ID "(" expr ")"        { $$ = 0; }
| FLAG_TEST "(" softtest ")"  { $$ = 0; }
;

// One or more "flag"s.
flags.1:
  flag             { $$ = 0; }
| flags.1 flag     { $$ = 0; }
;

// Zero or more "flag"s.
flags.0:
  /* empty. */   { $$ = 0; }
| flags.1        { $$ = 0; }
;



/*-------.
| stmt.  |
`-------*/

stmt:
  "{" stmts "}" { $$ = scope(@$, $2); }
;


/*----------.
| flavors.  |
`----------*/
%code requires
{
#include "flavorable.hh"
};
%union { Flavorable::UNodeType flavor; };
%token <flavor>
	TOK_COMMA        ","
	TOK_SEMICOLON    ";"
	TOK_AND          "&"
	TOK_PIPE         "|"
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


/*-------.
| Stmt.  |
`-------*/

stmt:
  /* empty */ { $$ = new ast::Noop (@$); }
| "noop"      { $$ = new ast::Noop (@$); }
| expr        { $$ = $1; }
| "echo" expr namedarguments { $$ = call (@$, 0, $1, $2); }
| "group" "identifier" "{" identifiers "}" { $$ = 0; }
| "addgroup" "identifier" "{" identifiers "}" { $$ = 0; }
| "delgroup" "identifier" "{" identifiers "}" { $$ = 0; }
| "group" { $$ = 0; }
//| "alias" name name { $$ = 0; }
//| name "inherits" name { $$ = 0; }
//| name "disinherits" name { $$ = 0; }
//| "alias" name { $$ = 0; }
//| "unalias" name { $$ = 0; }
| "alias" { $$ = 0; }
| OPERATOR { $$ = 0; }
| OPERATOR_ID tag { $$ = 0; }
//| OPERATOR_VAR name { $$ = 0; }
//| BINDER "object" name { $$ = 0; }
//| BINDER "var" name "from" name { $$ = 0; }
//| BINDER "function" "(" "integer" ")" name "from" name { $$ = 0; }
//| BINDER "event" "(" "integer" ")" name "from" name { $$ = 0; }
//| "emit" name args                  { $$ = 0; }
//| "emit" "(" expr.opt ")" name args { $$ = 0; }
| "wait" expr			    { $$ = call (@$, 0, $1, $2); }
| "waituntil" softtest              { $$ = 0; }
| "def" { $$ = 0; }
//| "var" name { $$ = 0; }
// Duplicates the previous one, and cannot be factored.
// | "def" name { $$ = 0; }
// The following one is incorrect: wrong separator, should be ;.
// | "var" "{" identifiers "}" { $$ = 0; }
| "class" "identifier" "{" class_declaration_list "}" { $$ = 0; }
| "class" "identifier" { $$ = 0; }
// | "event"    name formal_args { $$ = 0; }
| "function" k1_id formal_args "{" stmts "}"
  {
    // Compiled as "name = function args stmt".
    $$ = new ast::Assign (@$, $2,
			  new ast::Function (@$, take($3),
					     scope(@4+@6, $5)));
  }
;

/*-----------------------------.
| k1_id: A simple identifier.  |
`-----------------------------*/

// We would really like to support simultaneously the following
// constructs:
//
//  a.b = function (c) { c }
// and
//  function a.b (c) { c }
//
// unfortunately it unleashes a host of issues.  It requires introducing
// two nonterminals to denote on the one hand side "a.b.c" etc. and otoh
// "a().b(c,d).e".  Of course they overlap, so we have conflicts.
//
// We can also try to use a single nonterminal, i.e., accept to parse:
//
//   function a(b).c(1) { 1 }
//
// but then reject it by hand once "formal arguments" have been "reparsed".
// It sucks.  Yet we have another problem: the two "function"-constructs
// conflict between themselves.  The LR(1) parser cannot tell whether
// "function (" is starting an lambda expression ("a = function (){ 1 }")
// or a named function ("function (a).f() { 1 }").  So I chose to limit
// named function to what we need to be compatible with k1: basically
// "function a ()" and "function a.b()".
//
// Another option would have been to use two keywords, say using "lambda"
// for anonymous functions.  But that's not a good option IMHO (AD).
%type <call> k1_id;
k1_id:
  "identifier"                   { $$ = call (@$, 0, $1); }
| "identifier" "." "identifier"  { $$ = call (@$, call (@1, 0, $1), $3); }
;

/*-------------------.
| Stmt: Assignment.  |
`-------------------*/
stmt:
	lvalue "=" expr namedarguments { $$ = new ast::Assign (@$, $1, $3); }
| "var" lvalue "=" expr namedarguments { $$ = 0; }
| lvalue "+=" expr { $$ = 0; }
| lvalue "-=" expr { $$ = 0; }
| lvalue "--"      { $$ = 0; }
| lvalue "++"      { $$ = 0; }
;

/*---------------------.
| Stmt: Control flow.  |
`---------------------*/
stmt:
  "at" and.opt "(" softtest ")" stmt %prec CMDBLOCK
    { 
      $$ = 0; 
    }
| "at" and.opt "(" softtest ")" stmt "onleave" stmt
    { 
      $$ = 0; 
    }
| "every" "(" expr ")" stmt
    { 
      $$ = 0; 
    }
| "if" "(" expr ")" stmt %prec CMDBLOCK   
    { 
      $$ = 0; 
    }
| "if" "(" expr ")" stmt "else" stmt 
    { 
      $$ = 0; 
    }
| "for" flavor.opt "(" stmt ";" expr ";" stmt ")" stmt %prec CMDBLOCK
    { 
      $$ = 0; 
    }
| "foreach" flavor.opt "identifier" "in" expr "{" stmts "}"    %prec CMDBLOCK
    { 
      $$ = 0; 
    }
| "freezeif" "(" softtest ")" stmt
    { 
      $$ = 0; 
    }
| "loop" stmt %prec CMDBLOCK
    { 
      $$ = 0; 
    }
| "loopn" flavor.opt "(" expr ")" stmt %prec CMDBLOCK
    { 
      $$ = 0; 
    }
| "stopif" "(" softtest ")" stmt
    { 
      $$ = 0; 
    }
| "timeout" "(" expr ")" stmt
    { 
      $$ = 0; 
    }
| "return" expr.opt  
    { $$ = new ast::Return(@$, $2, false); }
| "whenever" "(" softtest ")" stmt %prec CMDBLOCK
    { 
      $$ = 0; 
    }
| "whenever" "(" softtest ")" stmt "else" stmt
    { 
      $$ = 0; 
    }
| "while" pipe.opt "(" expr ")" stmt %prec CMDBLOCK
    { 
      $$ = 0; 
    }
;

%type <call> call message;
message:
  "identifier" args
    {
      // The target of the message is currently unknown.
      $$ = call (@$, 0, take($1), $2);
    }
;

call:
  message           { $$ = $1; }
| expr "." message  
  { 
    // Now we know the target.
    $$ = $3; $$->args_get().front() = $1; 
  }
;

expr:
  call  { $$ = $1; }
;

/*-------.
| Name.  |
`-------*/

//%type <call>  name
//name:
//  "identifier"             { $$ = call(@$, 0, $1); }
//| "$" "(" expr ")"         { $$ = 0; }
//| name "." "identifier"    { $$ = call(@$, $1, $3); }
//| name "[" expr "]"        { $$ = 0; }
//| name "::" "identifier"   { $$ = 0; } // FIXME: Get rid of it, it's useless.
//;


/*------------.
| Variables.  |
`------------*/

// An lvalue is a Call without arguments.
lvalue:
  call
  {
    // There is an implicit target: the current object, 0.
    if ($$->args_get().size() != 1)
    {
      std::string lvalue ($1 ? boost::lexical_cast<std::string>(*$1)
			  : "<NULL>");
      error(@$, (std::string ("invalid lvalue: ") + lvalue));
      YYERROR;
    }
  }
;

//expr:
//  name          { $$ = $1; }
//| "static" name	{ /* FIXME: Fill. */ }
//| name derive   { /* FIXME: Fill. */ }
//| name "'e"	{ /* FIXME: Fill. */ }
//| name "'in"	{ /* FIXME: Fill. */ }
//| name "'out"   { /* FIXME: Fill. */ }
//| name "'n"	{ $$ = 0; }
//;
//
//%code requires
//{
//#include "uvariablename.hh" // UDeriveType
//};
//%union { UVariableName::UDeriveType derive; };
//%token
//  TOK_DERIV        "'"
//  TOK_DERIV2       "''"
//  TOK_TRUEDERIV    "'d"
//  TOK_TRUEDERIV2   "'dd";
//%type <derive> derive;
//derive:
//  "'"	{ $$ = UVariableName::UDERIV;	   }
//| "''"	{ $$ = UVariableName::UDERIV2;	   }
//| "'d"	{ $$ = UVariableName::UTRUEDERIV;  }
//| "'dd"	{ $$ = UVariableName::UTRUEDERIV2; }
//;


//expr:
//  name "->" "identifier" { $$ = 0; }
//;


/*------------.
| Functions.  |
`------------*/

// Anonymous function.
expr:
  // Because of conflicts, we need the braces
  "function" formal_args "{" stmts "}"
    {
      $$ = new ast::Function (@$, take($2), scope(@3+@5, $4));
    }
;

/*-----------------.
| namedarguments.  |
`-----------------*/

namedarguments:
  /* empty */ { $$ = 0; }
| "identifier" ":" expr namedarguments { $$ = 0; }
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
  number    { $$ = new ast::Float(@$, $1);        }
| time_expr { $$ = new ast::Float(@$, $1);        }
| "string"  { $$ = new ast::String(@$, take($1)); }
| "[" exprs "]" { $$ = new ast::List(@$, $2); }
//| name "(" exprs ")"
//    {
//      $1->args_get().splice($1->args_get().end(), *$3);
//      delete $3;
//      $$ = $1;
//    }
//| "%" name            { $$ = 0; }
| "group" "identifier"    { $$ = 0; }
| "new" "identifier" args
  {
    // Compiled as
    // id . clone () . init (args);
    // Parent class.
    ast::Exp* parent = call (@2, 0, $2);
    ast::exps_type* args = new ast::exps_type;
    args->push_back (call(@1 + @2, parent, "clone"));
    args->splice(args->end(), *$3);
    delete $3;
    $$ = new ast::Call (@$, "init", args);
  }
;



  /*---------.
  | num expr |
  `---------*/
// The name of the operators are the name of the messages.
%token <symbol>
	TOK_BANG    "!"
	TOK_PERCENT "%"
	TOK_STAR    "*"
	TOK_PLUS    "+"
	TOK_MINUS   "-"
	TOK_DIV     "/"
	TOK_EXP     "^"
;

expr:
  expr "+" expr	          { $$ = call(@$, $1, $2, $3); }
| expr "-" expr	          { $$ = call(@$, $1, $2, $3); }
| expr "*" expr	          { $$ = call(@$, $1, $2, $3); }
| expr "/" expr	          { $$ = call(@$, $1, $2, $3); }
| expr "%" expr	          { $$ = call(@$, $1, $2, $3); }
| expr "^" expr	          { $$ = call(@$, $1, $2, $3); }
| "-" expr    %prec UNARY { $$ = call(@$, $2, $1); }
| "(" expr ")"            { $$ = $2; }
| "copy" expr %prec UNARY { $$ = call(@$, $2, $1); }
;

/*--------.
| Tests.  |
`--------*/
%token <symbol>
	TOK_DEQ  "=~="
	TOK_EQU  "=="
	TOK_GEQ  ">="
	TOK_GTH  ">"
	TOK_LEQ  "<="
	TOK_LTH  "<"
	TOK_NEQ  "!="
	TOK_PEQ  "%="
	TOK_REQ  "~="

	TOK_LAND  "&&"
	TOK_LOR   "||"
;

expr:
  "false" { $$ = new ast::Float(@$, 0); }
| "true"  { $$ = new ast::Float(@$, 1); }

| expr "!="  expr { $$ = call(@$, $1, $2, $3); }
| expr "%="  expr { $$ = call(@$, $1, $2, $3); }
| expr "<"   expr { $$ = call(@$, $1, $2, $3); }
| expr "<="  expr { $$ = call(@$, $1, $2, $3); }
| expr "=="  expr { $$ = call(@$, $1, $2, $3); }
| expr "=~=" expr { $$ = call(@$, $1, $2, $3); }
| expr ">"   expr { $$ = call(@$, $1, $2, $3); }
| expr ">="  expr { $$ = call(@$, $1, $2, $3); }
| expr "~="  expr { $$ = call(@$, $1, $2, $3); }

| "!" expr { $$ = 0; }

| expr "&&" expr  { $$ = call(@$, $1, $2, $3); }
| expr "||" expr  { $$ = call(@$, $1, $2, $3); }
;

expr.opt:
  /* nothing */ { $$ = 0; }
| expr          { $$ = $1; }
;


/*--------------.
| Expressions.  |
`--------------*/

%union { ast::exps_type* exprs; };
%printer
{
  if ($$)
    debug_stream() << libport::separate (*$$, ", ");
  else
    debug_stream() << "NULL";
} <exprs>;
%type <exprs> exprs;
%type <exprs> exprs.1;
%type <exprs> args;

exprs:
  /* empty */ { $$ = new ast::exps_type; }
| exprs.1     { $$ = $1; }
;

exprs.1:
  expr             { $$ = new ast::exps_type; $$->push_back ($1); }
| exprs.1 "," expr { $$->push_back($3); }
;

// Effective arguments: 0 or more arguments in parens, or nothing.
args:
  /* empty */   { $$ = new ast::exps_type; }
| "(" exprs ")" { $$ = $2; }
;

/*-----------.
| softtest.  |
`-----------*/

softtest:
  expr
| expr "~" expr  { $$ = 0; }
| "(" expr "~" expr ")" { $$ = 0; }
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
  {
    $$ = new ast::symbols_type;
    $$->push_back($2);
  }
| identifiers.1 "," var.opt "identifier"
  {
    $$ = $1;
    $$->push_back($4);
  }
;

// Zero or several comma-separated identifiers.
identifiers:
  /* empty */     { $$ = new ast::symbols_type; }
| identifiers.1   { $$ = $1; }
;


/*---------------------------------------------.
| class_declaration & class_declaration_list.  |
`---------------------------------------------*/

class_declaration:
  "var"      "identifier"               { $$ = 0; }
| "function" "identifier" formal_args   { $$ = 0; }
| "event"    "identifier" formal_args   { $$ = 0; }
;

/* It used to be possible to not have the parens for empty identifiers.
   For the time being, this is disabled because it goes against
   factoring.  Might be reintroduced later. */
formal_args:
  "(" identifiers ")" { $$ = $2; }
;

class_declaration_list:
  /* empty */  { $$ = 0; }
| class_declaration { $$ = 0; }
| class_declaration ";" class_declaration_list { $$ = 0; }
;

/* End of grammar */

%%

// The error function that 'bison' calls.
void
yy::parser::error(const location_type& l, const std::string& m)
{
  up.error (l, m);
}

// Local Variables:
// mode: c++
// End:
