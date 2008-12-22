/// \file parser/ugrammar.y
/// \brief Definition of the parser used by the ParserImpl object.

%require "2.3"
%language "C++"
%error-verbose
%defines
// Instead of "yytoken yylex(yylval, yylloc)", use "symbol_type yylex()".
%define lex_symbol

// Prefix all our external definition of token names with "TOK_".
%define token.prefix "TOK_"

// The leading :: are needed to avoid symbol clashes in the
// parser class when it sees a parser namespace occurrence.
%parse-param {::parser::ParserImpl& up}
%parse-param {yyFlexLexer& scanner}
%lex-param   {::parser::ParserImpl& up}
%lex-param   {yyFlexLexer& scanner}

%code requires // Output in ugrammar.hh.
{
#include <kernel/config.h> // YYDEBUG.

#include <libport/hash.hh>
#include <libport/pod-cast.hh>
#include <libport/ufloat.hh>
#include <list>
#include <kernel/fwd.hh>
#include <ast/catches-type.hh>
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
  typedef std::pair<libport::Symbol, ast::rExp> modifier_type;

  // We need this type everywhere.
  using libport::ufloat;
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
  scanner.loc = up.loc_;
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

  using parser::ast_at;
  using parser::ast_at_event;
  using parser::ast_bin;
  using parser::ast_call;
  using parser::ast_class;
  using parser::ast_for;
  using parser::ast_nil;
  using parser::ast_scope;
  using parser::ast_string;
  using parser::ast_switch;
  using parser::ast_whenever_event;

#include <parser/parse.hh>
#include <parser/parser-impl.hh>
#include <parser/utoken.hh>

  namespace
  {

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


#define ERROR(Loc, Msg, Type)                   \
    do {                                        \
      std::stringstream s;                      \
      s << Msg;                                 \
      up.error(Loc, s.str());                   \
      yylhs.value.destroy<Type>();              \
      YYERROR;                                  \
    } while (false)                             \

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
    yy::parser::symbol_type
    yylex(parser::ParserImpl& up, yyFlexLexer& scanner)
  {
    return scanner.yylex(&up);
  }

  static ast::local_declarations_type*
    symbols_to_decs(ast::symbols_type* symbols,
                    const ast::loc& loc)
  {
    if (!symbols)
      return 0;
    ast::local_declarations_type* res =
      new ast::local_declarations_type();
    foreach (const libport::Symbol& var, *symbols)
      res->push_back(new ast::LocalDeclaration(loc, var, new ast::Implicit(loc)));
    delete symbols;
    return res;
  }

} // %code requires.


/*---------.
| Tokens.  |
`---------*/

%token
        __HERE__     "__HERE__"
        EQ           "="
        BREAK        "break"
        CASE         "case"
        CATCH        "catch"
        CLOSURE      "closure"
        CONTINUE     "continue"
        COLON        ":"
        DEFAULT       "default"
        ELSE         "else"
        EMIT         "emit"
        EVENT        "event"
        EVERY        "every"
        FREEZEIF     "freezeif"
        FUNCTION     "function"
        IF           "if"
        IN           "in"
        LBRACE       "{"
        LBRACKET     "["
        LPAREN       "("
        ONLEAVE      "onleave"
        POINT        "."
        RBRACE       "}"
        RBRACKET     "]"
        RETURN       "return"
        RPAREN       ")"
        STOPIF       "stopif"
        SWITCH       "switch"
        THROW        "throw"
        TILDA        "~"
        TIMEOUT      "timeout"
        TRY          "try"
        VAR          "var"
        WAITUNTIL    "waituntil"
        WHENEVER     "whenever"

%token EOF 0 "end of command"

 // Special tokens for the prescanner.
%token PRE_EOF        "prescanner end of file"
       PRE_WANTS_MORE "prescanner needs more input" // no terminator found
       PRE_COMPLETE   "prescanner found command" // Complete sentence.

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

// We use variants.
%define variant

%printer { debug_stream() << libport::deref << $$; } <ast::rCall>;


%token <ast::flavor_type>
        COMMA        ","
        SEMICOLON    ";"
        AMPERSAND    "&"
        PIPE         "|"
        FOR          "for"
        LOOP         "loop"
        WHILE        "while"
        AT           "at"
;
%printer { debug_stream() << $$; } <ast::flavor_type>;


 /*---------.
 | String.  |
 `---------*/

%token  <std::string>  STRING  "string";
%printer { debug_stream() << $$; } <std::string>;


 /*---------.
 | Symbol.  |
 `---------*/

%token <libport::Symbol> IDENTIFIER "identifier";

// id is meant to enclose all the symbols we use as operators.  For
// instance "+" is special so that we do have the regular priority and
// asssociativity, yet we can write "foo . + (2)" and call foo's +.
%type <libport::Symbol> id;

%printer { debug_stream() << $$; } <libport::Symbol>;


/*--------------.
| Expressions.  |
`--------------*/

%printer { debug_stream() << libport::deref << $$; } <ast::rExp>;
%type <ast::rExp> exp exp.opt softtest stmt stmt_loop;


/*----------------------.
| Operator precedence.  |
`----------------------*/

%expect 0

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

// ":" > "indentifier, so as we get the iteration:
//   for (var i : [1, 2, 3]) ;
//
// not the tagged statement:
//   for ((var i) : 42) ;
%nonassoc ":";


// Precedences are increasing)
%left  "," ";"
%left  "|"
%left  "&"
%left  CMDBLOCK ELSE_LESS
%left  "else" "onleave"

%right  "=" "+=" "-=" "*=" "/=" "^=" "%="

%left VAR

%nonassoc "~" // This is not the same as in C++, this is for "softest".
%left  "||"
%left  "&&"
%nonassoc "in"
%left  "bitor"
%left  "^"
%left  "bitand"
%nonassoc "==" "===" "~=" "=~=" "!=" "!=="
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


/*--------------.
| tagged stmt.  |
`--------------*/

%type <ast::rTag> tag;
%printer { debug_stream() << libport::deref << $$; } <ast::rTag>;
tag:
  exp
  {
    $$ = new ast::Tag (@$, $1);
  }
;

stmt:
  tag ":" stmt
  {
    $$ = new ast::TaggedStmt (@$, $1, ast_scope(@$, $3));
  }
;

/*-------.
| Stmt.  |
`-------*/

stmt:
  /* empty */ { $$ = new ast::Noop (@$, 0); }
| exp         { std::swap($$, $1); }
;

block:
  "{" stmts "}"       { std::swap($$, $2); }
;

/*----------.
| Classes.  |
`----------*/

%token PRIVATE    "private"
       PROTECTED  "protected"
       PUBLIC     "public"
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

%token CLASS "class";
stmt:
  "class" lvalue protos block
    {
      $$ = new ast::Class(@$, $2, $3, $4);
    }
;

%type <ast::rExp> identifier_as_string;
identifier_as_string:
  "identifier"
    {
      $$ = ast_string(@1, $1);
    }
;


/*-----------.
| Bindings.  |
`-----------*/

%token EXTERNAL "external";
stmt:
  "external" "identifier" identifier_as_string
  {
    PARAMETRIC_AST(a, "'external'.'object'(%exp:1)");

    if ($2 != SYMBOL(object))
      up.error (@2, "syntax error, external must be followed by "
                "object, var, function or event");
    $$ = exp(a % $3);
  }
| "external" "var" identifier_as_string "." identifier_as_string
	     "identifier" identifier_as_string
  {
    PARAMETRIC_AST(a, "'external'.'var'(%exp:1, %exp:2, %exp:3)");

    if ($6 != SYMBOL(from))
      up.error(@6, "unexpected `" + $6.name_get() +
	       "', expecting `from'");
    $$ = exp(a % $3 % $5 % $7);
  }
| "external" "function" "(" exp_float ")"
             identifier_as_string "." identifier_as_string
	     "identifier" identifier_as_string
  {
    PARAMETRIC_AST
      (     a, "'external'.'function'(%exp:1, %exp:2, %exp:3, %exp:4)");

    if ($9 != SYMBOL(from))
      up.error(@9, "unexpected `" + $9.name_get() +
	       "', expecting `from'");
    $$ = exp(a % $4 % $6 % $8 % $10);
  }
| "external" "event" "(" exp_float ")"
             identifier_as_string "." identifier_as_string
	     "identifier" identifier_as_string
  {
    PARAMETRIC_AST
      (     a, "'external'.'event'(%exp:1, %exp:2, %exp:3, %exp:4)");

    if ($9 != SYMBOL(from))
      up.error(@9, "unexpected `" + $9.name_get() +
	       "', expecting `from'");
    $$ = exp(a % $4 % $6 % $8 % $10);
  }
;


/*---------.
| Events.  |
`---------*/

stmt:
  "emit" k1_id %prec CMDBLOCK
  {
    $$ = new ast::Emit(@$, $2, 0, 0);
  }
| "emit" k1_id "~" exp %prec CMDBLOCK
  {
    $$ = new ast::Emit(@$, $2, 0, $4);
  }
| "emit" k1_id args %prec CMDBLOCK
  {
    $$ = new ast::Emit(@$, $2, $3, 0);
  }
| "emit" k1_id args "~" exp
  {
    $$ = new ast::Emit(@$, $2, $3, $5);
  }
;


/*------------.
| Functions.  |
`------------*/

stmt:
  // If you want to use something more general than "k1_id", read the
  // comment of k1_id.
  "function" k1_id formals doc block
    {
      // Compiled as "var name = function args stmt"
      $$ = new ast::Declaration(@$, $2,
                                new ast::Function(@$, symbols_to_decs($3, @3),
                                                  ast_scope (@$, $5)), 0);
      $$->doc_set($4);
    }
| "closure" k1_id formals doc block
    {
      if (!$3)
	error(@$, "closure cannot be lazy");
      // Compiled as "var name = closure args stmt"
      $$ = new ast::Declaration(@$, $2,
                                new ast::Closure(@$, symbols_to_decs($3, @3),
                                                 ast_scope (@$, $5)), 0);
      $$->doc_set($4);
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
// "function a ()", "function a.b()", and so on.
//
// Another option would have been to use two keywords, say using "lambda"
// for anonymous functions.  But that's not a good option IMHO (AD).
%type <ast::rCall> k1_id;
k1_id:
  "identifier"               { $$ = ast_call(@$, $1); }
| k1_id "." "identifier"     { $$ = ast_call(@$, ast::rExp($1), $3); }
;


/*----------.
| Modifiers |
`----------*/

%type <modifier_type> modifier;
%type <ast::modifiers_type*> modifiers;

modifier:
  "identifier" ":" exp
  {
    $$.first = $1;
    $$.second = $3;
  }
;

modifiers:
  modifier
    {
      $$ = new ast::modifiers_type();
      (*$$)[$1.first] = $1.second;
    }
| modifiers "," modifier
    {
      $$ = $1;
      (*$$)[$3.first] = $3.second;
    }
;

/*-------------------.
| Stmt: Assignment.  |
`-------------------*/

exp:
  exp "=" exp
    {
      $$ = new ast::Assign(@$, $1, $3, 0);
    }
| exp "=" "(" exp "," modifiers ")"
    {
      $$ = new ast::Assign(@$, $1, $4, $6);
    }

%token <libport::Symbol>
        CARET_EQ    "^="
        MINUS_EQ    "-="
        PERCENT_EQ  "%="
        PLUS_EQ     "+="
        SLASH_EQ    "/="
        STAR_EQ     "*="
;

exp:
  lvalue "+=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, 0, $2); }
| lvalue "-=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, 0, $2); }
| lvalue "*=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, 0, $2); }
| lvalue "/=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, 0, $2); }
| lvalue "^=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, 0, $2); }
| lvalue "%=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, 0, $2); }
;

%token  MINUS_MINUS "--"
        PLUS_PLUS   "++"
;
exp:
  lvalue "--"      { $$ = new ast::Decrementation(@$, $1); }
| lvalue "++"      { $$ = new ast::Incrementation(@$, $1); }
;


/*-------------.
| Properties.  |
`-------------*/

%token MINUS_GT     "->";
exp:
  lvalue "->" id
    {
      $$ = new ast::Property(@$, $1->call(), $3);
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
      $$ = ast_at(@$, $3, $5);
    }
| "at" "(" exp ")" nstmt "onleave" nstmt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      $$ = ast_at(@$, $3, $5, $7);
    }
| "at" "(" exp "~" exp ")" nstmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      $$ = ast_at(@$, $3, $7, 0, $5);
    }
