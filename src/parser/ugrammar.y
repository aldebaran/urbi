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
%lex-param   {UParser* up}
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

  namespace
  {
    typedef yy::parser::location_type loc;

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

    /// Create an AST node storing a float.
    static
    ast::Object*
    ast_object (const loc& l, ufloat val)
    {
      return new ast::Object(l, new object::Float(val));
    }

    /// Create an AST node storing a symbol.
    static
    ast::Object*
    ast_object (const loc& l, const libport::Symbol& val)
    {
      return new ast::Object(l, new object::String(val));
    }

    /// Create an AST node storing a string.
    static
    ast::Object*
    ast_object (const loc& l, const std::string& val)
    {
      return ast_object (l, libport::Symbol(val));
    }


    /*---------------------.
    | Calls, lvalues etc.  |
    `---------------------*/

    /// "<target> . <method> (args)".
    static
    ast::Call*
    ast_call (const loc& l,
	      ast::Exp* target, libport::Symbol method, ast::exps_type* args)
    {
      args->push_front (target);
      ast::Call* res = new ast::Call(l, method, args);
      return res;
    }

    /// "<target> . <method> ()".
    static
    ast::Call*
    ast_call(const loc& l, ast::Exp* target, libport::Symbol method)
    {
      return ast_call(l, target, method, new ast::exps_type);
    }

    /// "<target> . <method> ()".
    static
    ast::Call*
    ast_call(const loc& l, ast::Exp* target, libport::Symbol* method)
    {
      assert (method);
      return ast_call(l, target, take(method));
    }

    /// "<target> . <method> (<arg1>)".
    static
    ast::Call*
    ast_call(const loc& l,
	     ast::Exp* target, libport::Symbol* method, ast::Exp* arg1)
    {
      assert (method);
      ast::Call* res = ast_call(l, target, method);
      res->args_get().push_back(arg1);
      return res;
    }

    /// "<target> . <method> (<arg1>, <arg2>)".
    static
    ast::Call*
    ast_call(const loc& l,
	     ast::Exp* target, libport::Symbol* method,
	     ast::Exp* arg1, ast::Exp* arg2)
    {
      assert (method);
      ast::Call* res = ast_call(l, target, method);
      res->args_get().push_back(arg1);
      res->args_get().push_back(arg2);
      return res;
    }


    /*-----------------.
    | Changing slots.  |
    `-----------------*/

    /// Factor slot_set, slot_update, and slot_remove.
    /// \param l        source location.
    /// \param lvalue   object and slot to change.
    /// \param change   the Urbi method to invoke.
    /// \param value    optional assigned value.
    /// \return The AST node calling the slot assignment.
    static
    inline
    ast::Call*
    ast_slot_change (const loc& l,
		     ast::Call* lvalue, libport::Symbol& change,
		     ast::Exp* value)
    {
      ast::Call* res =
	ast_call(l,
	      lvalue->args_get().front(),
	      // this new is stupid.  We need to clean
	      // this set of call functions.
	      new libport::Symbol(change),
	      ast_object(lvalue->location_get(), lvalue->name_get()));
      if (value)
	res->args_get().push_back(value);
      return res;
    }

    static
    ast::Call*
    ast_slot_set (const loc& l, ast::Call* lvalue, ast::Exp* value)
    {
      return ast_slot_change(l, lvalue, SYMBOL(setSlot), value);
    }

    static
    ast::Call*
    ast_slot_update (const loc& l, ast::Call* lvalue, ast::Exp* value)
    {
      return ast_slot_change(l, lvalue, SYMBOL(updateSlot), value);
    }

    static
    ast::Call*
    ast_slot_remove  (const loc& l, ast::Call* lvalue)
    {
      return ast_slot_change(l, lvalue, SYMBOL(removeSlot), 0);
    }


    /// "<lvalue> = <value>".
    /// \param l        source location.
    /// \param lvalue   object and slot to assign to.
    /// \param value    assigned value.
    /// \param declare  whether we also declare the lvalue.
    /// \return The AST node calling the slot assignment.
    static
    ast::Call*
    ast_assign(const loc& l, ast::Call* lvalue, ast::Exp* value, bool declare)
    {
      return ast_call(l,
		   lvalue->args_get().front(),
		   // this new is stupid.  We need to clean
		   // this set of call functions.
		   new libport::Symbol(declare ? "setSlot" : "updateSlot"),
		   ast_object(lvalue->location_get(), lvalue->name_get()),
		   value);
    }


    /// Return \a e in a ast::Scope unless it is already one.
    static
    ast::Scope*
    ast_scope(const loc& l, ast::Exp* target, ast::Exp* e)
    {
      if (ast::Scope* res = dynamic_cast<ast::Scope*>(e))
	return res;
      else
	return new ast::Scope(l, target, e);
    }

    static
    ast::Scope*
    ast_scope(const loc& l, ast::Exp* e)
    {
      return ast_scope(l, 0, e);
    }

    // Compiled as
    //  {
    //     var res = id . clone ();
    //     res . init (args);
    //     res;
    //  }
    //
    // Used to be compiled as
    //
    //     id . clone () . init (args);
    //
    // but in that case the return value is that of the end of
    // "init".  And we don't want to require the users to end "init"
    // with "self".
    static
    ast::Scope*
    ast_new (const loc& l, libport::Symbol* id, ast::exps_type* args)
    {
      // I wish I could use tweasts here...  Lord, help me.

      // var res = id . clone ();
      ast::Exp* proto = ast_call(l, 0, id);
      // Cannot use a fixed string here, otherwise two successive "new"
      // will conflict.  Delete the slot afterwards?
      ast::Call* res = ast_call(l, 0, libport::Symbol::fresh());
      ast::Exp* decl = ast_slot_set (l, res, ast_call(l, proto, SYMBOL(clone)));

      // res . init (args);
      ast::Exp* init = ast_call(l, res, SYMBOL(init), args);

      // The sequence.
      ast::Nary* seq = new ast::Nary ();
      seq->push_back (decl);
      seq->back_flavor_set(ast::flavor_semicolon);
      seq->push_back (init);
      seq->back_flavor_set(ast::flavor_semicolon);
      seq->push_back (res);

      return ast_scope(l, seq);
    }


    /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
    /// \param op must be & or |.
    static
    ast::Exp*
    ast_bin(const loc& l, ast::flavor_type op, ast::Exp* lhs, ast::Exp* rhs)
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

    /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
    /// \param op can be any of the four cases.
    static
    ast::Exp*
    ast_nary(const loc& l, ast::flavor_type op, ast::Exp* lhs, ast::Exp* rhs)
    {
      switch (op)
      {
	case ast::flavor_and:
	case ast::flavor_pipe:
	  return ast_bin(l, op, lhs, rhs);

	case ast::flavor_comma:
	case ast::flavor_semicolon:
	{
	  ast::Nary* res = new ast::Nary ();
	  res->push_back (lhs);
	  res->back_flavor_set (op);
	  res->push_back (rhs);
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
    ast_for (const loc& l, ast::flavor_type op,
	      ast::Exp* init, ast::Exp* test, ast::Exp* inc,
	      ast::Exp* body)
    {
      passert (op, op == ast::flavor_pipe || op == ast::flavor_semicolon);
      assert (init);
      assert (test);
      assert (inc);
      assert (body);

      // BODY OP INC.
      ast::Exp* loop_body = ast_nary (l, op, body, inc);

      // WHILE OP (TEST) { BODY OP INC }.
      ast::While *while_loop =
	new ast::While(l, op, test, ast_scope(l, loop_body));

      // { INIT OP WHILE OP (TEST) { BODY OP INC } }.
      return ast_scope(l, ast_nary (l, op, init, while_loop));
    }


    /// Whether the \a e was the empty command.
    static bool
    implicit (const ast::Exp* e)
    {
      const ast::Noop* noop = dynamic_cast<const ast::Noop*>(e);
      return noop && noop->implicit_get();
    }

    /// Complain if \a command is not implicit.
    static void
    warn_implicit(UParser& up, const loc& l, const ast::Exp* e)
    {
      if (implicit(e))
	up.warn(l,
		"implicit empty instruction.  "
		"Use 'noop' to make it explicit.");
    }

  } // anon namespace

  /// Direct the call from 'bison' to the scanner in the right UParser.
  inline
  yy::parser::token_type
  yylex(yy::parser::semantic_type* val, yy::location* loc, UParser& up)
  {
    return up.scanner_.yylex(val, loc, &up);
  }

} // %code requires.

/* Tokens and nonterminal symbols, with their type */

%token
	TOK_ADDGROUP     "addgroup"
	TOK_ALIAS        "alias"
	TOK_ASSIGN       "="
	TOK_BREAK        "break"
	TOK_COLON        ":"
	TOK_DEF          "def"
	TOK_DELETE       "delete"
	TOK_DELGROUP     "delgroup"
	TOK_DIR          "->"
	TOK_DOLLAR       "$"
	TOK_DOUBLECOLON  "::"
	TOK_ELSE         "else"
	TOK_EMIT         "emit"
	TOK_EVENT        "event"
	TOK_EVERY        "every"
	TOK_FREEZEIF     "freezeif"
	TOK_FROM         "from"
	TOK_FUNCTION     "function"
	TOK_GROUP        "group"
	TOK_IF           "if"
	TOK_IN           "in"
	TOK_LBRACE       "{"
	TOK_LBRACKET     "["
	TOK_LPAREN       "("
	TOK_NEW          "new"
	TOK_NOOP         "noop"
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
	TOK_UNALIAS      "unalias"
	TOK_VAR          "var"
	TOK_WAITUNTIL    "waituntil"
	TOK_WHENEVER     "whenever"

%token TOK_EOF 0 "end of command"


/*----------.
| Flavors.  |
`----------*/

%code requires
{
#include "ast/flavor.hh"
};
%code
{
/// Generate a parse error for invalid keyword/flavor combination.
#define FLAVOR_CHECK(Loc, Keyword, Flav, Condition)			\
  do									\
    if (!(Condition))							\
    {									\
      error(Loc,							\
	    ("invalid flavor `" + boost::lexical_cast<std::string>(Flav) \
	     + "' for `" Keyword  "'"));				\
      YYERROR;								\
    }									\
  while (0)
};
%union { ast::flavor_type flavor; };
%token <flavor>
	TOK_COMMA        ","
	TOK_SEMICOLON    ";"
	TOK_AND          "&"
	TOK_PIPE         "|"
	TOK_FOR          "for"
	TOK_LOOP         "loop"
	TOK_WHILE        "while"
	TOK_AT           "at"
;
%printer { debug_stream() << $$; } <flavor>;


/*----------.
| Numbers.  |
`----------*/

%union { int ival; }
%token <ival>
	TOK_INTEGER    "integer"
	TOK_FLAG       "flag"
%printer { debug_stream() << $$; } <ival>;

%union { float fval; }
%token <fval>
	TOK_FLOAT      "float"
	TOK_TIME_VALUE "time"
%type <fval> number;
%type <fval> time_expr;
%printer { debug_stream() << $$; } <fval>;


 /*----------.
 | Strings.  |
 `----------*/
%union { std::string* str; }

// FIXME: Arguably, could be a Symbol too.
%token
   <str>  TOK_STRING             "string"
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
	TOK_IDENTIFIER         "identifier"
	TOK_BINDER             "binder"
	TOK_OPERATOR           "operator command"
	TOK_OPERATOR_ID        "operator"
;

// id is meant to enclose all the symbols we use as operators.  For
// instance "+" is special so that we do have the regular priority and
// asssociativity, yet we can write "foo . + (2)" and call foo's +.
%type <symbol> id;

// FIXME: this destructor entails double frees and invalid pointer
// frees.
// %destructor { delete $$; } <symbol>;
%printer { debug_stream() << libport::deref << $$; } <symbol>;


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

%type <expr>  expr
%type <expr>  expr.opt
%type <expr>  flag
%type <expr>  flags.0
%type <expr>  flags.1
%type <expr>  namedarguments
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
%left  "|"
%left  "&"
%left  CMDBLOCK
%left  "else" "onleave"

%left  "=" "+=" "-=" "*=" "/="
%left  "||"
%left  "&&"
%left  "^"
%nonassoc "==" "~=" "%=" "=~=" "!="
%nonassoc "<" "<=" ">" ">="
%left  "<<" ">>"
%left  "+" "-"
%left  "*" "/" "%"
%right "**"
%right "!" "++" "--" UNARY     /* Negation--unary minus */
%left  "("
%left  "."

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
| stmts      { up.command_tree_set ($1); }
;


/*--------.
| stmts.  |
`--------*/

%type <nary>  stmts;

// Statements: with ";" and ",".
stmts:
  cstmt
  {
    $$ = new ast::Nary();
    if (!dynamic_cast<ast::Noop*> ($1))
      $$->push_back ($1);
    else
      delete $1;
  }
| stmts ";" cstmt
  {
    if ($$->back_flavor_get() == ast::flavor_none)
      $$->back_flavor_set ($2, @2);
    if (!dynamic_cast<ast::Noop*> ($3))
      $$->push_back($3);
    else
      delete $3;
  }
| stmts "," cstmt
  {
    if ($$->back_flavor_get() == ast::flavor_none)
      $$->back_flavor_set ($2, @2);
    if (!dynamic_cast<ast::Noop*> ($3))
      $$->push_back($3);
    else
      delete $3;
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
| cstmt "|" cstmt { $$ = ast_bin(@$, $2, $1, $3); }
| cstmt "&" cstmt { $$ = ast_bin(@$, $2, $1, $3); }
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
  TOK_FLAG         { $$ = 0; }
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
| Stmt.  |
`-------*/

stmt:
  /* empty */ { $$ = new ast::Noop (@$, true); }
| "noop"      { $$ = new ast::Noop (@$, false); }
| expr        { $$ = $1; }
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
;

%code
{
  namespace
  {
    /// var s = Object.clone.
    static
    ast::Call*
    ast_class (const loc&l, ast::Call* s)
    {
      return ast_slot_set (l, s,
			   ast_call(l, ast_call(l, 0, SYMBOL(Object)),
				    SYMBOL(clone)));
    }
  }
};

stmt:
  "class" lvalue "{" stmts "}"
    {
      // Compiled as
      // var id = Object.clone; do id { stmts };
      $$ = ast_nary(@$, ast::flavor_semicolon,
		    ast_class (@1+@2, $2),
		    ast_scope(@$, $2, $4));
    }
| "class" lvalue
    {
      $$ = ast_class (@$, $2);
    }
;

stmt:
  TOK_OPERATOR        { $$ = 0; }
| TOK_OPERATOR_ID tag { $$ = 0; }
| "def"               { $$ = 0; }
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
  TOK_BINDER "object" k1_id                                  { $$ = 0; }
| TOK_BINDER "var" k1_id "from" k1_id                        { $$ = 0; }
| TOK_BINDER "function" "(" "integer" ")" k1_id "from" k1_id { $$ = 0; }
| TOK_BINDER "event" "(" "integer" ")" k1_id "from" k1_id    { $$ = 0; }
;

// Events.
stmt:
  "emit" k1_id args                  { $$ = 0; }
| "emit" "(" expr.opt ")" k1_id args { $$ = 0; }
| "waituntil" softtest               { $$ = 0; }
| "event" k1_id formals              { $$ = 0; }
;

// Functions.
stmt:
  "function" k1_id formals "{" stmts "}"
    {
      // Compiled as "var name = function args stmt", i.e.,
      // setSlot (name, function args stmt).
      $$ = ast_assign(@$, $2,
		      new ast::Function (@$, $3, $5),
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
  "identifier"                   { $$ = ast_call(@$, 0, $1); }
| "identifier" "." "identifier"  { $$ = ast_call(@$, ast_call(@1, 0, $1), $3); }
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
	lvalue "=" expr namedarguments { $$ = ast_slot_update (@$, $1, $3); }
| "var" lvalue "=" expr namedarguments { $$ = ast_slot_set    (@$, $2, $4); }
| "var" lvalue { $$ = ast_slot_set(@$, $2, ast_call(@$, 0, SYMBOL(nil)));}
| "delete" lvalue                      { $$ = ast_slot_remove (@$, $2);     }
;

%token <symbol>
	TOK_SLASH_ASSIGN  "/="
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
  expr "+=" expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "-=" expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "*=" expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "/=" expr { $$ = ast_call(@$, $1, $2, $3); }
;

expr:
  expr "--"      { $$ = ast_call(@$, $1, $2); }
| expr "++"      { $$ = ast_call(@$, $1, $2); }
;


/*---------------------.
| Stmt: Control flow.  |
`---------------------*/

stmt:
  "at" "(" softtest ")" stmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      warn_implicit(up, @5, $5);
      $$ = 0;
    }
| "at" "(" softtest ")" stmt "onleave" stmt
    {
      $$ = 0;
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
| "for" "(" stmt ";" expr ";" stmt ")" stmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "for", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_pipe);
      $$ = ast_for (@$, $1, $3, $5, $7, $9);
    }
| "for" "identifier" "in" expr "{" stmts "}"    %prec CMDBLOCK
    {
      $$ = new ast::Foreach(@$, $1, take($2), $4, $6);
    }
| "freezeif" "(" softtest ")" stmt
    {
      $$ = 0;
    }
/*
 *  This loop keyword can't be converted to a for, since it would
 *  cause an ambiguity in the language. Consider this line:
 *
 *      for (42);
 *
 *  It could be either:
 *
 *      for (42)
 *        ;
 *
 *  i.e, while 42 is true execute the empty instruction, either:
 *
 *      for
 *        42;
 *
 *  i.e. execute "42"  forever, with 42 being parenthesized.
 */
| "loop" stmt %prec CMDBLOCK
    {
      $$ = new ast::While(@$, $1, ast_object(@$, 1), $2);
    }
| "for" "(" expr ")" stmt %prec CMDBLOCK
    {
      /*
       * Compiled as
       *
       * {
       *   var ___idx = <expr>;
       *   while (___idx > 0)
       *   {
       *     <stmt>
       *     ___idx--;
       *   }
       * }
       *
       * using the ast_for function.
       */

      // var ___idx = <expr>
      ast::Call *idx = ast_call(@$, 0, libport::Symbol::fresh());
      ast::Call	*init = ast_slot_set(@$, idx, $3);

      // ___idx > 0
      ast::Call *test =
	ast_call(@$, idx, new libport::Symbol(">"), ast_object(@$, 0));
      // ___idx--
      ast::Call *dec = ast_call(@$, idx, libport::Symbol("--"));

      // Put all together into a while.
      $$ = ast_for(@$, $1, init, test, dec, $5);
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
      $$ = new ast::Throw(@$, ast::return_exception, $2);
    }
| "break"
    {
      $$ = new ast::Throw(@$, ast::break_exception, 0);
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
| "while" "(" expr ")" stmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "while", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_pipe);
      $$ = new ast::While(@$, $1, $3, $5);
    }
