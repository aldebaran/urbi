/// \file parser/ugrammar.y
/// \brief Definition of the parser used by the UParser object.
///
/// This parser is defined with bison, using the option %pure_parser
/// to make it reentrant. For more details about reentrancy issues,
/// check the definition of the UServer class.

%expect 0
%require "2.3"
%error-verbose
%defines
%skeleton "lalr1.cc"
// The leading :: are needed to avoid symbol clashes in the
// parser class when it sees a parser namespace occurrence.
%parse-param {::parser::UParser& up}
%parse-param {FlexLexer& scanner}
%lex-param   {::parser::UParser& up}
%lex-param   {FlexLexer& scanner}
%debug

%code requires
{
// Output in ugrammar.hh.
#include "kernel/fwd.hh"
#include "kernel/utypes.hh"
#include "ast/fwd.hh"
#include "parser/fwd.hh"
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

#include <libport/separator.hh>

#include "ast/all.hh"
#include "ast/clone.hh"

#include "object/atom.hh"

#include "parser/tweast.hh"
#include "parser/uparser.hh"
#include "parser/utoken.hh"

  namespace
  {
    typedef yy::parser::location_type loc;

    /// Get the metavar from the specified map.
    template <typename T>
    static
    T*
    metavar (parser::UParser& up, unsigned key)
    {
      return up.tweast_->template take<T> (key);
    }

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

    /// For some reason there are ambiguities bw MetaVar::append_ and
    /// Tweast::append_ when we don't use exactly ast::Exp*.
    inline
    ast::Exp*
    ast_exp (ast::Exp* e)
    {
      return e;
    }



    /*---------------------.
    | Calls, lvalues etc.  |
    `---------------------*/

  /// Store in Var the AST of the parsing of Code.
# define DESUGAR_(Var, Code)				\
  Var = ::parser::parse(::parser::Tweast() << Code)

  /// Store in $$ the AST of the parsing of Code.
  // Fragile in case Bison changes its expansion of $$.
# define DESUGAR(Code)				\
  DESUGAR_(yyval.expr, Code)
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
    /// "<target> . <method> (<arg1>, <arg2>, <arg3>)".
    static
    ast::Call*
    ast_call(const loc& l,
	     ast::Exp* target, libport::Symbol method,
	     ast::Exp* arg1, ast::Exp* arg2, ast::Exp* arg3 = 0)
    {
      ast::Call* res = ast_call(l, target, method);
      res->args_get().push_back(arg1);
      res->args_get().push_back(arg2);
      if (arg3)
	res->args_get().push_back(arg3);
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
    /// \param modifier optional time modifier object.
    /// \return The AST node calling the slot assignment.
    static
    inline
    ast::Exp*
    ast_slot_change (const loc& l,
		     ast::Call* lvalue, libport::Symbol& change,
		     ast::Exp* value, ast::Exp* modifier = 0)
    {
      ast::Exp* res = 0;
      if (modifier)
      {
	libport::Symbol gen = libport::Symbol::fresh(SYMBOL(gen));
	libport::Symbol tag = libport::Symbol::fresh(SYMBOL(tag));
	DESUGAR_
	  (res,
	   "TrajectoryGenerator.new"
	   "(" << lvalue << ", " << value << ", " << modifier << ")"
	   ".run(" << lvalue->args_get().front() << ".locateSlot("
	   "\"" << lvalue->name_get() << "\"),"
	   "\"" << lvalue->name_get() << "\")");
      }
      else
      {
	ast::Call* call =
	  ast_call(l,
		   lvalue->args_get().front(),
		   // FIXME: this new is stupid.  We need to clean
		   // this set of call functions.
		   new libport::Symbol(change),
		   new ast::String(lvalue->location_get(), lvalue->name_get()));
	if (value)
	  call->args_get().push_back(value);
	res = call;
      }
      return res;
    }

    static
    ast::Exp*
    ast_slot_set (const loc& l, ast::Call* lvalue,
		  ast::Exp* value, ast::Object* modifier = 0)
    {
      return ast_slot_change(l, lvalue, SYMBOL(setSlot), value, modifier);
    }

    static
    ast::Exp*
    ast_slot_update (const loc& l, ast::Call* lvalue,
		     ast::Exp* value, ast::Object* modifier = 0)
    {
      return ast_slot_change(l, lvalue, SYMBOL(updateSlot), value, modifier);
    }

    static
    ast::Exp*
    ast_slot_remove  (const loc& l, ast::Call* lvalue)
    {
      return ast_slot_change(l, lvalue, SYMBOL(removeSlot), 0);
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
    // "{ INIT OP WHILE OP (TEST) { BODY | INC } }"
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

      // BODY | INC.
      ast::Exp* loop_body = ast_nary (l, ast::flavor_pipe, body, inc);

      // WHILE OP (TEST) { BODY | INC }.
      ast::While *while_loop =
	new ast::While(l, op, test, ast_scope(l, loop_body));

      // { INIT OP WHILE OP (TEST) { BODY | INC } }.
      return ast_scope(l, ast_nary (l, op, init, while_loop));
    }


    /// Whether the \a e was the empty command.
    static bool
    implicit (const ast::Exp* e)
    {
      const ast::Noop* noop = dynamic_cast<const ast::Noop*>(e);
      return noop;
    }

    /// Complain if \a command is not implicit.
    static void
    warn_implicit(parser::UParser& up, const loc& l, const ast::Exp* e)
    {
      if (implicit(e))
	up.warn(l,
		"implicit empty instruction.  "
		"Use '{}' to make it explicit.");
    }

  } // anon namespace

  /// Direct the call from 'bison' to the scanner in the right parser::UParser.
  inline
    yy::parser::token_type
    yylex(yy::parser::semantic_type* val, yy::location* loc, parser::UParser& up,
	  FlexLexer& scanner)
  {
    return scanner.yylex(val, loc, &up);
  }

} // %code requires.

/* Tokens and nonterminal symbols, with their type */

%token
	TOK_ALIAS        "alias"
	TOK_EQ           "="
	TOK_BREAK        "break"
	TOK_COLON        ":"
	TOK_DEF          "def"
	TOK_DELETE       "delete"
	TOK_MINUS_GT     "->"
	TOK_DOLLAR       "$"
	TOK_COLON_COLON  "::"
	TOK_ELSE         "else"
	TOK_EMIT         "emit"
	TOK_EVENT        "event"
	TOK_EVERY        "every"
	TOK_FREEZEIF     "freezeif"
	TOK_FROM         "from"
	TOK_FUNCTION     "function"
	TOK_IF           "if"
	TOK_IN           "in"
	TOK_LBRACE       "{"
	TOK_LBRACKET     "["
	TOK_LPAREN       "("
	TOK_OBJECT       "object"
	TOK_ONLEAVE      "onleave"
	TOK_POINT        "."
	TOK_RBRACE       "}"
	TOK_RBRACKET     "]"
	TOK_RETURN       "return"
	TOK_RPAREN       ")"
	TOK_STATIC       "static"
	TOK_STOPIF       "stopif"
	TOK_TILDA        "~"
	TOK_TIMEOUT      "timeout"
	TOK_UNALIAS      "unalias"
	TOK_VAR          "var"
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
	TOK_AMPERSAND    "&"
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
  ast::Tag*    tag;
}

