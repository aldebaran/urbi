/// \file parser/ugrammar.y
/// \brief Definition of the parser used by the ParserImpl object.
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
%parse-param {::parser::ParserImpl& up}
%parse-param {FlexLexer& scanner}
%lex-param   {::parser::ParserImpl& up}
%lex-param   {FlexLexer& scanner}
%debug

%code requires // Output in ugrammar.hh.
{
#include <libport/pod-cast.hh>
#include <list>
#include <kernel/fwd.hh>
#include <ast/fwd.hh>
#include <ast/exps-type.hh>
#include <ast/symbols-type.hh>
#include <object/symbols.hh>
#include <parser/ast-factory.hh>
#include <parser/fwd.hh>
#include <ast/call.hh>
#include <ast/nary.hh>
#include <ast/tag.hh>

  // Typedef shorthands
  typedef std::pair<ast::rExp, ast::exps_type*> event_match_type;

  // It is inconvenient to use the pointer notation with the variants.
  typedef ast::exps_type* exps_pointer;
  typedef ast::symbols_type* symbols_pointer;
  typedef ::parser::Tweast* tweast_pointer;
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

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <libport/assert.hh>
#include <libport/finally.hh>
#include <libport/separator.hh>

#include <ast/all.hh>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <ast/print.hh>

  using parser::ast_bin;
  using parser::ast_call;
  using parser::ast_class;
  using parser::ast_exp;
  using parser::ast_for;
  using parser::ast_scope;
  using parser::ast_slot_remove;
  using parser::ast_slot_set;
  using parser::ast_slot_update;
  using parser::ast_string;
  using parser::ast_switch;

#include <parser/parse.hh>
#include <parser/parser-impl.hh>
#include <parser/tweast.hh>
#include <parser/utoken.hh>

  namespace
  {
    /// Get the metavar from the specified map.
    template <typename T>
    static
    T
    metavar(parser::ParserImpl& up, unsigned key)
    {
      return up.tweast_->template take<T>(key);
    }


    /// Return the parsing of \a Tweast.
    static
    ast::rExp
    desugar(::parser::Tweast& t)
    {
      ast::rExp res = ::parser::parse(t)->ast_get().unsafe_cast<ast::Exp>();
      if (!!getenv("DESUGAR"))
        LIBPORT_ECHO("res: " << get_pointer(res)
                     << ": " << libport::deref << res);
      return res;
    }

  /// Return the parsing of Code.
# define DESUGAR(Code)                          \
    desugar(::parser::Tweast() << Code)


    /*---------------.
    | Warnings etc.  |
    `---------------*/

# define NOT_IMPLEMENTED(Loc)                                           \
  do {                                                                  \
    error(Loc,                                                          \
          ("rule not implemented in the parser.\n"                      \
	   "Rerun with YYDEBUG=1 in the environment to know more."));   \
    YYERROR;                                                            \
  } while (0)

    /// Whether the \a e was the empty command.
    static bool
    implicit (const ast::rExp e)
    {
      ast::rConstNoop noop = e.unsafe_cast<const ast::Noop>();
      return noop;
    }

  } // anon namespace

  /// Use the scanner in the right parser::ParserImpl.
  inline
    yy::parser::token_type
    yylex(yy::parser::semantic_type* val, yy::location* loc,
          parser::ParserImpl& up,
	  FlexLexer& scanner)
  {
    return scanner.yylex(val, loc, &up);
  }

  static ast::declarations_type*
    symbols_to_decs(const ast::symbols_type* symbols,
                    const ast::loc& loc)
  {
    if (!symbols)
      return 0;
    ast::declarations_type* res = new ast::declarations_type();
    foreach (const libport::Symbol& var, *symbols)
      res->push_back(new ast::Declaration(loc, var, new ast::Implicit(loc)));
    return res;
  }

} // %code requires.

/* Tokens and nonterminal symbols, with their type */

%token
	TOK_EQ           "="
	TOK_BREAK        "break"
	TOK_CASE         "case"
	TOK_CLOSURE      "closure"
	TOK_CONTINUE     "continue"
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
	TOK_ONLEAVE      "onleave"
	TOK_POINT        "."
	TOK_RBRACE       "}"
	TOK_RBRACKET     "]"
	TOK_RETURN       "return"
	TOK_RPAREN       ")"
	TOK_STATIC       "static"
	TOK_STOPIF       "stopif"
	TOK_SWITCH       "switch"
	TOK_TILDA        "~"
	TOK_TIMEOUT      "timeout"
	TOK_VAR          "var"
        TOK_WAITUNTIL    "waituntil"
	TOK_WHENEVER     "whenever"