;


/*---------------------.
| expr: Control flow.  |
`---------------------*/

%token TOK_DO "do";

expr:
	    "{" stmts "}"   { $$ = ast_scope(@$,  0, $2); }
| "do" expr "{" stmts "}"   { $$ = ast_scope(@$, $2, $4); }
;

/*---------------------------.
| Function calls, messages.  |
`---------------------------*/

%type <call> lvalue call;
lvalue:
	   id   { $$ = ast_call(@$,  0, take($1)); }
| expr "." id   { $$ = ast_call(@$, $1, take($3)); }
;

id:
  "identifier"
;

call:
  lvalue args
    {
      $$ = $1;
      $$->args_get().splice($$->args_get().end(), *$2);
      $$->location_set(@$);
    }
;

// Instantiation looks a lot like a function call.
%type <expr> new;
new:
  "new" "identifier" args { $$ = ast_new (@$, $2, $3); }
;

expr:
  new   { $$ = $1; }
| call  { $$ = $1; }
;



//expr:
//  k1_id          { $$ = $1; }
//| "static" k1_id	{ /* FIXME: Fill. */ }
//| k1_id derive   { /* FIXME: Fill. */ }
//| k1_id "'n"	{ /* FIXME: Fill. */ }
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
  "function" formals "{" stmts "}"
    {
      $$ = new ast::Function (@$, $2, $4);
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
  TOK_TIME_VALUE
| time_expr TOK_TIME_VALUE { $$ += $2; }
;


number:
  "integer"  { $$ = $1; }
| "float"
;


/*-------.
| expr.  |
`-------*/

expr:
  number        { $$ = ast_object(@$, $1); }
| time_expr     { $$ = ast_object(@$, $1); }
| "string"      { $$ = ast_object(@$, take($1)); }
| "[" exprs "]" { $$ = new ast::List(@$, $2);	      }
//| "%" name            { $$ = 0; }
| "group" "identifier"    { $$ = 0; }
;



  /*---------.
  | num expr |
  `---------*/
// The name of the operators are the name of the messages.
%token <symbol>
	TOK_BANG       "!"
	TOK_CARET      "^"
	TOK_GT_GT      ">>"
	TOK_LT_LT      "<<"
	TOK_MINUS      "-"
	TOK_PERCENT    "%"
	TOK_PLUS       "+"
	TOK_SLASH      "/"
	TOK_STAR       "*"
	TOK_STAR_STAR  "**"
;

id:
//  "!"
  "%"
| "*"
| "+"
  //| "-"
| "/"
| "^"
| "**"
| "<<"
| ">>"
;

expr:
  expr "+" expr	          { $$ = ast_call(@$, $1, $2, $3); }
| expr "-" expr	          { $$ = ast_call(@$, $1, $2, $3); }
| expr "*" expr	          { $$ = ast_call(@$, $1, $2, $3); }
| expr "**" expr          { $$ = ast_call(@$, $1, $2, $3); }
| expr "/" expr	          { $$ = ast_call(@$, $1, $2, $3); }
| expr "%" expr	          { $$ = ast_call(@$, $1, $2, $3); }
| expr "^" expr	          { $$ = ast_call(@$, $1, $2, $3); }
| expr "<<" expr          { $$ = ast_call(@$, $1, $2, $3); }
| expr ">>" expr          { $$ = ast_call(@$, $1, $2, $3); }
| "-" expr    %prec UNARY { $$ = ast_call(@$, $2, $1); }
| "(" expr ")"            { $$ = $2; }
;

/*--------.
| Tests.  |
`--------*/
%token <symbol>
	TOK_DEQ   "=~="
	TOK_EQU   "=="
	TOK_GEQ   ">="
	TOK_GTH   ">"
	TOK_LEQ   "<="
	TOK_LTH   "<"
	TOK_NEQ   "!="
	TOK_PEQ   "%="
	TOK_REQ   "~="

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
  expr "!="  expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "%="  expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "<"   expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "<="  expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "=="  expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "=~=" expr { $$ = ast_call(@$, $1, $2, $3); }
| expr ">"   expr { $$ = ast_call(@$, $1, $2, $3); }
| expr ">="  expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "~="  expr { $$ = ast_call(@$, $1, $2, $3); }

| "!" expr        { $$ = ast_call(@$, $2, $1); }

| expr "&&" expr  { $$ = ast_call(@$, $1, $2, $3); }
| expr "||" expr  { $$ = ast_call(@$, $1, $2, $3); }
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

%type <symbols> identifiers identifiers.1 formals;
%printer { debug_stream() << libport::separate (*$$, ", "); } <symbols>;

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

// Function formal arguments.
formals:
  /* empty */         { $$ = 0; }
| "(" identifiers ")" { $$ = $2; }
;


/*-----------.
| K1's BIN.  |
`-----------*/

// This syntax is pure bullshit (and admittedly the bull was badly
// sick), but we don't need to do better that k1 itself, which accepts
// this only here.

cstmt:
  k1bin
  {
    ast::Nary* res = new ast::Nary();
    res->push_back ($1);
    up.command_tree_set (res);
  }
;

%token <expr> TOK_K1BIN "BIN data";
%type  <expr> k1bin;
k1bin:
	lvalue "=" "BIN data" { $$ = ast_slot_update (@$, $1, $3); }
| "var" lvalue "=" "BIN data" { $$ = ast_slot_set    (@$, $2, $4); }
;

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