%printer { debug_stream() << libport::deref << $$; } <expr> <call> <nary>;

%type <expr>  expr
%type <expr>  expr.opt
%type <expr>  flag
%type <expr>  flags.0
%type <expr>  flags.1
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
%nonassoc "==" "===" "~=" "%=" "=~=" "!=" "!=="
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
  error  { up.ast_set (0); /* FIXME: We should probably free it. */ }
| stmts  { up.ast_set ($1); }
;


/*--------.
| stmts.  |
`--------*/

%type <nary> stmts block;

// Statements: with ";" and ",".
stmts:
  cstmt
  {
    $$ = new ast::Nary();
    if (!implicit ($1))
      $$->push_back ($1);
    else
      delete $1;
  }
| stmts ";" cstmt
  {
    if ($$->back_flavor_get() == ast::flavor_none)
      $$->back_flavor_set ($2, @2);
    if (!implicit ($3))
      $$->push_back($3);
    else
      delete $3;
  }
| stmts "," cstmt
  {
    if ($$->back_flavor_get() == ast::flavor_none)
      $$->back_flavor_set ($2, @2);
    if (!implicit ($3))
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
      $$ = new ast::Noop (@$);
    else
      $$ = $1;
  }
| cstmt "|" cstmt { $$ = ast_bin(@$, $2, $1, $3); }
| cstmt "&" cstmt { $$ = ast_bin(@$, $2, $1, $3); }
;