%token TOK_EOF 0 "end of command"


/*----------.
| Flavors.  |
`----------*/

%code requires
{
#include <ast/flavor.hh>
};
%code
{
/// Generate a parse error for invalid keyword/flavor combination.
/// The check is performed by the parser, not the scanner, because
/// some keywords may, or may not, have some flavors dependencies
/// on the syntactic construct.  See the various "for"s for instance.
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

%code variant
{
  ast::flavor_type,
  std::string,
  libport::Symbol,
  ast::rExp,
  ast::rCall,
  ast::rNary,
  ast::rTag,
  ::parser::cases_type,
  ::parser::case_type,
  int,
  float,
  event_match_type,
  tweast_pointer,
  exps_pointer,
  symbols_pointer
};
%printer { debug_stream() << libport::deref << $$; } <ast::rCall>;


%token <ast::flavor_type>
	TOK_COMMA        ","
	TOK_SEMICOLON    ";"
	TOK_AMPERSAND    "&"
	TOK_PIPE         "|"
	TOK_FOR          "for"
	TOK_LOOP         "loop"
	TOK_WHILE        "while"
	TOK_AT           "at"
        TOK_INT_MARK     "?"
;
%printer { debug_stream() << $$; } <ast::flavor_type>;


 /*---------.
 | String.  |
 `---------*/
%token  <std::string>  TOK_STRING  "string";
%printer { debug_stream() << $$; } <std::string>;


 /*---------.
 | Symbol.  |
 `---------*/

%token <libport::Symbol> TOK_IDENTIFIER "identifier";

// id is meant to enclose all the symbols we use as operators.  For
// instance "+" is special so that we do have the regular priority and
// asssociativity, yet we can write "foo . + (2)" and call foo's +.
%type <libport::Symbol> id;

%printer { debug_stream() << $$; } <libport::Symbol>;


/*--------------.
| Expressions.  |
`--------------*/

%printer { debug_stream() << libport::deref << $$; } <ast::rExp>;
%type <ast::rExp> exp exp.opt flag flags.0 flags.1 softtest stmt stmt_loop;


/*----------------------.
| Operator precedence.  |
`----------------------*/

// man operator (precedences are increasing).

// Operator                        Associativity
// --------                        -------------
// ,                               left to right
// = += -= etc.                    right to left
// ?:                              right to left
// ||                              left to right
// &&                              left to right
// |                               left to right
// ^                               left to right
// &                               left to right
// == !=                           left to right
// < <= > >=                       left to right
// << >>                           left to right
// + -                             left to right
// * / %                           left to right
// ! ~ ++ -- - (type) * & sizeof   right to left
// () [] -> .                      left to right

 /*
   ! < ( so that !m(x) be read as !(m(x)).
 */

// This is very painful: because we want to declare the resolution
// in the following case, we give a relative precedence between "in"
// and "identifier" which kills the support of "for (var i in c)".
// We cannot address this properly, unless we have scoped precedences.
//
// Here, because we have "identifier" < "in", we promote the shift,
// which is what we want.  But it is sucks hard.
//
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


// Precedences are increasing)
%left  "," ";"
%left  "|"
%left  "&"
%left  CMDBLOCK ELSE_LESS
%left  "else" "onleave"

%left  "=" "+=" "-=" "*=" "/=" "^="
%nonassoc "~" // This is not the same as in C++, this is for "softest".
%left  "||"
%left  "&&"
%nonassoc "in"
%left  "bitor"
%left  "^"
%left  "bitand"
%nonassoc "==" "===" "~=" "%=" "=~=" "!=" "!=="
%nonassoc "<" "<=" ">" ">="
%left  "<<" ">>"
%left  "+" "-"
%left  "*" "/" "%"
%right "**"
%right "!" "compl" "++" "--" UNARY     /* Negation--unary minus */
%left  "("
%nonassoc "["
%left  "."

/* URBI Grammar */
%%

%start start;
start:
  root
  {
    up.result_->ast_set($1);
    up.loc_ = @$;
  }
;

root:
    /* Minimal error recovery, so that all the tokens are read,
       especially the end-of-lines, to keep error messages in sync. */
  error  { $$ = 0;  }
| stmts  { std::swap($$, $1); }
;


/*--------.
| stmts.  |
`--------*/

%type <ast::rNary> block root stmts;
%printer { debug_stream() << libport::deref << $$; } <ast::rNary>;

// Statements: with ";" and ",".
stmts:
  cstmt
  {
    $$ = new ast::Nary(@$);
    if (!implicit($1))
      $$->push_back($1);
  }
| stmts ";" cstmt
  {
    std::swap($$, $1);
    if ($$->back_flavor_get() == ast::flavor_none)
      $$->back_flavor_set($2, @2);
    if (!implicit($3))
      $$->push_back($3);
  }
| stmts "," cstmt
  {
    std::swap($$, $1);
    if ($$->back_flavor_get() == ast::flavor_none)
      $$->back_flavor_set($2, @2);
    if (!implicit($3))
      $$->push_back($3);
  }
;

%type <ast::rExp> cstmt;
// Composite statement: with "|" and "&".
cstmt:
  stmt            { assert($1); std::swap($$, $1); }
| cstmt "|" cstmt { $$ = ast_bin(@$, $2, $1, $3); }
| cstmt "&" cstmt { $$ = ast_bin(@$, $2, $1, $3); }
;


/*--------------------------.
| tagged and flagged stmt.  |
`--------------------------*/

%type <ast::rTag> tag;
%printer { debug_stream() << libport::deref << $$; } <ast::rTag>;
tag:
  exp
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
| exp         { std::swap($$, $1); }
;

block:
  "{" stmts "}"       { std::swap($$, $2); }
;

/*----------.
| Classes.  |
`----------*/

%token TOK_PRIVATE    "private"
       TOK_PROTECTED  "protected"
       TOK_PUBLIC     "public"
       ;

// A useless optional visibility.
visibility:
  /* Nothing. */
| "private"
| "protected"
| "public"
;

%type <ast::rExp> proto;
proto:
  visibility exp   { std::swap($$, $2); }
;

%type <exps_pointer> protos.1 protos;
%printer { debug_stream() << libport::deref << $$; } <exps_pointer>;

protos.1:
  proto               { $$ = new ast::exps_type; $$->push_back ($1); }
// Cannot add this part currently, as the prescanner cuts
//
//    class A : B, C {};
//
// at the comma.
//
// | protos.1 "," proto  { std::swap($$, $1); $$->push_back($3); }
;

// A list of parents to derive from.
protos:
  /* nothing */   { $$ = 0; }
| ":" protos.1    { std::swap($$, $2); }
;

%token TOK_CLASS "class";
stmt:
  "class" lvalue protos block
    {
      $$ = ast_class(@2, $2, $3, $4);
    }
;

%type <ast::rExp> identifier_as_string;
identifier_as_string:
  "identifier"
    {
      $$ = ast_string(@1, $1);
    }
;

// Bindings.
%token TOK_EXTERNAL "external";
stmt:
  "external" "object" identifier_as_string
  {
    static ast::ParametricAst a("'external'.'object'(%exp:1)");
    $$ = exp(a % $3);
  }
| "external" "var" identifier_as_string "." identifier_as_string
	     "from" identifier_as_string
  {
    static ast::ParametricAst a("'external'.'var'(%exp:1, %exp:2, %exp:3)");
    $$ = exp(a % $3 % $5 % $7);
  }
| "external" "function" "(" exp_integer ")"
             identifier_as_string "." identifier_as_string
	     "from" identifier_as_string
  {
    static ast::ParametricAst
      a("'external'.'function'(%exp:1, %exp:2, %exp:3, %exp:4)");
    $$ = exp(a % $4 % $6 % $8 % $10);
  }
| "external" "event" "(" exp_integer ")"
             identifier_as_string "." identifier_as_string
	     "from" identifier_as_string
  {
    static ast::ParametricAst
      a("'external'.'event'(%exp:1, %exp:2, %exp:3, %exp:4)");
    $$ = exp(a % $4 % $6 % $8 % $10);
  }
;


/*---------.
| Events.  |
`---------*/
stmt:
  "emit" k1_id args %prec CMDBLOCK
  {
    $$ = DESUGAR($2 << ".'emit'(" << $3 << ")");
  }
| "emit" k1_id args "~" exp
  {
    libport::Symbol e = libport::Symbol::fresh("_emit_");
    $$ = DESUGAR("{var " << e << " = " << $2 << ".trigger(" << $3
                 << ") | detach({sleep(" << $5 << ") | "
                 << e << ".'stop'})}");
  }
;


/*------------.
| Functions.  |
`------------*/

stmt:
  // If you want to use something more general than "k1_id", read the
  // comment of k1_id.
  "function" k1_id formals block
    {
      // Compiled as "var name = function args stmt"
      $$ = ast_slot_set(@$, $2,
			new ast::Function(@$, symbols_to_decs($3, @3),
                                          ast_scope (@$, $4)));
    }
| "closure" k1_id formals block
    {
      // Compiled as "var name = closure args stmt"
      $$ = ast_slot_set(@$, $2,
			new ast::Closure(@$, symbols_to_decs($3, @3),
                                         ast_scope (@$, $4)));
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
%type <ast::rCall> k1_id;
k1_id:
  "identifier"                   { $$ = ast_call(@$, $1); }
| "identifier" "." "identifier"  { $$ = ast_call(@$, ast_call(@1, $1), $3); }
;


/*-------------------.
| Stmt: Assignment.  |
`-------------------*/

exp:
  "(" "var" "identifier" ")"
    {
      $$ = new ast::Binding(@$, $3);
    }

stmt:
  lvalue "=" exp modifiers
    {
      $$ = ast_slot_update(@$, $1, $3, $4);
    }
| "var" lvalue "=" exp modifiers
    {
      $$ = ast_slot_set(@$, $2, $4, $5);
    }
| "var" lvalue
    {
      $$ = ast_slot_set(@$, $2, ast_call(@$, SYMBOL(nil)));
    }
| "delete" lvalue
    {
      $$ = ast_slot_remove(@$, $2);
    }
;

%code // Output in ugrammar.cc.
{
  static
    ast::rExp
    desugar_assign(ast::rCall lvalue, libport::Symbol op, ast::rExp exp)
  {
    ::parser::Tweast tweast;
    lvalue = ast_lvalue_once(lvalue, tweast);
    ast::rExp res = desugar(tweast
                            << new_clone(lvalue) << '='
                            << lvalue << op << exp);
    if (!!getenv("DESUGAR"))
    {
      LIBPORT_ECHO("Id: " << typeid(*res).name()
                   << " (" << get_pointer(res) << ")");
      LIBPORT_ECHO("DESUGAR_ASSIGN: " << *res);
    }
    return res;
  }
};

%token <libport::Symbol>
	TOK_CARET_EQ    "^="
	TOK_SLASH_EQ    "/="
	TOK_MINUS_EQ    "-="
	TOK_PLUS_EQ     "+="
	TOK_STAR_EQ     "*="
;

exp:
  lvalue "+=" exp    { $$ = desugar_assign($1, $2, $3); }
| lvalue "-=" exp    { $$ = desugar_assign($1, $2, $3); }
| lvalue "*=" exp    { $$ = desugar_assign($1, $2, $3); }
| lvalue "/=" exp    { $$ = desugar_assign($1, $2, $3); }
| lvalue "^=" exp    { $$ = desugar_assign($1, $2, $3); }
;

%token  TOK_MINUS_MINUS "--"
	TOK_PLUS_PLUS   "++"
;
exp:
  lvalue "--"      { $$ = DESUGAR('(' << $1 << "-= 1) + 1"); }
| lvalue "++"      { $$ = DESUGAR('(' << $1 << "+= 1) - 1"); }
;


/*-------------.
| Properties.  |
`-------------*/
%token TOK_MINUS_GT     "->";
stmt:
  lvalue "->" id "=" exp
    {
      $$ = ast_call(@$, $1->target_get(), SYMBOL(setProperty),
		    ast_string(@1, $1->name_get()),
		    ast_string(@3, $3),
		    $5);
      $1->counter_dec();
    }
;

exp:
  lvalue "->" id
    {
      $$ = ast_call(@$, $1->target_get(), SYMBOL(getProperty),
		    ast_string(@1, $1->name_get()),
		    ast_string(@3, $3));
      $1->counter_dec();
    }
;

/*---------------------.
| Stmt: Control flow.  |
`---------------------*/

// non-empty-statement: A statement that triggers a warning if empty.
%type <ast::rExp> nstmt;
nstmt:
  stmt
    {
      std::swap($$, $1);
      if (implicit($$))
	up.warn(@1,
		"implicit empty instruction.  Use '{}' to make it explicit.");
    }
;

stmt:
  stmt_loop
    {
      std::swap($$, $1);
    }
| "at" "(" exp ")" nstmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      static ast::ParametricAst
        at("Control.at_(%exp:1, detach(%exp:2), nil)");
      $$ = exp (at % $3 % $5);
    }
| "at" "(" exp ")" nstmt "onleave" nstmt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      static ast::ParametricAst
        at("Control.at_(%exp:1, detach(%exp:2), detach(%exp:3))");
      $$ = exp (at % $3 % $5 % $7);
    }
