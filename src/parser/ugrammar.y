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

%code requires // Output in ugrammar.hh.
{
#include <libport/pod-cast.hh>
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

#include <libport/assert.hh>
#include <libport/separator.hh>

#include "ast/all.hh"
#include "ast/new-clone.hh"
#include "ast/parametric-ast.hh"

#include "object/atom.hh"

#include "parser/tweast.hh"
#include "parser/uparser.hh"
#include "parser/utoken.hh"

  namespace
  {
    /// Shorthand.
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
      args->push_front(target);
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

    /// "<target> . <method> (<arg1>)".
    static
    ast::Call*
    ast_call(const loc& l,
	     ast::Exp* target, libport::Symbol method, ast::Exp* arg1)
    {
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
		     ast::Call* lvalue, libport::Symbol change,
		     ast::Exp* value, ast::Exp* modifier = 0)
    {
      ast::Exp* res = 0;
      if (modifier)
      {
        static ast::ParametricAst
          traj("TrajectoryGenerator"
               ".new(%exp:1, %exp:2, %exp:3)"
               ".run(%exp:4.target(%exp:5),"
               "     %exp:6)");
        ast::String* name = new ast::String(l, lvalue->name_get());
        res = exp(traj
                  %lvalue %value %modifier
                  %new_clone(lvalue->args_get().front()) %new_clone(name)
                  %name);
      }
      else
      {
        // FIXME: We leak lvalue itself.
	ast::Call* call =
	  ast_call(l,
		   &lvalue->args_get().front(), change,
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
	  ast::Nary* res = new ast::Nary(l);
	  res->push_back(lhs, op);
	  res->push_back(rhs);
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


    /*---------------.
    | Warnings etc.  |
    `---------------*/

# define NOT_IMPLEMENTED(Loc)                                           \
    pabort(Loc << ": rule not implemented in the parser.\n"             \
	   "Rerun with YYDEBUG=1 in the environment to know more.")

    /// Whether the \a e was the empty command.
    static bool
    implicit (const ast::Exp* e)
    {
      const ast::Noop* noop = dynamic_cast<const ast::Noop*>(e);
      return noop;
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
	TOK_DELETE       "delete"
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
	TOK_ONEVENT      "onevent"
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
%type <fval> number time_expr;
%printer { debug_stream() << $$; } <fval>;


 /*---------.
 | String.  |
 `---------*/
%union { std::string* str; };
%token  <str>  TOK_STRING  "string";
%destructor { delete $$; } <str>;
%printer { debug_stream() << libport::deref << $$; } <str>;


 /*---------.
 | Symbol.  |
 `---------*/

%union
{
  typedef libport::pod_caster<libport::Symbol> symbol_type;
  symbol_type symbol;
}

%token <symbol> TOK_IDENTIFIER "identifier";

// id is meant to enclose all the symbols we use as operators.  For
// instance "+" is special so that we do have the regular priority and
// asssociativity, yet we can write "foo . + (2)" and call foo's +.
%type <symbol> id;

%printer { debug_stream() << $$.value(); } <symbol>;


/*--------------.
| Expressions.  |
`--------------*/

%union
{
  ast::Exp*    expr;
  ast::Call*   call;
  ast::Nary*   nary;
  ast::Tag*    tag;
};

%printer { debug_stream() << libport::deref << $$; } <expr> <call> <nary>;

%type <expr> expr expr.opt flag flags.0 flags.1 softtest stmt;


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
%nonassoc "~"
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
    $$ = new ast::Nary(@$);
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
  stmt            { assert($1); $$ = $1; }
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
  tag flags.0 ":" stmt  { $$ = new ast::TaggedStmt (@$, $1, $4); }
|     flags.1 ":" stmt  { NOT_IMPLEMENTED(@$); }
;

/*--------.
| flags.  |
`--------*/

flag:
  TOK_FLAG         { NOT_IMPLEMENTED(@$); }
;

// One or more "flag"s.
flags.1:
  flag             { NOT_IMPLEMENTED(@$); }
| flags.1 flag     { NOT_IMPLEMENTED(@$); }
;

// Zero or more "flag"s.
flags.0:
  /* When use tags, we use the following rule, but ignore the result.
     So don't abort here.  FIXME: Once flags handled, do something else
     than $$ = 0.  */
  /* empty. */   { $$ = 0;  }
| flags.1        { NOT_IMPLEMENTED(@$); }
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
    DESUGAR("var " << $2 << " = Global.Group.new(" << $4 << ")");
  }
| "addgroup" "identifier" "{" identifiers "}"
  {
    DESUGAR($2 << ".add(" << $4 << ")");
  }
| "delgroup" "identifier" "{" identifiers "}"
  {
    DESUGAR($2 << ".remove(" << $4 << ")");
  }
| "group" { NOT_IMPLEMENTED(@$); }
;

expr:
  "group" "identifier"    { DESUGAR($2 << ".members"); }
;

// Aliases.
stmt:
  "alias"             { NOT_IMPLEMENTED(@$); }
| "alias" k1_id k1_id { NOT_IMPLEMENTED(@$); }
| "alias" k1_id       { NOT_IMPLEMENTED(@$); }
| "unalias" k1_id     { NOT_IMPLEMENTED(@$); }
;

block:
  "{" stmts "}"       { $$ = $2; }
;

// Classes.
%token TOK_CLASS "class";
stmt:
  "class" lvalue block
    {
      ::parser::Tweast tweast;
      libport::Symbol owner = libport::Symbol::fresh(SYMBOL(__class__));
      ast::Call* target = $2;
      if (!dynamic_cast<ast::Implicit*>(&$2->args_get().front()))
      {
        // If the lvalue call is qualified, we need to store the
        // target in a variable to avoid evaluating it several times.
        tweast << "var " << owner
               << " = " << new_clone($2->args_get().front()) << "|";
        ast::exps_type* args2 = new ast::exps_type();
        args2->push_back(new ast::Implicit(@2));
        ast::exps_type* args = new ast::exps_type();
        args->push_back(new ast::Call(@2, owner, args2));
        target = new ast::Call(@2, $2->name_get(), args);
      }
      tweast << "var " << new_clone(target) << "= Object.clone|"
             << "do " << new_clone(target) << " {"
             << "var protoName = "
             << ast_exp(new ast::String(@2, $2->name_get().name_get())) << "|"
             << "function " << ("as" + $2->name_get().name_get()) << "() {self}|"
             << ast_exp($3) << "}";

      $$ = ::parser::parse(tweast);
      delete $2;
    }
| "class" lvalue
    {
      DESUGAR("var " << $2 << "= Object.clone");
    }
;

// Variables.
// stmt:
// | "var" k1_id { NOT_IMPLEMENTED(@$); }
// The following one is incorrect: wrong separator, should be ;.
// | "var" "{" identifiers "}" { NOT_IMPLEMENTED(@$); }
// ;

// Bindings.
%token TOK_EXTERNAL "external";
stmt:
  "external" "object" "identifier"
  {
    DESUGAR("'external'.'object'(\"" << $3 << "\")");
  }
| "external" "var" "identifier" "." "identifier"
	     "from" "identifier"
  {
    DESUGAR("'external'.'var'(\"" << $3 << "\", "
	    <<               "\"" << $5 << "\", "
	    <<               "\"" << $7 << "\")");
  }
| "external" "function" "(" "integer" ")" "identifier" "." "identifier"
	     "from" "identifier"
  {
    DESUGAR("'external'.'function'(" << $4 << ", "
	    <<                    "\"" << $6 << "\", "
	    <<                    "\"" << $8 << "\", "
	    <<                    "\"" << $10 << "\")");
  }
| "external" "event" "(" "integer" ")" "identifier" "." "identifier"
	     "from" "identifier"
  {
    DESUGAR("'external'.'event'(" << $4 << ", "
	    <<                "\"" << $6 << "\", "
	    <<                "\"" << $8 << "\", "
	    <<                "\"" << $10 << "\")");
  }
;


/*---------.
| Events.  |
`---------*/
stmt:
 "event" k1_id formals
  {
    if ($3)
      up.warn(@3, "ignored arguments in event declaration");
    delete $3;
    DESUGAR("var " << $2
            << "= Global.Event.new(\"" << $2->name_get() << "\")");
  }
| "emit" k1_id args
  {
    DESUGAR($2 << ".'emit'(" << $3 << ")");
  }
| "emit" "(" expr.opt ")" k1_id args
  {
    NOT_IMPLEMENTED(@$);
  }
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
  "identifier"
    {
      $$ = ast_call(@$, new ast::Implicit(@$), $1);
    }
| "identifier" "." "identifier"
    {
      $$ = ast_call(@$,
                    ast_call(@1, new ast::Implicit(@$), $1), $3); }
;


/*-------------------.
| Stmt: Assignment.  |
`-------------------*/

stmt:
 lvalue "=" expr namedarguments
    {
      $$ = ast_slot_update(@$, $1, $3, $4);
    }
| "var" lvalue "=" expr namedarguments
    {
      $$ = ast_slot_set(@$, $2, $4, $5);
    }
| "var" lvalue
    {
      $$ = ast_slot_set(@$, $2,
                        ast_call(@$, new ast::Implicit(@$), SYMBOL(nil)));
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
  lvalue "+=" expr { DESUGAR(new_clone($1) << '=' << $1 << '+' << $3); }
| lvalue "-=" expr { DESUGAR(new_clone($1) << '=' << $1 << '-' << $3); }
| lvalue "*=" expr { DESUGAR(new_clone($1) << '=' << $1 << '*' << $3); }
| lvalue "/=" expr { DESUGAR(new_clone($1) << '=' << $1 << '/' << $3); }
;

expr:
  lvalue "--"      { DESUGAR('(' << $1 << "-= 1) + 1"); }
| lvalue "++"      { DESUGAR('(' << $1 << "+= 1) - 1"); }
;


/*-------------.
| Properties.  |
`-------------*/
%token TOK_MINUS_GT     "->";
stmt:
  lvalue "->" id "=" expr
    {
      // FIXME: We leak lvalue itself.
      $$ = ast_call(@$, &$1->args_get().front(), SYMBOL(setProperty),
		    new ast::String(@1, $1->name_get()),
		    new ast::String(@3, $3.value()),
		    $5);
    }
;

expr:
  lvalue "->" id
    {
      // FIXME: lvalue leaks.
      $$ = ast_call(@$, &$1->args_get().front(), SYMBOL(getProperty),
		    new ast::String(@1, $1->name_get()),
		    new ast::String(@3, $3.value()));
    }
;

/*---------------------.
| Stmt: Control flow.  |
`---------------------*/

// non-empty-statement: A statement that triggers a warning if empty.
%type <expr> nstmt;
nstmt:
  stmt
    {
      if (implicit($1))
	up.warn(@1,
		"implicit empty instruction.  Use '{}' to make it explicit.");
    }
;

stmt:
  "at" "(" expr ")" nstmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      static ast::ParametricAst at("at_(%exp:1, detach(%exp:2))");
      $$ = exp (at % $3 % $5);
    }
| "at" "(" expr ")" nstmt "onleave" nstmt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      static ast::ParametricAst at("at_(%exp:1, detach(%exp:2), detach(%exp:3))");
      $$ = exp (at % $3 % $5 % $7);
    }
| "at" "(" expr "~" expr ")" nstmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      libport::Symbol s = libport::Symbol::fresh(SYMBOL(_at_));
      DESUGAR("var " << s << " = persist (" << $3 << "," << $5 << ") |"
	      "at_( " << s << ".val, " << $7 << ")");
    }
| "at" "(" expr "~" expr ")" nstmt "onleave" nstmt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      libport::Symbol s = libport::Symbol::fresh(SYMBOL(_at_));
      DESUGAR("var " << s << " = persist (" << $3 << "," << $5 << ") |"
	      "at_( " << s << ".val, " << $7 << ", " << $9 << ")");
    }
| "every" "(" expr ")" nstmt
    {
      static ast::ParametricAst every("every_(%exp:1, %exp:2)");
      $$ = exp (every % $3 % $5);
    }
| "if" "(" expr ")" nstmt %prec CMDBLOCK
    {
      $$ = new ast::If(@$, $3, $5, new ast::Noop(@$));
    }
| "if" "(" expr ")" nstmt "else" nstmt
    {
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
      $$ = new ast::Foreach(@$, $1, $2, $4, $5);
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
| "whenever" "(" expr ")" nstmt %prec CMDBLOCK
    {
      DESUGAR("whenever_(" << $3 << ", " << $5 << ")");
    }
| "whenever" "(" expr "~" expr ")" nstmt %prec CMDBLOCK
    {
      libport::Symbol s = libport::Symbol::fresh(SYMBOL(_whenever_));
      DESUGAR("var " << s << " = persist (" << $3 << "," << $5 << ") |"
	      "whenever_(" << s << ".val, " << $7 << ")");
    }
| "whenever" "(" expr ")" nstmt "else" nstmt
    {
      DESUGAR("whenever_(" << $3 << ", " << $5 << ", " << $7 << ")");
    }
| "whenever" "(" expr "~" expr ")" nstmt "else" nstmt
    {
      libport::Symbol s = libport::Symbol::fresh(SYMBOL(_whenever_));
      DESUGAR("var " << s << " = persist (" << $3 << "," << $5 << ") |"
	      "whenever_(" << s << ".val, " << $7 << ", " << $9 << ")");
    }
| "while" "(" expr ")" stmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "while", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_pipe);
      $$ = new ast::While(@$, $1, $3, $5);
    }
| "onevent" "identifier" formals block
    {
      DESUGAR($2 << ".onEvent(function " << $3
              << " { " << ast_exp($4) << " })");
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
	   id		{ $$ = ast_call(@$, new ast::Implicit(@$), $1); }
| expr "." id		{ $$ = ast_call(@$, $1, $3); }
;

id:
  "identifier"
;

call:
  lvalue args
    {
      $$ = $1;
      $$->args_get().transfer($$->args_get().end(), *$2);
      delete $2;
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
    $$ = ast_call(@$, ast_call(@$, new ast::Implicit(@$), $2), SYMBOL(new), $3);
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
  "identifier" ":" expr   { $$ = new ast::Slot(@$, $1, $3); }
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


/*----------------.
| Metavariables.  |
`----------------*/

%token TOK__CALL "_call";
lvalue:
  "_call" "(" "integer" ")"    { $$ = metavar<ast::Call> (up, $3); }
;

%token TOK__EXP "_exp";
expr:
  "_exp" "(" "integer" ")"     { $$ = metavar<ast::Exp> (up, $3); }
;

%token TOK__EXPS "_exps";
exprs:
  "_exps" "(" "integer" ")"    { $$ = metavar<ast::exps_type> (up, $3); }
;

%token TOK__FORMALS "_formals";
formals:
  "_formals" "(" "integer" ")" { $$ = metavar<ast::symbols_type> (up, $3); }
;

%token TOK_PERCENT_EXP_COLON "%exp:";
expr:
  "%exp:" "integer"    { $$ = new ast::MetaExp(@$, $2); }
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
%type <exprs> exprs exprs.1 args;

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
softtest: expr


/*----------------------.
| List of identifiers.  |
`----------------------*/

// "var"?
var.opt:
  /* empty. */
| "var"
;

%union { ast::symbols_type* symbols; };
%printer
{
  if ($$)
    debug_stream() << *$$;
  else
    debug_stream() << "NULL";
} <symbols>;
%type <symbols> identifiers identifiers.1 formals;

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