| "at" "(" exp "~" exp ")" nstmt "onleave" nstmt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      $$ = ast_at(@$, $3, $7, $9, $5);
    }
| "at" "(" event_match ")" nstmt %prec CMDBLOCK
    {
      $$ = ast_at_event(@$, $3.first, new ast::List(@3, $3.second), $5);
    }
| "at" "(" event_match ")" nstmt "onleave" nstmt
    {
      $$ = ast_at_event(@$, $3.first, new ast::List(@3, $3.second), $5, $7);
    }
| "every" "(" exp ")" nstmt
    {
      PARAMETRIC_AST(every, "Control.every_(%exp:1, %exp:2)");
      $$ = exp (every % $3 % $5);
    }
| "if" "(" stmts ")" nstmt %prec CMDBLOCK
    {
      $$ = new ast::If(@$, $3,
		       ast_scope(@$,$5),
		       ast_scope(@$,new ast::Noop(@$, 0)));
    }
| "if" "(" stmts ")" nstmt "else" nstmt
    {
      $$ = new ast::If(@$, $3,
		       ast_scope(@$,$5),
		       ast_scope(@$,$7));
    }
| "freezeif" "(" softtest ")" stmt
    {
      PARAMETRIC_AST(desugar,
        "var '$freezeif_ex' = Tag.new(\"$freezeif_ex\") |"
        "var '$freezeif_in' = Tag.new(\"$freezeif_in\") |"
        "'$freezeif_ex' :"
        "{"
        "  at(%exp:1)"
        "    '$freezeif_in'.freeze"
        "  onleave"
        "    '$freezeif_in'.unfreeze |"
        "  '$freezeif_in' :"
        "  {"
        "    %exp:2 |"
        "    '$freezeif_ex'.stop |"
        "    "
        "  }"
        "}"
        );
      $$ = exp(desugar % $3 % $5);
    }