| "at" "(" exp "~" exp ")" nstmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      libport::Symbol s = libport::Symbol::fresh("_at_");
      $$ = DESUGAR("var " << s << " = persist (" << $3 << ","
              << $5 << ") | at( " << s << ") " << $7);
    }
| "at" "(" exp "~" exp ")" nstmt "onleave" nstmt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      libport::Symbol s = libport::Symbol::fresh("_at_");
      $$ = DESUGAR("var " << s << " = persist (" << $3 << ","
              << $5 << ") | at( " << s << ") "
              << $7 << " onleave " << $9 << "");
    }
| "at" "(" event_match ")" nstmt "onleave" nstmt
    {
      libport::Symbol e = libport::Symbol::fresh("_event_");
      $$ = DESUGAR("detach({" << $3.first << ".onEvent(closure ("
	      << e << ") {"
	      << "if (Pattern.new("
	      << ast::rExp(new ast::List(@3, $3.second))
	      << ").match("
	      << e << ".payload)) detach({" << $5
	      << "| waituntil (!" << e << ".active) | " << $7
	      << "})})})");
    }
| "at" "(" event_match ")" nstmt %prec CMDBLOCK
    {
      $$ = DESUGAR("at(?(" << $3.first << ")(" << $3.second << "))"
	      << $5 << " onleave {}");
    }