/*--------------------------.
| tagged and flagged stmt.  |
`--------------------------*/

%type <tag> tag;
tag:
  expr
  {
    $$ = new ast::Tag (@$, $1);
  }
;

stmt:
  tag flags.0 ":" stmt
  {
    $$ = new ast::TaggedStmt (@$, $1, $4);
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
  /* empty */ { $$ = new ast::Noop (@$); }
| expr        { $$ = $1; }
;

// Groups.
%token	TOK_ADDGROUP     "addgroup"
	TOK_DELGROUP     "delgroup"
	TOK_GROUP        "group";
stmt:
  "group" "identifier" "{" identifiers "}"
  {
    DESUGAR("var " << *$2 << " = Object.Group.new;"
	    << *$2 << ".addGroups([" << libport::separate (*$4, ", ") << "])");
  }
| "addgroup" "identifier" "{" identifiers "}"
  {
    DESUGAR(*$2 << ".addGroups([" << libport::separate (*$4, ", ") << "])");
  }
| "delgroup" "identifier" "{" identifiers "}"
  {
    DESUGAR(*$2 << ".removeGroups([" << libport::separate (*$4, ", ") << "])");
  }
| "group" { $$ = 0; }
;

expr:
  "group" "identifier"    { DESUGAR(*$2 << ".members"); }
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
    ast::Exp*
    ast_class (const loc&l, ast::Call* s)
    {
      return ast_slot_set (l, s,
			   ast_call(l, ast_call(l, 0, SYMBOL(Object)),
				    SYMBOL(clone)));
    }
  }
};


block:
  "{" stmts "}" { $$ = $2; }
;

stmt:
  "class" lvalue block
    {
      DESUGAR("var " << ast::clone(*$2) << "= Object.clone|"
	      << "do " << $2 << "{" << static_cast<ast::Exp*>($3) << "}");
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
%token TOK_EXTERNAL "external";
stmt:
  "external" "object" "identifier"
  {
    DESUGAR("'external'.'object'(\"" << take($3) << "\")");
  }
| "external" "var" "identifier" "." "identifier"
	     "from" "identifier"
  {
    DESUGAR("'external'.'var'(\"" << take($3) << "\", "
	    <<               "\"" << take($5) << "\", "
	    <<               "\"" << take($7) << "\")");
  }
| "external" "function" "(" "integer" ")" "identifier" "." "identifier"
	     "from" "identifier"
  {
    DESUGAR("'external'.'function'(" << $4 << ", "
	    <<                    "\"" << take($6) << "\", "
	    <<                    "\"" << take($8) << "\", "
	    <<                    "\"" << take($10) << "\")");
  }
| "external" "event" "(" "integer" ")" "identifier" "." "identifier"
	     "from" "identifier"
  {
    DESUGAR("'external'.'event'(" << $4 << ", "
	    <<                "\"" << take($6) << "\", "
	    <<                "\"" << take($8) << "\", "
	    <<                "\"" << take($10) << "\")");
  }
;

// Events.
stmt:
  "emit" k1_id args                  { $$ = 0; }
| "emit" "(" expr.opt ")" k1_id args { $$ = 0; }
| "event" k1_id formals              { $$ = 0; }
;

// Functions.
stmt:
  // If you want to use something more general than "k1_id", read the
  // comment of k1_id.
  "function" k1_id formals block
    {
      // Compiled as "var name = function args stmt", i.e.,
      // setSlot (name, function args stmt).
      $$ = ast_slot_set(@$, $2,
			new ast::Function (@$, $3, ast_scope (@$, $4)));
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
 lvalue "=" expr namedarguments
    {
      $$ = ast_slot_update(@$, $1, $3, $4);
    }
// FIXME: Factor these four rules
| id "->" id "=" expr
    {
      $$ = ast_call(@$, 0, SYMBOL(setProperty),
		    new ast::String(@1, take($1)),
		    new ast::String(@3, take($3)),
		    $5);
    }
| expr "." id "->" id "=" expr
    {
      $$ = ast_call(@$, $1, SYMBOL(setProperty),
		    new ast::String(@3, take($3)),
		    new ast::String(@5, take($5)),
		    $7);
    }
| id "->" id
    {
      $$ = ast_call(@$, 0, SYMBOL(getProperty),
		    new ast::String(@1, take($1)),
		    new ast::String(@3, take($3)));
    }
| expr "." id "->" id
    {
      $$ = ast_call(@$, $1, SYMBOL(getProperty),
		    new ast::String(@3, take($3)),
		    new ast::String(@5, take($5)));
    }
| "var" lvalue "=" expr namedarguments
    {
      $$ = ast_slot_set(@$, $2, $4, $5);
    }
| "var" lvalue
    {
      $$ = ast_slot_set(@$, $2, ast_call(@$, 0, SYMBOL(nil)));
    }
| "delete" lvalue
    {
      $$ = ast_slot_remove(@$, $2);
    }
;

%token	TOK_SLASH_EQ    "/="
	TOK_MINUS_EQ    "-="
	TOK_MINUS_MINUS "--"
	TOK_PLUS_EQ     "+="
	TOK_PLUS_PLUS   "++"
	TOK_STAR_EQ     "*="
;

expr:
  lvalue "+=" expr { DESUGAR(ast::clone(*$1) << '=' << $1 << '+' << $3); }
| lvalue "-=" expr { DESUGAR(ast::clone(*$1) << '=' << $1 << '-' << $3); }
| lvalue "*=" expr { DESUGAR(ast::clone(*$1) << '=' << $1 << '*' << $3); }
| lvalue "/=" expr { DESUGAR(ast::clone(*$1) << '=' << $1 << '/' << $3); }
;

expr:
  lvalue "--"      { DESUGAR('(' << $1 << "-= 1) + 1"); }
| lvalue "++"      { DESUGAR('(' << $1 << "+= 1) - 1"); }
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
      DESUGAR ("at_(" << $3 << ", " << $5 << ")");
    }
| "at" "(" softtest ")" stmt "onleave" stmt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      warn_implicit(up, @5, $5);
      warn_implicit(up, @7, $7);
      DESUGAR ("at_(" << $3 << ", " << $5 << ", " << $7 << ")");
    }