| "stopif" "(" softtest ")" stmt
    {
      PARAMETRIC_AST(desugar,
        "{"
        "  var '$stopif' = Tag.new(\"$stopif\") |"
        "  '$stopif':"
        "  {"
        "    { %exp:1 | '$stopif'.stop } &"
        "    { waituntil(%exp:2) | '$stopif'.stop }"
        "  } |"
        "}"
        );
      $$ = exp(desugar % $5 % $3);
    }
| "switch" "(" exp ")" "{" cases "}"
    {
      $$ = ast_switch(@3, $3, $6, 0);
    }
| "switch" "(" exp ")" "{" cases "default" ":" stmts "}"
    {
      $$ = ast_switch(@3, $3, $6, $9);
    }
| "timeout" "(" exp ")" stmt
    {
      $$ = ::parser::ast_timeout($3, $5);
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
      $$ = ::parser::ast_waituntil($3);
    }
| "waituntil" "(" exp "~" exp ")"
    {
      PARAMETRIC_AST(desugar,
        "{"
        "  var '$waituntil' = persist(%exp:1, %exp:2) |"
        "  waituntil('$waituntil'())"
        "}"
        );
      $$ = exp(desugar % $3 % $5);
    }
| "waituntil" "(" event_match ")"
    {
      $$ = ::parser::ast_waituntil_event(@$, $3.first, $3.second);
    }