| "every" "(" exp ")" nstmt
    {
      static ast::ParametricAst every("Control.every_(%exp:1, %exp:2)");
      $$ = exp (every % $3 % $5);
    }
| "if" "(" exp ")" nstmt %prec CMDBLOCK
    {
      $$ = new ast::If(@$, $3, $5, new ast::Noop(@$));
    }
| "if" "(" exp ")" nstmt "else" nstmt
    {
      $$ = new ast::If(@$, $3, $5, $7);
    }
| "freezeif" "(" softtest ")" stmt
    {
      libport::Symbol ex = libport::Symbol::fresh("freezeif");
      libport::Symbol in = libport::Symbol::fresh("freezeif");
      $$ = DESUGAR
        ("var " << ex << " = " << "new Tag (\"" << ex << "\")|"
         << "var " << in << " = " << "new Tag (\"" << in << "\")|"
         << ex << " : { "
         << "at(" << $3 << ") " << in << ".freeze onleave "
         << in << ".unfreeze|" << in << " : { " << $5 << "| "
         << ex << ".stop } }");
    }
| "stopif" "(" softtest ")" stmt
    {
      libport::Symbol tag = libport::Symbol::fresh("stopif");
      $$ = DESUGAR("var " << tag << " = " << "new Tag (\"" << tag << "\")|"
	      << tag << " : { { " << $5 << "|" << tag << ".stop }" << "&"
	      << "{ waituntil(" << $3 << ")|"
	      << tag << ".stop } }");
    }
