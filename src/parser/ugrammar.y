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
#include <parser/fwd.hh>

  // Typedef shorthands
  typedef std::pair<ast::rExp, ast::rNary> case_type;
  typedef std::list<case_type> cases_type;
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

#include <object/atom.hh>

#include <parser/ast-factory.hh>
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

#include <parser/tweast.hh>
#include <parser/parse.hh>
#include <parser/parser-impl.hh>
#include <parser/utoken.hh>

  namespace
  {
    /// Get the metavar from the specified map.
    template <typename T>
    static
    T
    metavar (parser::ParserImpl& up, unsigned key)
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

    /// Store in Var the AST of the parsing of Code.
# define DESUGAR_(Var, Tweast)                  \
    Var = ::parser::parse(Tweast)->ast_get()

    /// Store in $$ the result of appending Code to \a tweast.
# define DESUGAR_TWEAST(Code)                   \
    DESUGAR_(yyval.exp, tweast << Code)

  /// Store in $$ the AST of the parsing of Code.
# define DESUGAR(Code)				\
    DESUGAR_(yyval.exp, ::parser::Tweast() << Code)



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
    symbols_to_decs(const ast::symbols_type* symbols, const ast::loc& loc)
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
	TOK_ONEVENT      "onevent"
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
        TOK_INT_MARK     "?"
;
%printer { debug_stream() << $$; } <flavor>;


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
  typedef libport::pod_caster<ast::rExp>  podExp;
  typedef libport::pod_caster<ast::rCall> podCall;
  typedef libport::pod_caster<ast::rNary> podNary;
  typedef libport::pod_caster<ast::rTag>  podTag;

  podExp    exp;
  podCall   call;
  podNary   nary;
  podTag    tag;
};

%printer { debug_stream() << libport::deref << $$; } <exp> <call> <nary>;

%type <exp> exp exp.opt flag flags.0 flags.1 softtest stmt stmt_loop;


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
%left  CMDBLOCK
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
  root  { up.loc_ = @$; }
;

root:
    /* Minimal error recovery, so that all the tokens are read,
       especially the end-of-lines, to keep error messages in sync. */
  error  { up.result_->ast_reset(); /* FIXME: We should probably free it. */ }
| stmts  { up.result_->ast_reset($1); }
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
      $$.value()->push_back ($1);
  }
| stmts ";" cstmt
  {
    if ($$.value()->back_flavor_get() == ast::flavor_none)
      $$.value()->back_flavor_set ($2, @2);
    if (!implicit ($3))
      $$.value()->push_back($3);
  }
| stmts "," cstmt
  {
    if ($$.value()->back_flavor_get() == ast::flavor_none)
      $$.value()->back_flavor_set ($2, @2);
    if (!implicit ($3))
      $$.value()->push_back($3);
  }
;

%type <exp> cstmt;
// Composite statement: with "|" and "&".
cstmt:
  stmt            { assert($1.value()); $$ = $1; }
| cstmt "|" cstmt { $$ = ast_bin(@$, $2, $1, $3); }
| cstmt "&" cstmt { $$ = ast_bin(@$, $2, $1, $3); }
;


/*--------------------------.
| tagged and flagged stmt.  |
`--------------------------*/

%type <tag> tag;
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
| exp         { $$ = $1; }
;

block:
  "{" stmts "}"       { $$ = $2; }
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

%type <exp> proto;
proto:
  visibility exp   { $$ = $2; }
;

%type <exps> protos.1 protos;

protos.1:
  proto               { $$ = new ast::exps_type; $$->push_back ($1); }
// Cannot add this part currently, as the prescanner cuts
//
//    class A : B, C {};
//
// at the comma.
//
// | protos.1 "," proto  { $$->push_back($3); }
;

// A list of parents to derive from.
protos:
  /* nothing */   { $$ = 0; }
| ":" protos.1    { $$ = $2; }
;

%token TOK_CLASS "class";
stmt:
  "class" lvalue protos block
    {
      $$ = ast_class(@2, $2, $3, $4.value());
    }
;

%type <exp> identifier_as_string;
identifier_as_string:
  "identifier"
    {
      $$ = ast_string(@1, $1.value());
    }
;