;

// An optional else branch for a whenever.
%type <ast::rExp> else_stmt;
else_stmt:
  /* nothing. */ %prec ELSE_LESS // ELSE_LESS < "else" to promote shift.
  {
    $$ = ast_nil();
  }
| "else" nstmt
  {
    std::swap($$, $2);
  }
;

stmt:
  "whenever" "(" exp ")" nstmt else_stmt %prec CMDBLOCK
    {
      PARAMETRIC_AST(desugar,
        "Control.whenever_(%exp:1, %exp:2, %exp:3)");
      $$ = exp(desugar % $3 % $5 % $6);
    }
| "whenever" "(" exp "~" exp ")" nstmt else_stmt  %prec CMDBLOCK
    {
      PARAMETRIC_AST(desugar,
        "var '$whenever' = persist(%exp:1, %exp:2) |"
        "Control.whenever_('$whenever'.val, %exp:3, %exp:4) |'"
        );
      $$ = exp(desugar % $3 % $5 % $7 % $8);
    }
| "whenever" "(" event_match ")" nstmt %prec CMDBLOCK
    {
      $$ = ast_whenever_event(@$, $3.first,
                              ast::rExp(new ast::List(@3, $3.second)), $5);
    }
| "whenever" "(" event_match ")" nstmt "else" nstmt %prec CMDBLOCK
    {
      $$ = ast_whenever_event(@$, $3.first,
                              ast::rExp(new ast::List(@3, $3.second)), $5, $7);
    }