| "switch" "(" exp ")" "{" cases "}"
    {
      $$ = ast_switch(@3, $3, $6);
    }
| "timeout" "(" exp ")" stmt
    {
      $$ = DESUGAR("stopif ({sleep(" << $3 << ") | true}) " << $5);
    }
| "return" exp.opt
    {
      $$ = new ast::Return(@$, $2);
    }
| "break"
    {
      $$ = new ast::Break(@$);
    }
| "continue"
    {
      $$ = new ast::Continue(@$);
    }
| "waituntil" "(" exp ")"
    {
      static ast::ParametricAst a("Control.'waituntil'(%exp:1)");
      $$ = exp(a % $3);
    }
| "waituntil" "(" exp "~" exp ")"
    {
      libport::Symbol s = libport::Symbol::fresh("_waituntil_");
      $$ = DESUGAR("{var " << s << " = persist (" << $3 << ","
	      << $5 << ") | waituntil(" << s << "())}");
    }
| "waituntil" "(" event_match ")"
    {
      libport::Symbol s = libport::Symbol::fresh("_waituntil_");
      $$ = DESUGAR("var " << s << " = Pattern.new("
	      << ast::rExp(new ast::List(@3, $3.second))
	      << ") | " << $3.first << ".'waituntil'(" << s << ")");
    }
;

// An optional else branch for a whenever.
%type <ast::rExp> else_stmt;
else_stmt:
  /* nothing. */ %prec ELSE_LESS // ELSE_LESS < "else" to promote shift.
  {
    static ast::ParametricAst a("nil");
    $$ = exp(a);
  }
| "else" nstmt
  {
    std::swap($$, $2);
  }
;

stmt:
  "whenever" "(" exp ")" nstmt else_stmt %prec CMDBLOCK
    {
      $$ = DESUGAR("Control.whenever_(" << $3 << ", "
	      << $5 << ", " << $6 << ")");
    }