// Bindings.
%token TOK_EXTERNAL "external";
stmt:
  "external" "object" identifier_as_string
  {
    static ast::ParametricAst a("'external'.'object'(%exp:1)");
    $$ = exp(a % $3.value());
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
 "event" k1_id formals
  {
    if ($3)
      up.warn(@3, "ignored arguments in event declaration");
    delete $3;
    DESUGAR("var " << $2.value()
            << "= Global.Event.new(\"" << $2.value()->name_get() << "\")");
  }
| "emit" k1_id args
  {
    DESUGAR($2.value() << ".'emit'(" << $3 << ")");
  }
| "emit" "(" exp.opt ")" k1_id args
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
      // Compiled as "var name = function args stmt"
      $$ = ast_slot_set(@$, $2.value(),
			new ast::Function (@$, symbols_to_decs($3, @3),
                                           ast_scope (@$, $4.value())));
      delete $3;
    }
| "closure" k1_id formals block
    {
      // Compiled as "var name = closure args stmt"
      $$ = ast_slot_set(@$, $2.value(),
			new ast::Closure (@$, symbols_to_decs($3, @3)
                                          , ast_scope (@$, $4.value())));
      delete $3;
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
# define DESUGAR_ASSIGN(Op, Lvalue, Exp)                                \
  {                                                                     \
    ::parser::Tweast tweast;                                            \
    ast::rCall lvalue = ast_lvalue_once(Lvalue, tweast);                \
    DESUGAR_TWEAST(new_clone(lvalue) << '='                             \
                   << lvalue << Op << ast_exp(Exp));                    \
  }
};

%token	TOK_CARET_EQ    "^="
	TOK_SLASH_EQ    "/="
	TOK_MINUS_EQ    "-="
	TOK_PLUS_EQ     "+="
	TOK_STAR_EQ     "*="
;

exp:
  lvalue "+=" exp    { DESUGAR_ASSIGN('+', $1, $3); }
| lvalue "-=" exp    { DESUGAR_ASSIGN('-', $1, $3); }
| lvalue "*=" exp    { DESUGAR_ASSIGN('*', $1, $3); }
| lvalue "/=" exp    { DESUGAR_ASSIGN('/', $1, $3); }
| lvalue "^=" exp    { DESUGAR_ASSIGN('^', $1, $3); }
;

%token  TOK_MINUS_MINUS "--"
	TOK_PLUS_PLUS   "++"
;
exp:
  lvalue "--"      { DESUGAR('(' << $1.value() << "-= 1) + 1"); }
| lvalue "++"      { DESUGAR('(' << $1.value() << "+= 1) - 1"); }
;


/*-------------.
| Properties.  |
`-------------*/
%token TOK_MINUS_GT     "->";
stmt:
  lvalue "->" id "=" exp
    {
      $$ = ast_call(@$, $1.value()->target_get(), SYMBOL(setProperty),
		    ast_string(@1, $1.value()->name_get()),
		    ast_string(@3, $3.value()),
		    $5);
      $1.value()->counter_dec();
    }
;

exp:
  lvalue "->" id
    {
      $$ = ast_call(@$, $1.value()->target_get(), SYMBOL(getProperty),
		    ast_string(@1, $1.value()->name_get()),
		    ast_string(@3, $3.value()));
      $1.value()->counter_dec();
    }
;

/*---------------------.
| Stmt: Control flow.  |
`---------------------*/

// non-empty-statement: A statement that triggers a warning if empty.
%type <exp> nstmt;
nstmt:
  stmt
    {
      if (implicit($1))
	up.warn(@1,
		"implicit empty instruction.  Use '{}' to make it explicit.");
    }
;

stmt:
  stmt_loop
| "at" "(" exp ")" nstmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      static ast::ParametricAst
        at("at_(%exp:1, detach(%exp:2), nil)");
      $$ = exp (at % $3.value() % $5.value());
    }
| "at" "(" exp ")" nstmt "onleave" nstmt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      static ast::ParametricAst
        at("at_(%exp:1, detach(%exp:2), detach(%exp:3))");
      $$ = exp (at % $3.value() % $5.value() % $7.value());
    }
| "at" "(" exp "~" exp ")" nstmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      libport::Symbol s = libport::Symbol::fresh(SYMBOL(_at_));
      DESUGAR("var " << s << " = persist (" << $3.value() << ","
              << $5.value() << ") | at( " << s << ") " << $7.value());
    }
| "at" "(" exp "~" exp ")" nstmt "onleave" nstmt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      libport::Symbol s = libport::Symbol::fresh(SYMBOL(_at_));
      DESUGAR("var " << s << " = persist (" << $3.value() << ","
              << $5.value() << ") | at( " << s << ") "
              << $7.value() << " onleave " << $9.value() << "");
    }
