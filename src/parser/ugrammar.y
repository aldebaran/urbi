/// \file parser/ugrammar.y
/// \brief Definition of the parser used by the ParserImpl object.

%require "2.3"
%language "C++"
%defines
// Instead of "yytoken yylex(yylval, yylloc)", use "symbol_type yylex()".
%define lex_symbol

// Prefix all our external definition of token names with "TOK_".
%define api.tokens.prefix "TOK_"

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
#include <parser/event-match.hh>

  // Typedef shorthands
  // It is inconvenient to use the pointer notation with the variants.
  typedef ast::exps_type* exps_pointer;
  typedef ast::symbols_type* symbols_pointer;
  typedef std::pair<libport::Symbol, ast::rExp> modifier_type;
  typedef modifier_type formal_type;
  typedef std::vector<formal_type> formals_type;

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
#include <boost/format.hpp>
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
  using parser::ast_if;
  using parser::ast_nil;
  using parser::ast_scope;
  using parser::ast_string;
  using parser::ast_strip;
  using parser::ast_switch;
  using parser::ast_whenever;
  using parser::ast_whenever_event;

#include <parser/parse.hh>
#include <parser/parser-impl.hh>
#include <parser/utoken.hh>

  namespace
  {

    static void
    check_modifiers_accumulation(ast::loc loc,
                                 const ast::modifiers_type& modifiers,
                                 libport::Symbol name,
                                 parser::ParserImpl& up)
    {
      if (libport::mhas(modifiers, name))
        up.warn(loc,
                str(boost::format("accumulated modifier: %s.") % name));
    }

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

# undef ERROR
# define ERROR(Loc, Msg, Type)                  \
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
    symbols_to_decs(formals_type* formals,
                    const ast::loc& loc)
  {
    if (!formals)
      return 0;
    ast::local_declarations_type* res = new ast::local_declarations_type();
    foreach (const formal_type& var, *formals)
      res->push_back(new ast::LocalDeclaration(loc, var.first, var.second));
    delete formals;
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
        CONST        "const"
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
%type <ast::rExp> block exp exp.opt softtest stmt stmt_loop;


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
%left  CMDBLOCK
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
%right "!" "compl" "++" "--" UNARY     /* Negation--unary minus */
%right "**"
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

%type <ast::rNary> root stmts;
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

%type <ast::rExp> tag;
tag:
  exp { std::swap($$, $1); }
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
  "{" stmts "}"       { $$ = ast_strip($2); }
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
      (a, "'external'.'function'(%exp:1, %exp:2, %exp:3, %exp:4)");

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
      (a, "'external'.'event'(%exp:1, %exp:2, %exp:3, %exp:4)");

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
  exp "!" args.opt tilda.opt
  {
    $$ = new ast::Emit(@$, $1, $3, $4);
  }
//<no-space< Emit.
| "emit" k1_id args.opt tilda.opt
  {
    up.warn(@$,
            "`emit myEvent(Args...)' is deprecated.  "
            "Use `myEvent!(Args...)' instead.");
    $$ = new ast::Emit(@$, $2, $3, $4);
  }
//>no-space>
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
      $$ = new ast::Declaration(@$, $2,
                                new ast::Function(@$, symbols_to_decs($3, @3),
                                                  ast_scope (@4, $4)));
    }
| "closure" k1_id formals block
    {
      if (!$3)
	error(@$, "closure cannot be lazy");
      // Compiled as "var name = closure args stmt"
      $$ = new ast::Declaration(@$, $2,
                                new ast::Closure(@$, symbols_to_decs($3, @3),
                                                 ast_scope (@4, $4)));
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
%type <ast::rDictionary> dictionary;

modifier:
  "identifier" ":" exp
  {
    $$.first = $1;
    $$.second = $3;
  }
;

dictionary:
  dictionary modifier
  {
    check_modifiers_accumulation(@2, $1->value_get(), $2.first, up);
    std::swap($$, $1);
    $$->value_get()[$2.first] = $2.second;
  }
| modifier
  {
    $$ = new ast::Dictionary(@$, 0, ast::modifiers_type());
    $$->value_get()[$1.first] = $1.second;
  }

/*-------------------.
| Stmt: Assignment.  |
`-------------------*/

exp:
  exp "=" exp
    {
      ast::rDictionary d = $3.unsafe_cast<ast::Dictionary>();
      if (d && d->base_get())
        $$ = new ast::Assign(@$, $1, d->base_get(),
                             new ast::modifiers_type(d->value_get()));
      else
        $$ = new ast::Assign(@$, $1, $3, 0);
    }
| "(" dictionary ")"
    {
      $$ = $2;
    }
| exp modifier
    {
      if (ast::rDictionary d = $1.unsafe_cast<ast::Dictionary>())
      {
        check_modifiers_accumulation(@2, d->value_get(), $2.first, up);
        d->value_get()[$2.first] = $2.second;
        $$ = $1;
      }
      else if (ast::rAssign a = $1.unsafe_cast<ast::Assign>())
      {
        ast::modifiers_type* m = a->modifiers_get();
        if (m)
          check_modifiers_accumulation(@2, *a->modifiers_get(), $2.first, up);
        else
        {
          m = new ast::modifiers_type();
          a->modifiers_set(m);
        }
        (*m)[$2.first] = $2.second;
        $$ = $1;
      }
      else
      {
        ast::rDictionary d = new ast::Dictionary(@$, 0, ast::modifiers_type());
        d->value_get()[$2.first] = $2.second;
        d->base_get() = $1;
        $$ = d;
      }
    }

    /*
  | exp modifier
    {
      ast::rDictionary d = $1.unsafe_cast<ast::Dictionary>();
      if (!d)
      {
        d = new ast::Dictionary(@$, ast::modifiers_type(), $1);
      }
      d->value_get()[$2.first] = $2.second;
      $$ = d;

    }*/

%token <libport::Symbol>
        CARET_EQ    "^="
        MINUS_EQ    "-="
        PERCENT_EQ  "%="
        PLUS_EQ     "+="
        SLASH_EQ    "/="
        STAR_EQ     "*="
;

exp:
  lvalue "+=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, $2); }
| lvalue "-=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, $2); }
| lvalue "*=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, $2); }
| lvalue "/=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, $2); }
| lvalue "^=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, $2); }
| lvalue "%=" exp    { $$ = new ast::OpAssignment(@2, $1, $3, $2); }
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
| "at" "(" exp tilda.opt ")" nstmt onleave.opt
    {
      FLAVOR_CHECK(@$, "at", $1,
		   $1 == ast::flavor_semicolon || $1 == ast::flavor_and);
      $$ = ast_at(@$, $3, $6, $7, $4);
    }
| "at" "(" event_match ")" nstmt onleave.opt
    {
      $$ = ast_at_event(@$, $3, $5, $6);
    }
| "every" "(" exp ")" nstmt
    {
      PARAMETRIC_AST(every, "Control.every_(%exp:1, %exp:2)");
      $$ = exp (every % $3 % $5);
    }
| "if" "(" stmts ")" nstmt else.opt
    {
      $$ = ast_if(@$, $3, $5, $6);
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
| "switch" "(" exp ")" "{" cases default.opt "}"
    {
      $$ = ast_switch(@3, $3, $6, $7);
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
| "waituntil" "(" exp tilda.opt ")"
    {
      $$ = ::parser::ast_waituntil(@$, $3, $4);
    }
| "waituntil" "(" event_match ")"
    {
      $$ = ::parser::ast_waituntil_event(@$, $3.event, $3.pattern);
    }
;


/*----------------------------------------.
| Optional default/else/onleave clauses.  |
`----------------------------------------*/

// CMDBLOCK < "else" and "onleave" to promote shift in else.opt and
// onleave.opt.

%type <ast::rNary> default.opt;
default.opt:
  /* nothing. */ %prec CMDBLOCK   { $$ = 0;            }
|  "default" ":" stmts            { std::swap($$, $3); }
;

%type <ast::rExp> else.opt;
else.opt:
  /* nothing. */ %prec CMDBLOCK   { $$ = 0;            }
| "else" nstmt                    { std::swap($$, $2); }
;

// An optional onleave clause.
%type <ast::rExp> onleave.opt;
onleave.opt:
  /* nothing. */ %prec CMDBLOCK   { $$ = 0;            }
| "onleave" nstmt                 { std::swap($$, $2); }
;


stmt:
  "whenever" "(" exp tilda.opt ")" nstmt else.opt
    {
      $$ = ast_whenever(@$, $3, $6, $7, $4);
    }
| "whenever" "(" event_match ")" nstmt else.opt
    {
      $$ = ast_whenever_event(@$, $3, $5, $6);
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
| "throw" exp.opt
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
        "     0 < '$for';" // Use 0 < n, not n > 0, since < is faster.
        "     '$for' -= 1)"
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
    ast::rBinding res = new ast::Binding(@$, $2.unchecked_cast<ast::LValue>());
    $$ = res;
  }
| "const" "var" exp %prec VAR
  {
    if (!$3.unsafe_cast<ast::LValue>()
        || ($3.unsafe_cast<ast::Call>()
            && $3.unsafe_cast<ast::Call>()->arguments_get()))
      ERROR(@2, "syntax error, " << *$3 << " is not a valid lvalue",
            ast::rExp);
    ast::rBinding res = new ast::Binding(@$, $3.unchecked_cast<ast::LValue>());
    res->constant_set(true);
    $$ = res;
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

//<no-space< new "identifier".
// Instantiation looks a lot like a function call.
%token <libport::Symbol> NEW "new";
%type <ast::rExp> new;
new:
  "new" "identifier" args.opt
  {
    // Compiled as "id . new (args)".
    $$ = ast_call(@$, ast_call(@$, $2), SYMBOL(new), $3);
    up.warn(@$,
	    "deprecated construct. Instead of using 'a = new b(x)', "
	    "use 'a = b.new(x)'.");
  }
;

exp:
  new   { std::swap($$, $1); }
;

// Allow Object.new etc.
id:
  "new" { std::swap($$, $1); }
;
//>no-space>


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
%type <::parser::EventMatch> event_match;
event_match:
  exp "?" args.opt guard.opt
  {
    $$ = ::parser::EventMatch($1, $3, $4);
  }
| "?" exp guard.opt
  {
    up.warn(@$,
            "`?myEvent(Args...)' is deprecated.  "
            "Use `myEvent?(Args...)' instead.");
    ast::rCall call = $2.unsafe_cast<ast::Call>();
    if (call && call->arguments_get())
    {
      ast::exps_type* args = new ast::exps_type(*call->arguments_get());
      call->arguments_set(0);
      assert(args);
      $$ = ::parser::EventMatch(call, args, $3);
    }
    else
      $$ = ::parser::EventMatch($2, 0, $3);
  }
;

%type <ast::rExp> guard.opt;
guard.opt:
  /* nothing */  { $$ = 0; }
| "if" exp       { std::swap($$, $2); }
;

%type<ast::rExp> tilda.opt;
tilda.opt:
  /* nothing */ { $$ = 0; }
| "~" exp       { std::swap($$, $2); }
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

%type <exps_pointer> exps exps.1 args args.opt;

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

args.opt:
  /* empty */  { $$ = 0; }
| args         { std::swap($$, $1); }
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

%type <formal_type> formal_argument;
formal_argument:
  var.opt "identifier"
  {
    $$ = formal_type($2, 0);
  }
| var.opt "identifier" "=" exp
  {
    $$ = formal_type($2, $4);
  }
;

// One or several comma-separated identifiers.
%printer { debug_stream() << libport::deref << $$; } <symbols_pointer>;
%type <formals_type*> formal_arguments formal_arguments.1 formals;
formal_arguments.1:
  formal_argument
  {
    $$ = new formals_type;
    $$->push_back($1);
  }
| formal_arguments.1 "," formal_argument
  {
    std::swap($$, $1);
    $$->push_back($3);
  }
;

// Zero or several comma-separated identifiers.
formal_arguments:
  /* empty */            { $$ = new formals_type; }
| formal_arguments.1   { std::swap($$, $1); }
;

// Function formal arguments.
formals:
  /* empty */                { $$ = 0; }
| "(" formal_arguments ")" { std::swap($$, $2); }
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