| "whenever" "(" exp "~" exp ")" nstmt else_stmt  %prec CMDBLOCK
    {
      libport::Symbol s = libport::Symbol::fresh("_whenever_");
      $$ = DESUGAR("var " << s << " = persist (" << $3 << ","
              << $5 << ") | Control.whenever_(" << s << ".val, "
              << $7 << ", " << $8 << ")");
    }
| "whenever" "(" event_match ")" nstmt %prec CMDBLOCK
    {
      $$ = DESUGAR("whenever(?(" << $3.first << ")(" << $3.second << "))"
	      << $5 << " else {}");
    }
| "whenever" "(" event_match ")" nstmt "else" nstmt %prec CMDBLOCK
    {
      libport::Symbol e = libport::Symbol::fresh("_event_");
      $$ = DESUGAR("detach({" << $3.first << ".onEvent(closure ("
	      << e << ") {"
	      << "if (Pattern.new("
	      << ast::rExp(new ast::List(@3, $3.second))
	      << ").match("
	      << e << ".payload)) detach({while (true){" << $5
	      << " | if(!" << e << ".active) break } | " << $7
	      << "})})})");
    }
;

/*--------.
| Cases.  |
`--------*/

%type < ::parser::cases_type > cases;

cases:
  /* empty */  { $$ = ::parser::cases_type();            }
| cases case   { std::swap($$, $1); $$.push_back($2); }
;

%type < ::parser::case_type > case;

case:
  "case" exp ":" stmts  {  $$ = ::parser::case_type($2, $4); }
;

/*--------.
| Loops.  |
`--------*/

stmt_loop:
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
  "loop" stmt %prec CMDBLOCK
    {
      $$ = new ast::While(@$, $1, new ast::Float(@$, 1), $2);
    }
| "for" "(" exp ")" stmt %prec CMDBLOCK
    {
      libport::Symbol id = libport::Symbol::fresh();
      $$ = DESUGAR("for (var " << id << " = " << $3 << ";"
                   << "    0 < " << id << ";"
                   << "    " << id << "--)"
                   << "  " << $5);
    }
| "for" "(" stmt ";" exp ";" stmt ")" stmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "for", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_pipe);
      $$ = ast_for(@$, $1, $3, $5, $7, $9);
    }
| "for" "(" "var" "identifier" in_or_colon exp ")" stmt    %prec CMDBLOCK
    {
      $$ = new ast::Foreach(@$, $1,
                            new ast::Declaration(@4, $4, new ast::Implicit(@4)),
                            $6, $8);
    }
| "while" "(" exp ")" stmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "while", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_pipe);
      $$ = new ast::While(@$, $1, $3, $5);
    }
;

in_or_colon: "in" | ":";

/*--------------------.
| exp: Control flow.  |
`--------------------*/

%token TOK_DO "do";

exp:
	   block  { $$ = ast_scope(@$, 0, $1); }
| "do" exp block  { $$ = ast_scope(@$, $2, $3); }
;

/*---------------------------.
| Function calls, messages.  |
`---------------------------*/

%type <ast::rCall> lvalue call;
lvalue:
	  id	{ $$ = ast_call(@$, $1); }
| exp "." id	{ $$ = ast_call(@$, $1, $3); }
;

id:
  "identifier"  { std::swap($$, $1); }
;

call:
  lvalue args
    {
      std::swap($$, $1);
      $$->arguments_set($2);
      $$->location_set(@$);
    }
;

// Instantiation looks a lot like a function call.
%token <libport::Symbol> TOK_NEW "new";
%type <ast::rExp> new;
new:
  "new" "identifier" args
  {
    // Compiled as "id . new (args)".
    $$ = ast_call(@$, ast_call(@$, $2), SYMBOL(new), $3);
  }
;

// Allow Object.new etc.
id:
  "new" { std::swap($$, $1); }
;

exp:
  new   { std::swap($$, $1); }
| call  { $$ = $1; }  // They don't have the same type, this is not a swap.
;


/*------------.
| Functions.  |
`------------*/

// Anonymous function.
exp:
  "function" formals block
    {
      $$ = new ast::Function(@$, symbols_to_decs($2, @2),
                             ast_scope(@$, $3));
    }
| "closure" formals block
    {
      $$ = new ast::Closure(@$, symbols_to_decs($2, @2),
                            ast_scope(@$, $3));
    }
;


/*----------.
| Numbers.  |
`----------*/

%printer { debug_stream() << $$; } <int>;
%token <int>
	TOK_INTEGER    "integer"
        TOK_FLAG       "flag";