| "at" "(" "?" "(" exp ")" ")" nstmt
    {
      DESUGAR("at(?(" << $5.value() << ")())" << $8.value());
    }
| "at" "(" "?" "(" exp ")" "(" exps ")" ")" nstmt
    {
      libport::Symbol handle = libport::Symbol::fresh(SYMBOL(handle));
      DESUGAR("detach({while(true) " << $5.value() << ".onEvent(closure ("
	      << handle << ") {"
	      << "if (Pattern.new(" << ast::rExp(new ast::List(@8, $8)) << ").match("
	      << handle << ".values))" << $11.value() << "})})");
    }
| "at" "(" "?" k1_id "(" exps ")" ")" nstmt
    {
      DESUGAR("at(?(" << $4.value() << ") (" << $6
	      << "))" << $9.value());
    }
| "at" "(" "?" k1_id ")" nstmt
    {
      DESUGAR("at(?(" << $4.value() << ")())" << $6.value());
    }
| "every" "(" exp ")" nstmt
    {
      static ast::ParametricAst every("every_(%exp:1, %exp:2)");
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
      libport::Symbol ex = libport::Symbol::fresh(SYMBOL(freezeif));
      libport::Symbol in = libport::Symbol::fresh(SYMBOL(freezeif));
      DESUGAR
      ("var " << ex << " = " << "new Tag (\"" << ex << "\")|"
       << "var " << in << " = " << "new Tag (\"" << in << "\")|"
       << ex << " : { "
       << "at(" << $3.value() << ") " << in << ".freeze onleave "
       << in << ".unfreeze|" << in << " : { " << $5.value() << "| "
       << ex << ".stop } }");
    }
| "stopif" "(" softtest ")" stmt
    {
      libport::Symbol tag = libport::Symbol::fresh(SYMBOL(stopif));
      DESUGAR("var " << tag << " = " << "new Tag (\"" << tag << "\")|"
	      << tag << " : { { " << $5.value() << "|" << tag << ".stop }" << "&"
	      << "{ waituntil(" << $3.value() << ")|"
	      << tag << ".stop } }");
    }
| "timeout" "(" exp ")" stmt
    {
      DESUGAR("stopif ({sleep(" << $3.value() << ") | true}) " << $5.value());
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
| "whenever" "(" exp ")" nstmt %prec CMDBLOCK
    {
      DESUGAR("whenever_(" << $3.value() << ", " << $5.value() << ", nil)");
    }
| "whenever" "(" exp "~" exp ")" nstmt %prec CMDBLOCK
    {
      libport::Symbol s = libport::Symbol::fresh(SYMBOL(_whenever_));
      DESUGAR("var " << s << " = persist (" << $3.value() << ","
              << $5.value() << ") | whenever_(" << s << ".val, "
              << $7.value() << ", nil)");
    }
| "whenever" "(" exp ")" nstmt "else" nstmt
    {
      DESUGAR("whenever_(" << $3.value() << ", " << $5.value()
              << ", " << $7.value() << ")");
    }
| "whenever" "(" exp "~" exp ")" nstmt "else" nstmt
    {
      libport::Symbol s = libport::Symbol::fresh(SYMBOL(_whenever_));
      DESUGAR("var " << s << " = persist (" << $3.value() << ","
              << $5.value() << ") | whenever_(" << s << ".val, "
              << $7.value() << ", " << $9.value() << ")");
    }
| "onevent" "identifier" formals block
    {
      DESUGAR($2.value() << ".onEvent(function " << $3
              << " { " << ast_exp($4.value()) << " })");
    }
| "switch" "(" exp ")" "{" cases "}"
    {
      ::parser::Tweast tweast;
      libport::Symbol switched = libport::Symbol::fresh(SYMBOL(switched));
      tweast << "var " << switched << " = ";
      tweast << $3.value() << ";";
      ast::ParametricAst nil("nil");
      ast::rExp inner = exp(nil);
      rforeach (const case_type& c, *$6)
      {
       ast::ParametricAst a(
         "if (Pattern.new(%exp:1).match(%exp:2)) %exp:3 else %exp:4");
        a % c.first
          % ast_call(@3, switched)
          % c.second
          % inner;
        inner = ast::exp(a);
      }
      tweast << inner;
      $$ = ::parser::parse(tweast)->ast_get();
      delete $6;
    }
;

/*--------.
| Cases.  |
`--------*/

%union { cases_type* cases; };
%type <cases> cases;

cases:
  /* empty */  { $$ = new cases_type();            }
| cases case   { $$ = $1; $$->push_back(take($2)); }
;

%union { case_type* _case; };
%type <_case> case;

case:
  "case" exp ":" stmts  {  $$ = new case_type($2, $4); }
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
      DESUGAR("for (var " << id << " = " << $3.value() << ";"
	      << "    0 < " << id << ";"
	      << "    " << id << "--)"
	      << "  " << $5.value());
    }