| "every" "(" expr ")" stmt
    {
      warn_implicit(up, @5, $5);
      DESUGAR ("every_(" << $3 << ", " << $5 << ")");
    }
| "if" "(" expr ")" stmt %prec CMDBLOCK
    {
      warn_implicit(up, @5, $5);
      $$ = new ast::If(@$, $3, $5, new ast::Noop(@$));
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
| "for" "identifier" "in" expr block    %prec CMDBLOCK
    {
      $$ = new ast::Foreach(@$, $1, take($2), $4, $5);
    }
| "freezeif" "(" softtest ")" stmt
    {
      libport::Symbol ex = libport::Symbol::fresh(SYMBOL(freezeif));
      libport::Symbol in = libport::Symbol::fresh(SYMBOL(freezeif));
      DESUGAR
      ("var " << ex << " = " << "new Tag (\"" << ex << "\")|"
       << "var " << in << " = " << "new Tag (\"" << in << "\")|"
       << ex << " : { "
       << "at(" << $3 << ") " << in << ".freeze onleave " << in << ".unfreeze|"
       << in << " : { " << $5 << "| " << ex << ".stop } }");
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
      $$ = new ast::While(@$, $1, new ast::Float(@$, 1), $2);
    }
| "for" "(" expr ")" stmt %prec CMDBLOCK
    {
      libport::Symbol id = libport::Symbol::fresh();
      DESUGAR("for (var " << id << " = " << $3 << ";"
	      << "    0 < " << id << ";"
	      << "    " << id << "--)"
	      << "  " << $5);
    }
| "stopif" "(" softtest ")" stmt
    {
      libport::Symbol tag = libport::Symbol::fresh(SYMBOL(stopif));
      DESUGAR("var " << tag << " = " << "new Tag (\"" << tag << "\")|"
	      << tag << " : { { " << $5 << "|" << tag << ".stop }" << ","
	      << "waituntil(" << $3 << ")|"
	      << tag << ".stop }");
    }
| "timeout" "(" expr ")" stmt
    {
      libport::Symbol tag = libport::Symbol::fresh(SYMBOL(timeout));
      DESUGAR("var " << tag << " = " << "new Tag (\"" << tag << "\")|"
	      << tag << " : { { " << $5 << "|" << tag << ".stop }" << ","
	      << "sleep(" << $3 << ")|"
	      << tag << ".stop }");
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
      $$ = ast_call(@$, 0, SYMBOL(whenever_), $3, $5, new ast::Noop(@$));
    }
| "whenever" "(" softtest ")" stmt "else" stmt
    {
      warn_implicit(up, @5, $5);
      warn_implicit(up, @7, $7);
      $$ = ast_call(@$, 0, SYMBOL(whenever_), $3, $5, $7);
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
	    block   { $$ = ast_scope(@$,  0, $1); }
| "do" expr block   { $$ = ast_scope(@$, $2, $3); }
;

/*---------------------------.
| Function calls, messages.  |
`---------------------------*/

%type <call> lvalue call;
lvalue:
	   id		{ $$ = ast_call(@$,  0, take($1)); }
| expr "." id		{ $$ = ast_call(@$, $1, take($3)); }
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
%token <symbol> TOK_NEW "new";
%type <expr> new;
new:
  "new" "identifier" args
  {
    // Compiled as "id . new (args)".
    $$ = ast_call(@$, ast_call(@$, 0, $2), SYMBOL(new), $3);
  }
;

// Allow Object.new etc.
id: "new";

// There is a shift/reduce conflict that results from the two uses of "new":
//
//   new -> "new" . "identifier" args
//   id  -> "new" .
//    "identifier"  shift, and go to state 108
//    "identifier"  [reduce using rule 89 (id)]
//
// Obviously the shift should win.
%left "new";
%left "identifier";

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
  "function" formals block
    {
      $$ = new ast::Function (@$, $2, ast_scope (@$, $3));
    }
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
  number        { $$ = new ast::Float(@$, $1); }
| time_expr     { $$ = new ast::Float(@$, $1); }
| "string"      { $$ = new ast::String(@$, take($1)); }
| "[" exprs "]" { $$ = new ast::List(@$, $2);	      }
//| "%" name            { $$ = 0; }
;


/*----------------------------.
| slots and literal objects.  |
`----------------------------*/

%token TOK_LPAREN_PIPE "(|"
       TOK_PIPE_RPAREN "|)";
expr:
  "(|" slots "|)" { $$ = $2; }
;

%union { ast::Object* object; };
%type <object> slots slots.1 namedarguments namedarguments.1;
%printer { debug_stream() << libport::deref << $$; } <slot> <object>;

slots:
  /* nothing */  { $$ = new ast::Object(@$); }
| slots.1
;

slots.1:
  slot           { $$ = new ast::Object(@$); $$->slots_get().push_back($1); }
| slots "," slot { $$->slots_get().push_back($3); }
;

/*-----------------.
| namedarguments.  |
`-----------------*/

namedarguments:
  /* empty */      { $$ = 0;  }
| namedarguments.1 { $$ = $1; }
;

namedarguments.1:
  slot
    {
      $$ = new ast::Object(@$);
      $$->slots_get().push_back($1);
    }
| namedarguments.1 slot
    {
      $$->slots_get().push_back($2);
    }
;

%union { ast::Slot* slot; };
%type <slot> slot;
slot:
  "identifier" ":" expr   { $$ = new ast::Slot(@$, take($1), $3); }
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
	TOK_EQ_TILDA_EQ   "=~="
	TOK_EQ_EQ         "=="
	TOK_EQ_EQ_EQ      "==="
	TOK_GT_EQ         ">="
	TOK_GT            ">"
	TOK_LT_EQ         "<="
	TOK_LT            "<"
	TOK_BANG_EQ       "!="
	TOK_BANG_EQ_EQ    "!=="
	TOK_PERCENT_EQ    "%="
	TOK_TILDA_EQ      "~="

	TOK_AMPERSAND_AMPERSAND  "&&"
	TOK_PIPE_PIPE            "||"
;

id:
  "=~="
| "=="
| "==="
| ">="
| ">"
| "<="
| "<"
| "!="
| "!=="
| "%="
| "~="
| "&&"
| "||"
;

expr:
  expr "!="  expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "!==" expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "%="  expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "<"   expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "<="  expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "=="  expr { $$ = ast_call(@$, $1, $2, $3); }
| expr "===" expr { $$ = ast_call(@$, $1, $2, $3); }
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

// Metavariables.
%token TOK__CALL "_call";
lvalue:
  "_call" "(" "integer" ")"    { $$ = metavar<ast::Call> (up, $3); }
;

%token TOK__EXP "_exp";
expr:
  "_exp" "(" "integer" ")"    { $$ = metavar<ast::Exp> (up, $3); }
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
%printer
{
  if ($$)
    debug_stream() << libport::separate (*$$, ", ");
  else
    debug_stream() << "NULL";
} <symbols>;

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