%type <ast::rExp> exp_integer;
exp_integer:
  "integer"  { $$ = new ast::Float(@$, $1); }
;


%printer { debug_stream() << $$; } <float>;
%token <float>
	TOK_FLOAT      "float"
        TOK_DURATION   "duration";
%type <ast::rExp> exp_float;
exp_float:
  "float"  { $$ = new ast::Float(@$, $1); }
;


/*-----------.
| duration.  |
`-----------*/

%type <float> duration;
duration:
  "duration"          { std::swap($$, $1);  }
| duration "duration" { $$ += $2; }
;


/*-------.
| exp.   |
`-------*/

exp:
  exp_integer    { std::swap($$, $1);  }
| exp_float      { std::swap($$, $1);  }
| duration       { $$ = new ast::Float(@$, $1);  }
| "string"       { $$ = new ast::String(@$, $1); }
| "[" exps "]"   { $$ = new ast::List(@$, $2); }
;


/*---------.
| Events.  |
`---------*/

%type <event_match_type> event_match;
event_match:
  "?" "(" exp ")" "(" exps ")"
  {
    $$ = event_match_type($3, $6);
  }
| "?" "(" exp ")"
  {
    $$ = event_match_type($3, new ast::exps_type);
  }
| "?" k1_id "(" exps ")"
  {
    $$ = event_match_type($2, $4);
  }
| "?" k1_id
  {
    $$ = event_match_type($2, new ast::exps_type);
  }
;

/*---------------------------.
| Square brackets operator.  |
`---------------------------*/

exp:
  exp "[" exp "]"
  {
    static ast::ParametricAst rewrite("%exp:1 .'[]'(%exp:2)");
    $$ = ast::exp(rewrite % $1 % $3);
  }
| exp "[" exp "]" "=" exp
  {
    static ast::ParametricAst rewrite("%exp:1 .'[]='(%exp:2, %exp:3)");
    $$ = ast::exp(rewrite % $1 % $3 % $6);
  }
;


/*--------------------.
| special variables.  |
`--------------------*/

%token  TOK_CALL         "call"
        TOK_THIS         "this"
;

exp:
  "this"         { $$ = new ast::This(@$); }
| "call"         { $$ = new ast::CallMsg(@$); }
;

/*-----------.
| num. exp.  |
`-----------*/
// The name of the operators are the name of the messages.
%token <libport::Symbol>
	TOK_BANG       "!"
	TOK_BITAND     "bitand"
	TOK_BITOR      "bitor"
	TOK_CARET      "^"
	TOK_COMPL      "compl"
	TOK_GT_GT      ">>"
	TOK_LT_LT      "<<"
	TOK_MINUS      "-"
	TOK_PERCENT    "%"
	TOK_PLUS       "+"
	TOK_SLASH      "/"
	TOK_STAR       "*"
	TOK_STAR_STAR  "**"
;

exp:
  exp "+" exp	          { $$ = ast_call(@$, $1, $2, $3); }
| exp "-" exp	          { $$ = ast_call(@$, $1, $2, $3); }
| exp "*" exp	          { $$ = ast_call(@$, $1, $2, $3); }
| exp "**" exp            { $$ = ast_call(@$, $1, $2, $3); }
| exp "/" exp	          { $$ = ast_call(@$, $1, $2, $3); }
| exp "%" exp	          { $$ = ast_call(@$, $1, $2, $3); }
| exp "^" exp	          { $$ = ast_call(@$, $1, $2, $3); }
| exp "<<" exp            { $$ = ast_call(@$, $1, $2, $3); }
| exp "bitand" exp        { $$ = ast_call(@$, $1, $2, $3); }
| exp "bitor" exp         { $$ = ast_call(@$, $1, $2, $3); }
| exp ">>" exp            { $$ = ast_call(@$, $1, $2, $3); }
| "-" exp    %prec UNARY  { $$ = ast_call(@$, $2, $1); }
| "!" exp                 { $$ = ast_call(@$, $2, $1); }
| "compl" exp             { $$ = ast_call(@$, $2, $1); }
| "(" exp ")"             { std::swap($$, $2); }
;

/*--------.
| Tests.  |
`--------*/
%token <libport::Symbol>
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

exp:
  exp "!="  exp { $$ = ast_call(@$, $1, $2, $3); }