;

/*--------.
| Cases.  |
`--------*/

%type <::parser::cases_type> cases;
%printer { debug_stream() << $$; } <::parser::cases_type>;

cases:
  /* empty */  { $$ = ::parser::cases_type();   }
| cases case   { std::swap($$, $1); $$.push_back($2); }
;

%type <::parser::case_type> case;
%printer { debug_stream() << $$; } <::parser::case_type>;

case:
  "case" match ":" stmts  {  $$ = ::parser::case_type($2, $4); }
;

/*-------------.
| Exceptions.  |
`-------------*/

%type <ast::catches_type> catches;
catches:
  /* empty */ { $$ = ast::catches_type(); }
| catches catch { std::swap($$, $1); $$.push_back($2); }
;

%type <ast::rMatch> match;
match:
  exp           { $$ = new ast::Match(@$, $1, 0);  }
| exp "if" exp  { $$ = new ast::Match(@$, $1, $3); }


%type <ast::rCatch> catch;
catch:
  "catch" "(" match ")" block
  {
    $$ = new ast::Catch(@$, $3, $5);
  }
| "catch" block
  {
    $$ = new ast::Catch(@$, 0, $2);
  }
;

stmt:
  "try" block catches
  {
    $$ = new ast::Try(@$, ast_scope(@$,$2), $3);
  }
| "throw"
  {
    $$ = new ast::Throw(@$, 0);
  }
| "throw" exp
  {
    $$ = new ast::Throw(@$, $2);
  }
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
      $$ = new ast::While(@$, $1, new ast::Float(@$, 1),
			  ast_scope(@$,$2));
    }
| "for" "(" exp ")" stmt %prec CMDBLOCK
    {
      PARAMETRIC_AST(desugar,
        "for (var '$for' = %exp:1;"
        "     0 < '$for';" // Use 0 < n, not n > 0, since < is quicker
        "     '$for'--)"
        "  %exp:2"
        );
      $$ = exp(desugar % $3 % $5);
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
                            new ast::LocalDeclaration(@4, $4, new ast::Implicit(@4)),
                            $6, ast_scope(@$,$8));
    }