| "for" "(" stmt ";" exp ";" stmt ")" stmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "for", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_pipe);
      $$ = ast_for (@$, $1, $3, $5, $7, $9);
    }
| "for" "(" "var" "identifier" in_or_colon exp ")" stmt    %prec CMDBLOCK
    {
      $$ = new ast::Foreach(@$, $1,
                            new ast::Declaration(@4, $4, new ast::Implicit(@4)),
                            $6.value(), $8.value());
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
	   block  { $$ = ast_scope(@$, 0, $1.value()); }
| "do" exp block  { $$ = ast_scope(@$, $2.value(), $3.value()); }
;

/*---------------------------.
| Function calls, messages.  |
`---------------------------*/

%type <call> lvalue call;
lvalue:
	  id	{ $$ = ast_call(@$, $1); }
| exp "." id	{ $$ = ast_call(@$, $1, $3); }
;

id:
  "identifier"
;

call:
  lvalue args
    {
      $$ = $1;
      $1.value()->arguments_set($2);
      $$.value()->location_set(@$);
    }
;

// Instantiation looks a lot like a function call.
%token <symbol> TOK_NEW "new";
%type <exp> new;
new:
  "new" "identifier" args
  {
    // Compiled as "id . new (args)".
    $$ = ast_call(@$, ast_call(@$, $2), SYMBOL(new), $3);
  }
;

// Allow Object.new etc.
id: "new";

exp:
  new   { $$ = $1.value(); }
| call  { $$ = $1.value(); }
;


/*------------.
| Functions.  |
`------------*/

// Anonymous function.
exp:
  "function" formals block
    {
      $$ = new ast::Function (@$, symbols_to_decs($2, @2),
                              ast_scope (@$, $3.value()));
    }
| "closure" formals block
    {
      $$ = new ast::Closure (@$, symbols_to_decs($2, @2),
                             ast_scope (@$, $3.value()));
    }
;


/*----------.
| Numbers.  |
`----------*/

%union { int ival; };
%printer { debug_stream() << $$; } <ival>;
%token <ival>
	TOK_INTEGER    "integer"
        TOK_FLAG       "flag";
%type <exp> exp_integer;
exp_integer:
  "integer"  { $$ = new ast::Float(@$, $1); }
;


%union { float fval; };
%printer { debug_stream() << $$; } <fval>;
%token <fval>
	TOK_FLOAT      "float"
        TOK_DURATION   "duration";
%type <exp> exp_float;
exp_float:
  "float"  { $$ = new ast::Float(@$, $1); }
;


/*-----------.
| duration.  |
`-----------*/

%type <fval> duration;
duration:
  "duration"
| duration "duration" { $$ += $2; }
;


/*-------.
| exp.   |
`-------*/

exp:
  exp_integer
| exp_float
| duration       { $$ = new ast::Float(@$, $1);        }
| "string"       { $$ = new ast::String(@$, take($1)); }
| "[" exps "]"   { $$ = new ast::List(@$, $2);	       }
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
%token <symbol>
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
| "(" exp ")"             { $$ = $2; }
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
| exp           { $$ = $1; }
;


/*------------.
| Modifiers.  |
`------------*/

%union { ::parser::Tweast* tweast; };
%type <tweast> modifier modifiers.1;
%type <exp> modifiers;
%printer { debug_stream() << libport::deref << $$; } <tweast>;

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
      $$ = $1;
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

%union { ast::exps_type* exps; };
%printer
{
  if ($$)
    debug_stream() << libport::separate (*$$, ", ");
  else
    debug_stream() << "NULL";
} <exps>;
%type <exps> exps exps.1 args;

exps:
  /* empty */ { $$ = new ast::exps_type; }
| exps.1      { $$ = $1; }
;

exps.1:
  exp             { $$ = new ast::exps_type; $$->push_back ($1); }
| exps.1 "," exp  { $$->push_back($3); }
;

// Effective arguments: 0 or more arguments in parens, or nothing.
args:
  /* empty */   { $$ = 0; }
| "(" exps ")"  { $$ = $2; }
;


/*-----------.
| softtest.  |
`-----------*/
softtest: exp


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