| exp "!==" exp { $$ = ast_call(@$, $1, $2, $3); }
| exp "%="  exp { $$ = ast_call(@$, $1, $2, $3); }
| exp "<"   exp { $$ = ast_call(@$, $1, $2, $3); }
| exp "<="  exp { $$ = ast_call(@$, $1, $2, $3); }
| exp "=="  exp { $$ = ast_call(@$, $1, $2, $3); }
| exp "===" exp { $$ = ast_call(@$, $1, $2, $3); }
| exp "=~=" exp { $$ = ast_call(@$, $1, $2, $3); }
| exp ">"   exp { $$ = ast_call(@$, $1, $2, $3); }
| exp ">="  exp { $$ = ast_call(@$, $1, $2, $3); }
| exp "~="  exp { $$ = ast_call(@$, $1, $2, $3); }

| exp "&&" exp  { $$ = ast_call(@$, $1, $2, $3); }
| exp "||" exp  { $$ = ast_call(@$, $1, $2, $3); }
;

// e in c => c.has(e).
exp:
  exp "in" exp
  {
    $$ = ast_call(@$, $3, SYMBOL(has), $1);
  }
// "!" is a synonym for "not", typing "not in" is "! in" here.
| exp "!" "in" exp
  {
    $$ = ast_call(@$, ast_call(@$, $4, SYMBOL(has), $1), SYMBOL(BANG));
  }
;

exp.opt:
  /* nothing */ { $$ = 0; }
| exp           { std::swap($$, $1); }
;


/*------------.
| Modifiers.  |
`------------*/

%type <tweast_pointer> modifier modifiers.1;
%type <ast::rExp> modifiers;
%printer { debug_stream() << libport::deref << $$; } <tweast_pointer>;

modifier:
  "identifier" ":" exp
  {
    $$ = new ::parser::Tweast();
    *$$ << ".set(" << ast_string(@1, $1) << ", " << ast_exp($3) << ")";
  }
;

modifiers:
  /* empty */    { $$ = 0;  }
| modifiers.1    { $$ = ::parser::parse(*$1)->ast_get(); }
;

modifiers.1:
  modifier
    {
      $$ = new ::parser::Tweast();
      *$$ << "Dictionary.new" << *$1;
    }
| modifiers.1 modifier
    {
      std::swap($$, $1);
      *$$ << *$2;
    }
;

/*----------------.
| Metavariables.  |
`----------------*/

%token TOK__CALL "_call";
lvalue:
  "_call" "(" "integer" ")"    { $$ = metavar<ast::rCall> (up, $3); }
;

%token TOK__EXP "_exp";
exp:
  "_exp" "(" "integer" ")"     { $$ = metavar<ast::rExp> (up, $3); }
;

%token TOK__EXPS "_exps";
exps:
  "_exps" "(" "integer" ")"    { $$ = metavar<ast::exps_type*> (up, $3); }
;

%token TOK__FORMALS "_formals";
formals:
  "_formals" "(" "integer" ")" { $$ = metavar<ast::symbols_type*> (up, $3); }
;

%token TOK_PERCENT_EXP_COLON "%exp:";
exp:
  "%exp:" "integer"    { $$ = new ast::MetaExp(@$, $2); }
;


/*--------------.
| Expressions.  |
`--------------*/

%type <exps_pointer> exps exps.1 args;

exps:
  /* empty */ { $$ = new ast::exps_type; }
| exps.1      { std::swap($$, $1); }
;

exps.1:
  exp             { $$ = new ast::exps_type; $$->push_back($1); }
| exps.1 "," exp  { std::swap($$, $1); $$->push_back($3); }
;

// Effective arguments: 0 or more arguments in parens, or nothing.
args:
  /* empty */   { $$ = 0; }
| "(" exps ")"  { std::swap($$, $2); }
;


/*-----------.
| softtest.  |
`-----------*/
softtest:
  exp    { std::swap($$, $1);  }
;

/*----------------------.
| List of identifiers.  |
`----------------------*/

// "var"?
var.opt:
  /* empty. */
| "var"
;

%printer { debug_stream() << libport::deref << $$; } <symbols_pointer>;
%type <symbols_pointer> identifiers identifiers.1 formals;

// One or several comma-separated identifiers.
identifiers.1:
  var.opt "identifier"
  {
    $$ = new ast::symbols_type;
    $$->push_back($2);
  }
| identifiers.1 "," var.opt "identifier"
  {
    std::swap($$, $1);
    $$->push_back($4);
  }
;

// Zero or several comma-separated identifiers.
identifiers:
  /* empty */     { $$ = new ast::symbols_type; }
| identifiers.1   { std::swap($$, $1); }
;

// Function formal arguments.
formals:
  /* empty */         { $$ = 0; }
| "(" identifiers ")" { std::swap($$, $2); }
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