| "while" "(" exp ")" stmt %prec CMDBLOCK
    {
      FLAVOR_CHECK(@$, "while", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_pipe);

      $$ = new ast::While(@$, $1, $3, ast_scope(@$,$5));
    }
;

in_or_colon: "in" | ":";


/*---------------.
| Control flow.  |
`---------------*/

%token DO "do";

exp:
	           block  { $$ = ast_scope(@$, 0, $1);  }
| "do" "(" exp ")" block  { $$ = ast_scope(@$, $3, $5); }
;

/*---------------------------.
| Function calls, messages.  |
`---------------------------*/

%type <ast::rLValue> lvalue;
%printer { debug_stream() << *$$; } <ast::rLValue>;
lvalue:
	  id	{ $$ = ast_call(@$, $1); }
| exp "." id	{ $$ = ast_call(@$, $1, $3); }
;

id:
  "identifier"  { std::swap($$, $1); }
;

exp:
  "var" exp %prec VAR
  {
    if (!$2.unsafe_cast<ast::LValue>()
        || ($2.unsafe_cast<ast::Call>()
            && $2.unsafe_cast<ast::Call>()->arguments_get()))
      ERROR(@2, "syntax error, " << *$2 << " is not a valid lvalue",
            ast::rExp);
    $$ = new ast::Binding(@$, $2.unchecked_cast<ast::LValue>());
  }
| lvalue
  {
    $$ = $1;
  }
| lvalue args
    {
      $$ = $1;
      $$.unchecked_cast<ast::LValueArgs>()->arguments_set($2);
      $$->location_set(@$);
    }
;

// Instantiation looks a lot like a function call.
%token <libport::Symbol> NEW "new";
%type <ast::rExp> new;
new:
  "new" "identifier" args
  {
    // Compiled as "id . new (args)".
    $$ = ast_call(@$, ast_call(@$, $2), SYMBOL(new), $3);
    up.warn(@$,
	    "deprecated construct. Instead of using 'a = new b(x)', "
	    "use 'a = b.new(x)'.");
  }
;

// Allow Object.new etc.
id:
  "new" { std::swap($$, $1); }
;

exp:
  new   { std::swap($$, $1); }
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
      if (!$2)
	error(@$, "closure cannot be lazy");
      $$ = new ast::Closure(@$, symbols_to_decs($2, @2),
                            ast_scope(@$, $3));
    }
;


/*----------.
| Numbers.  |
`----------*/

%printer { debug_stream() << $$; } <ufloat>;
%token <ufloat>
        FLOAT      "float"
        DURATION   "duration";
%type <ast::rExp> exp_float;
exp_float:
  "float"  { $$ = new ast::Float(@$, $1); }
;


/*-----------.
| Duration.  |
`-----------*/

%type <ufloat> duration;
duration:
  "duration"          { $$ = $1;      }
| duration "duration" { $$ = $1 + $2; }
;


/*-----------.
| Literals.  |
`-----------*/

exp:
  exp_float      { std::swap($$, $1);  }
| duration       { $$ = new ast::Float(@$, $1);  }
| "string"       { $$ = new ast::String(@$, $1); }
| "[" exps "]"   { $$ = new ast::List(@$, $2); }
;


/*------------------.
| Location support. |
`------------------*/

exp:
  "__HERE__"
  {
    PARAMETRIC_AST(pos, "Position.new(%exp:1, %exp:2, %exp:3)");
    PARAMETRIC_AST(no_file, "nil");
    const libport::Symbol* fn = @$.begin.filename;
    $$ = exp(pos
             % (fn ? new ast::String(@$, fn->name_get()) : ast_nil())
             % new ast::Float(@$, @$.begin.line)
             % new ast::Float(@$, @$.begin.column));
  }
