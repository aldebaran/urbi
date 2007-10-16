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

#include "libport/separator.hh"

#include "ast/all.hh"
#include "runner/runner.hh"

#include "kernel/uconnection.hh"
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
    new_bin(const yy::parser::location_type& l, ast::flavor_type op,
	    ast::Exp* lhs, ast::Exp* rhs)
    {
      ast::Exp* res = 0;
      assert (lhs);
      assert (rhs);
      switch (op)
      {
	case ast::flavor_and:
	  res = new ast::And (l, lhs, rhs);
	  break;
	case ast::flavor_pipe:
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

    /// "<target> . <method> (<arg1>, <arg2>)".
    static
    ast::Call*
    call (const yy::parser::location_type& l,
	  ast::Exp* target, libport::Symbol* method,
	  ast::Exp* arg1, ast::Exp* arg2)
    {
      assert (method);
      ast::Call* res = call (l, target, method);
      res->args_get().push_back(arg1);
      res->args_get().push_back(arg2);
      return res;
    }


    /// "<lvalue> = <value>".
    /// \param l        source location.
    /// \param lvalue   object and slot to assign to.
    /// \param value    assigned value.
    /// \param declare  whether we also declare the lvalue.
    /// \return The AST node calling the slot assignment.
    static
    ast::Call*
    assign (const yy::parser::location_type& l,
	    ast::Call* lvalue, ast::Exp* value, bool declare = false)
    {
      return call (l,
		   lvalue->args_get().front(),
		   // this new is stupid.  We need to clean
		   // this set of call functions.
		   new libport::Symbol(declare ? "setSlot" : "updateSlot"),
		   new ast::String(lvalue->location_get(),
				   lvalue->name_get().name_get()),
		   value);
    }


    /// Return \a e in a ast::Scope unless it is already one.
    static
    ast::Scope*
    scope (const yy::parser::location_type& l, ast::Exp* e)
    {
      if (ast::Scope* res = dynamic_cast<ast::Scope*>(e))
	return res;
      else
	return new ast::Scope(l, 0, e);
    }

    /// When op can be either of the four cases.
    static
    ast::Exp*
    new_flavor(const yy::parser::location_type& l, ast::flavor_type op,
	       ast::Exp* lhs, ast::Exp* rhs)
    {
      switch (op)
      {
	case ast::flavor_and:
	case ast::flavor_pipe:
	  return new_bin(l, op, lhs, rhs);

	case ast::flavor_comma:
	case ast::flavor_semicolon:
	{
	  ast::Nary* res = new ast::Nary ();
	  res->push_back (lhs);
	  res->push_back (op, rhs);
	  return res;
	}
	default:
	  pabort(op);
      }
    }


    /// Build a for loop.
    // Since we don't have "continue", for is really a sugared
    // while:
    //
    // "for OP ( INIT; TEST; INC ) BODY"
    //
    // ->
    //
    // "{ INIT OP WHILE OP (TEST) { BODY OP INC } }"
    //
    // OP is either ";" or "|".
    static
    ast::Exp*
    for_loop (const yy::parser::location_type& l,
	      ast::flavor_type op,
	      ast::Exp* init, ast::Exp* test, ast::Exp* inc,
	      ast::Exp* body)
    {
      passert (op, op == ast::flavor_pipe || op == ast::flavor_semicolon);
      assert (init);
      assert (test);
      assert (inc);
      assert (body);

      // BODY OP INC.
      ast::Exp* loop_body = new_flavor (l, op, body, inc);

      // WHILE OP (TEST) { BODY OP INC }.
      ast::While *while_loop = new ast::While(l, op, test, scope(l, loop_body));

      // { INIT OP WHILE OP (TEST) { BODY OP INC } }.
      return scope (l, new_flavor (l, op, init, while_loop));
    }

    /// Whether the \a e was the empty command.
    bool
    implicit (const ast::Exp* e)
    {
      const ast::Noop* noop = dynamic_cast<const ast::Noop*>(e);
      return noop && noop->implicit_get();
    }

    /// Issue a warning.
    void
    warn (UParser& up, const yy::parser::location_type& l, const std::string& m)
    {
      std::ostringstream o;
      o << "!!! " << l << ": " << m << "\n" << std::ends;
      up.connection.send(o.str().c_str(), "warning");
    }

    /// Complain if \a command is not implicit.
    void
    warn_implicit(UParser& up,
		  const yy::parser::location_type& l, const ast::Exp* e)
    {
      if (implicit(e))
	warn (up, l,
	      "implicit empty instruction.  "
	      "Use 'noop' to make it explicit.");
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
	TOK_COLON        ":"
	TOK_DEF          "def"
	TOK_DELGROUP     "delgroup"
	TOK_DIR          "->"
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
	TOK_LBRACE       "{"
	TOK_LBRACKET     "["
	TOK_LOOP         "loop"
	TOK_LOOPN        "loopn"
	TOK_LPAREN       "("
	TOK_NEW          "new"
	TOK_NOOP         "noop"
	TOK_NORM         "'n"
	TOK_OBJECT       "object"
	TOK_ONLEAVE      "onleave"
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
;

// id is meant to enclose all the symbols we use as operators.  For
// instance "+" is special so that we do have the regular priority and
// asssociativity, yet we can write "foo . + (2)" and call foo's +.
%type <symbol> id;

%type <symbols> identifiers identifiers.1 formal_args;
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

// man operator

// Operator                        Associativity
// --------                        -------------
// () [] -> .                      left to right
// ! ~ ++ -- - (type) * & sizeof   right to left
// * / %                           left to right
// + -                             left to right
// << >>                           left to right
// < <= > >=                       left to right
// == !=                           left to right
// &                               left to right
// ^                               left to right
// |                               left to right
// &&                              left to right
// ||                              left to right
// ?:                              right to left
// = += -= etc.                    right to left
// ,                               left to right

 /*
   ! < ( so that !m(x) be read as !(m(x)).
 */

%left  "," ";"
%left  "&" "|"
%left  CMDBLOCK
%left  "else" "onleave"

%left "=" "+=" "-=" "*=" "/="
%left "inherits" "disinherits"
%left  "||"
%left  "&&"
%right "^"
%nonassoc "==" "~=" "%=" "=~=" "!="
%nonassoc  ">" ">=" "<" "<="
%left  "+" "-"
%left  "*" "/" "%"
%right  "!" "++" "--" UNARY     /* Negation--unary minus */
%left  "("
%left "."

%right "'n"



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
| stmts
  {
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
    if (!dynamic_cast<ast::Noop*> ($1))
      $$->push_back ($1);
    else
      delete $1;
  }
| stmts ";" cstmt
  {
    if (!dynamic_cast<ast::Noop*> ($3))
      $$->push_back($2, $3);
    else
    {
      delete $3;
      $$->back_flavor_set ($2);
    }
  }
| stmts "," cstmt
  {
    if (!dynamic_cast<ast::Noop*> ($3))
      $$->push_back($2, $3);
    else
    {
      delete $3;
      $$->back_flavor_set ($2);
    }
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
      $$ = new ast::Noop (@$, true);
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
#include "ast/flavor.hh"
};
%union { ast::flavor_type flavor; };
%token <flavor>
	TOK_COMMA        ","
	TOK_SEMICOLON    ";"
	TOK_AND          "&"
	TOK_PIPE         "|"
;

%type <flavor> and.opt pipe.opt;
%printer { debug_stream() << $$; } <flavor>;

// One or zero "&", defaulting to ";".
and.opt:
  /* empty. */  { $$ = ast::flavor_semicolon; }
| "&"
;

// One or zero "|", defaulting to ";".
pipe.opt:
  /* empty. */  { $$ = ast::flavor_semicolon; }
| "|"
;


/*-------.
| Stmt.  |
`-------*/

stmt:
  /* empty */ { $$ = new ast::Noop (@$, true); }
| "noop"      { $$ = new ast::Noop (@$, false); }
| expr        { $$ = $1; }
| "echo" expr namedarguments { $$ = call (@$, 0, $1, $2); }
;

// Groups.
stmt:
  "group"    "identifier" "{" identifiers "}" { $$ = 0; }
| "addgroup" "identifier" "{" identifiers "}" { $$ = 0; }
| "delgroup" "identifier" "{" identifiers "}" { $$ = 0; }
| "group" { $$ = 0; }
;

// Aliases.
stmt:
  "alias" { $$ = 0; }
//| "alias" k1_id k1_id { $$ = 0; }
//| "alias" k1_id { $$ = 0; }
//| "unalias" k1_id { $$ = 0; }
;

// Classes.
%token
	TOK_CLASS       "class"
	TOK_DISINHERITS	"disinherits"
	TOK_INHERITS	"inherits"
;

expr:
  expr "inherits" expr
    { $$ = call (@$, $1, new libport::Symbol("addParent"), $3); }
| expr "disinherits" expr
    { $$ = call (@$, $1, new libport::Symbol("removeParent"), $3); }
| "class" lvalue "{" stmts "}"
    {
      // Compiled as
      // var id; do id { stmts };
      $$ = new_flavor(@$, ast::flavor_semicolon,
		      assign (@1+@2, $2, 0, true),
		      new ast::Scope(@$, $2, $4));
    }
// | "class" lvalue
//    { $$ = assign (@$, $2, 0, true); }
;

stmt:
  OPERATOR        { $$ = 0; }
| OPERATOR_ID tag { $$ = 0; }
| "def" { $$ = 0; }
;

// Variables.
// stmt:
// | OPERATOR_VAR k1_id { $$ = 0; }
// | "var" k1_id { $$ = 0; }
// Duplicates the previous one, and cannot be factored.
// | "def" k1_id { $$ = 0; }
// The following one is incorrect: wrong separator, should be ;.
// | "var" "{" identifiers "}" { $$ = 0; }
// ;

// Bindings.
stmt:
  BINDER "object" k1_id { $$ = 0; }
| BINDER "var" k1_id "from" k1_id { $$ = 0; }
| BINDER "function" "(" "integer" ")" k1_id "from" k1_id { $$ = 0; }
| BINDER "event" "(" "integer" ")" k1_id "from" k1_id { $$ = 0; }
;

// Events.
stmt:
  "emit" k1_id args                  { $$ = 0; }
| "emit" "(" expr.opt ")" k1_id args { $$ = 0; }
| "wait" expr			     { $$ = call (@$, 0, $1, $2); }
| "waituntil" softtest               { $$ = 0; }
| "event" k1_id formal_args          { $$ = 0; }
;

// Functions.
stmt:
  "function" k1_id formal_args "{" stmts "}"
    {
      // Compiled as "name = function args stmt", i.e.,
      // updateSlot (name, function args stmt).
      $$ = assign (@$, $2,
		   new ast::Function (@$, take($3), scope(@4+@6, $5)),
		   true);
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

// These should probably be integrated into k1_id too, but without
// the recursion.

//name:
//  "identifier"             { $$ = call(@$, 0, $1); }
//| "$" "(" expr ")"         { $$ = 0; }
//| name "." "identifier"    { $$ = call(@$, $1, $3); }
//| name "[" expr "]"        { $$ = 0; }
//| name "::" "identifier"   { $$ = 0; } // FIXME: Get rid of it, it's useless.
//;


/*-------------------.
| Stmt: Assignment.  |
`-------------------*/

stmt:
	lvalue "=" expr namedarguments { $$ = assign (@$, $1, $3);        }
| "var" lvalue "=" expr namedarguments { $$ = assign (@$, $2, $4, true);  }
;

%token <symbol>
	TOK_DIV_ASSIGN    "/="
	TOK_MINUS_ASSIGN  "-="
	TOK_MINUS_MINUS   "--"
	TOK_PLUS_ASSIGN   "+="
	TOK_PLUS_PLUS     "++"
	TOK_STAR_ASSIGN   "*="
;

id:
  "/="
| "-="
| "--"
| "+="
| "++"
| "*="
;

expr:
  expr "+=" expr { $$ = call (@$, $1, $2, $3); }
| expr "-=" expr { $$ = call (@$, $1, $2, $3); }
| expr "*=" expr { $$ = call (@$, $1, $2, $3); }
| expr "/=" expr { $$ = call (@$, $1, $2, $3); }
;

expr:
  expr "--"      { $$ = call (@$, $1, $2); }
| expr "++"      { $$ = call (@$, $1, $2); }
;

/*---------------------.
| Stmt: Control flow.  |
`---------------------*/
%token TOK_DO "do";

stmt:
  "at" and.opt "(" softtest ")" stmt %prec CMDBLOCK
    {
      warn_implicit(up, @6, $6);
      $$ = 0;
    }
| "at" and.opt "(" softtest ")" stmt "onleave" stmt
    {
      $$ = 0;
    }
| "do" expr "{" stmts "}"
    {
      $$ = new ast::Scope(@$, $2, $4);
    }
| "every" "(" expr ")" stmt
    {
      $$ = 0;
    }
| "if" "(" expr ")" stmt %prec CMDBLOCK
    {
      warn_implicit(up, @5, $5);
      $$ = new ast::If(@$, $3, $5, new ast::Noop(@$, true));
    }
| "if" "(" expr ")" stmt "else" stmt
    {
      warn_implicit(up, @5, $5);
      $$ = new ast::If(@$, $3, $5, $7);
    }
| "for" pipe.opt "(" stmt ";" expr ";" stmt ")" stmt %prec CMDBLOCK
    {
      $$ = for_loop (@$, $2, $4, $6, $8, $10);
    }
| "foreach" pipe.opt "identifier" "in" expr "{" stmts "}"    %prec CMDBLOCK
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
| "loopn" pipe.opt "(" expr ")" stmt %prec CMDBLOCK
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
    {
      $$ = new ast::Return(@$, $2, false);
    }
| "whenever" "(" softtest ")" stmt %prec CMDBLOCK
    {
      warn_implicit(up, @5, $5);
      $$ = 0;
    }
| "whenever" "(" softtest ")" stmt "else" stmt
    {
      $$ = 0;
    }
| "while" pipe.opt "(" expr ")" stmt %prec CMDBLOCK
    {
      $$ = new ast::While(@$, $2, $4, $6);
    }
;

%type <call> call message;
message:
  id args
    {
      // The target of the message is currently unknown.
      $$ = call (@$, 0, take($1), $2);
    }
;

id:
  "identifier"
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
//  k1_id          { $$ = $1; }
//| "static" k1_id	{ /* FIXME: Fill. */ }
//| k1_id derive   { /* FIXME: Fill. */ }
//| k1_id "'e"	{ /* FIXME: Fill. */ }
//| k1_id "'in"	{ /* FIXME: Fill. */ }
//| k1_id "'out"   { /* FIXME: Fill. */ }
//| k1_id "'n"	{ $$ = 0; }
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
//  k1_id "->" "identifier" { $$ = 0; }
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
  number        { $$ = new ast::Float(@$, $1);        }
| time_expr     { $$ = new ast::Float(@$, $1);        }
| "string"      { $$ = new ast::String(@$, take($1)); }
| "[" exprs "]" { $$ = new ast::List(@$, $2); 	      }
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

id:
//  "!"
  "%"
| "*"
| "+"
  //| "-"
| "/"
| "^"
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

id:
  "=~="
| "=="
| ">="
| ">"
| "<="
| "<"
| "!="
| "%="
| "~="
| "&&"
| "||"
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

// FIXME: This is not good: we are not short-circuiting.  Ideally we
// would like to use an If here, but it's not an expression (yet?).
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

/* It used to be possible to not have the parens for empty identifiers.
   For the time being, this is disabled because it goes against
   factoring.  Might be reintroduced later. */
formal_args:
  "(" identifiers ")" { $$ = $2; }
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