;

/*---------.
| Events.  |
`---------*/

%token QUEST_MARK "?";
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
  exp "[" exps "]"
  {
    $$ = new ast::Subscript(@$, $3, $1);
  }
;


/*--------------------.
| Special variables.  |
`--------------------*/

%token  CALL         "call"
        THIS         "this"
;

exp:
  "this"         { $$ = new ast::This(@$); }
| "call"         { $$ = new ast::CallMsg(@$); }
;

/*---------------------.
| Numeric operations.  |
`---------------------*/

// The name of the operators are the name of the messages.
%token <libport::Symbol>
        BANG       "!"
        BITAND     "bitand"
        BITOR      "bitor"
        CARET      "^"
        COMPL      "compl"
        GT_GT      ">>"
        LT_LT      "<<"
        MINUS      "-"
        PERCENT    "%"
        PLUS       "+"
        SLASH      "/"
        STAR       "*"
        STAR_STAR  "**"
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
| "+" exp    %prec UNARY  { $$ = ast_call(@$, $2, $1); }
| "-" exp    %prec UNARY  { $$ = ast_call(@$, $2, $1); }
| "!" exp                 { $$ = ast_call(@$, $2, $1); }
| "compl" exp             { $$ = ast_call(@$, $2, $1); }
| "(" exp ")"             { std::swap($$, $2); }
;

/*--------.
| Tests.  |
`--------*/
%token <libport::Symbol>
        EQ_TILDA_EQ   "=~="
        EQ_EQ         "=="
        EQ_EQ_EQ      "==="
        GT_EQ         ">="
        GT            ">"
        LT_EQ         "<="
        LT            "<"
        BANG_EQ       "!="
        BANG_EQ_EQ    "!=="
        TILDA_EQ      "~="

        AMPERSAND_AMPERSAND  "&&"
        PIPE_PIPE            "||"
;

exp:
  exp "!="  exp { $$ = ast_call(@$, $1, $2, $3); }
| exp "!==" exp { $$ = ast_call(@$, $1, $2, $3); }
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

/*---------------------.
| Desugaring internals |
`---------------------*/

%type <unsigned> unsigned;
unsigned:
  "float" { $$ = static_cast<unsigned int>($1); }
;

%token PERCENT_UNSCOPE_COLON "%unscope:";
exp:
  "%unscope:" unsigned
  {
    $$ = new ast::Unscope(@$, $2);
  }
;

/*----------------.
| Metavariables.  |
`----------------*/

%token PERCENT_EXP_COLON "%exp:";
exp:
  "%exp:" unsigned
  {
    $$ = new ast::MetaExp(@$, $2);
  }
;

%token PERCENT_LVALUE_COLON "%lvalue:";
lvalue:
  "%lvalue:" unsigned
  {
    $$ = new ast::MetaLValue(@$, new ast::exps_type(),
			     $2);
  }
;

%token PERCENT_ID_COLON "%id:";
lvalue:
  "%id:" unsigned
  {
    $$ = new ast::MetaId(@$, 0, $2);
  }
| exp "." "%id:" unsigned
  {
    $$ = new ast::MetaCall(@$, 0, $1, $4);
  }
;

%token PERCENT_EXPS_COLON "%exps:";
exp:
  lvalue "(" "%exps:" unsigned ")"
  {
    assert($1.unsafe_cast<ast::LValueArgs>());
    assert(!$1.unsafe_cast<ast::LValueArgs>()->arguments_get());
    $$ = new ast::MetaArgs(@$, $1, $4);
  }
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
  "(" exps ")"  { std::swap($$, $2); }
;


/*------------.
| Soft test.  |
`------------*/

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

/*--------------.
| Documentation |
`--------------*/

%token <std::string> DOC "documentation";
%type <std::string> doc;

doc:
  /* empty */   { $$ = ""; }
| DOC       { $$ = $1; }

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
