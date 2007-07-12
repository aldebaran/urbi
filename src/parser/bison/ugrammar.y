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
#include "uvariablename.hh" // UDeriveType
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

%union
{
  ast::Exp*       expr;
  ast::exps_type* exprs;

  UVariableName::UDeriveType derive;
}

%printer { EVALUATE(*$$); } <expr>;

%code // Output in ugrammar.cc.
{
#include <string>
#include <iostream>
#include <sstream>

#include "libport/ref-pt.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "ast/all.hh"
#include "runner/runner.hh"

#include "parser/uparser.hh"
#include "ubinary.hh"
#include "ucommand.hh"
#include "uasynccommand.hh"
#include "ugroup.hh"
#include "uobj.hh"
#include "uproperty.hh"
#include "uvariablename.hh"
#include "uvariablelist.hh"

#define EVALUATE(Tree)					\
 do {							\
    std::cerr << "Command: " << Tree << std::endl;	\
    runner::Runner r;					\
    r(Tree);						\
    std::cerr << "Result: ";				\
    if (r.result())					\
      std::cerr << *r.result();				\
    else						\
      std::cerr << "<NULL>";				\
    std::cerr << std::endl;				\
 } while (0)

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

    /// Create a new Tree node composing \c Lhs and \c Rhs with \c Op.
    static
    ast::Exp*
    new_bin(const yy::parser::location_type& l, Flavorable::UNodeType op,
	    ast::Exp* lhs, ast::Exp* rhs)
    {
      ast::Exp* res = 0;
      switch (op)
      {
	case Flavorable::UAND:
	  res = new ast::AndExp (l, lhs, rhs);
	  break;
	case Flavorable::UCOMMA:
	  res = new ast::CommaExp (l, lhs, rhs);
	  break;
	case Flavorable::UPIPE:
	  res = new ast::PipeExp (l, lhs, rhs);
	  break;
	case Flavorable::USEMICOLON:
	  res = new ast::SemicolonExp (l, lhs, rhs);
	  break;
	default:
	  pabort(op);

      }
      EVALUATE(*res);
      return res;
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
    ast::Exp*
    new_exp (UParser&, const yy::parser::location_type& l,
	     libport::Symbol* s, ast::Exp* t1, ast::Exp* t2)
    {
      ast::exps_type* args = new ast::exps_type;
      args->push_back(t1);
      args->push_back(t2);
      ast::CallExp* res = new ast::CallExp(l, take(s), args);
      EVALUATE(*res);
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
  TOK_AROBASE      "@"
  TOK_ASSIGN       "="
  TOK_AT           "at"
  TOK_BIN          "bin"
  TOK_BLOCK        "block"
  TOK_CLASS        "class"
  TOK_COLON        ":"
  TOK_COPY         "copy"
  TOK_DEF          "def"
  TOK_DELGROUP     "delgroup"
  TOK_DERIV        "'"
  TOK_DERIV2       "''"
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
%type <expr>                stmts           "scheduled statements"
%type <expr>                stmt            "statement"
%type <exprs>     	    exprs           "zero or more expressions"
%type <exprs>               exprs.1         "one or more expressions"
%type <named_arguments>     raw_arguments   "list of attributes"
%type <named_arguments>     namedarguments  "list of named arguments"
%type <named_arguments>     flag            "a flag"
%type <named_arguments>     flags.0         "zero or more flags"
%type <named_arguments>     flags.1         "one or more flags"
%type <variablelist>        names           "list of names"
%type <expr>                softtest        "soft test"
%type <named_arguments>     identifiers     "list of identifiers"
%type <expr>                class_declaration "class declaration"
%type <named_arguments>     class_declaration_list "class declaration list"
%type <binary>              binary          "binary"
%type <property>            property        "property"
%type <variable>            lvalue          "lvalue"
%type <variable>            name            "slot name"


/*----------------------.
| Operator precedence.  |
`----------------------*/

 /*
   ! < ( so that !m(x) be read as !(m(x)).
 */

// FIXME: This is sick!  Puke puke puke.
%left "("
%left  "||" "&&" "!"
%left  "==" "~=" "%=" "=~=" "!=" ">" ">=" "<" "<="
%left  "-" "+"
%left  "*" "/" "%"
%left  NEG     /* Negation--unary minus */
%right "^"
%right "'n"

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
    up.commandTree = 0;
  }
| lvalue "=" binary ";"  {}
| stmts                  { EVALUATE(*$1); }
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
  /* empty */                {}
| raw_arguments raw_argument {}
;


/*--------.
| stmts.  |
`--------*/

stmts:
  stmt
| stmts "," stmts { $$ = new_bin(@$, $2, $1, $3); }
| stmts ";" stmts { $$ = new_bin(@$, $2, $1, $3); }
| stmts "|" stmts { $$ = new_bin(@$, $2, $1, $3); }
| stmts "&" stmts { $$ = new_bin(@$, $2, $1, $3); }
;


/*--------------------------.
| tagged and flagged stmt.  |
`--------------------------*/

%type <expr> tag;
tag: expr;

stmt:
  tag flags.0 ":" stmt
  {
    $$ = new ast::TagExp (@$, $1, $4);
  }
| flags.1 ":" stmt {}
;


/*--------.
| flags.  |
`--------*/

flag:
  FLAG                        {}
| FLAG_TIME "(" expr ")"      {}
| FLAG_ID "(" expr ")"        {}
| FLAG_TEST "(" softtest ")"  {}
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



/*-------.
| stmt.  |
`-------*/

stmt:
  "{" stmts "}" {}
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


/*-------.
| Stmt.  |
`-------*/

stmt:
  /* empty */ { $$ = new ast::YieldExp (@$); }
| "noop"      { $$ = new ast::YieldExp (@$); }
| expr        {}
| "echo" expr namedarguments {}
| "group" "identifier" "{" identifiers "}" {}
| "addgroup" "identifier" "{" identifiers "}" {}
| "delgroup" "identifier" "{" identifiers "}" {}
| "group" {}
| "alias" name name {}
| name "inherits" name {}
| name "disinherits" name {}
| "alias" name {}
| "unalias" name {}
| "alias" {}
| OPERATOR {}
| OPERATOR_ID tag {}
| OPERATOR_VAR name {}
| BINDER "object" name {}
| BINDER "var" name "from" name {}
| BINDER "function" "(" "integer" ")" name "from" name {}
| BINDER "event" "(" "integer" ")" name "from" name {}
| "wait" expr {}
| "emit" name {}
| "emit" name "(" exprs ")" {}
| "emit" "(" expr ")" name {}
| "emit" "(" expr ")" name "(" exprs ")" {}
| "emit" "(" ")" name {}
| "emit" "(" ")" name "(" exprs ")" {}
| "waituntil" softtest {}
| "def" {}
| "var" name {}
| "def" name {}
| "var" "{" names "}" {}
| "class" "identifier" "{" class_declaration_list "}" {}
| "class" "identifier" {}
| "event" name formal_arguments {}
| "function" name formal_arguments stmt {}
;

/*-------------------.
| Stmt: Assignment.  |
`-------------------*/
stmt:
	lvalue "=" expr namedarguments {}
| "var" lvalue "=" expr namedarguments {}
| lvalue "+=" expr {}
| lvalue "-=" expr {}
| property "=" expr {}
| lvalue "=" "new" "identifier" {}
| lvalue "=" "new" "identifier" "(" exprs ")" {}
| lvalue "--" {}
| lvalue "++" {}
;

/*---------------------.
| Stmt: Control flow.  |
`---------------------*/
stmt:
  "at" and.opt "(" softtest ")" stmt %prec CMDBLOCK {}
| "at" and.opt "(" softtest ")" stmt "onleave" stmt {}
| "every" "(" expr ")" stmt {}
| "if" "(" expr ")" stmt %prec CMDBLOCK    {}
| "if" "(" expr ")" stmt "else" stmt  {}
| "for" flavor.opt "(" stmt ";" expr ";" stmt ")" stmt %prec CMDBLOCK {}
| "foreach" flavor.opt name "in" expr "{" stmts "}"    %prec CMDBLOCK {}
| "freezeif" "(" softtest ")" stmt {}
| "loop" stmt %prec CMDBLOCK {}
| "loopn" flavor.opt "(" expr ")" stmt %prec CMDBLOCK {}
| "stopif" "(" softtest ")" stmt {}
| "timeout" "(" expr ")" stmt {}
| "return" expr.opt   { $$ = new ast::ReturnExp(@$, $2, false); }
| "whenever" "(" softtest ")" stmt %prec CMDBLOCK {}
| "whenever" "(" softtest ")" stmt "else" stmt {}
| "while" pipe.opt "(" expr ")" stmt %prec CMDBLOCK {}
;



/*-------.
| Name.  |
`-------*/

name:
  "identifier"             {}
| "$" "(" expr ")"         {}
| name "." "identifier"    {}
| name "[" expr "]"        {}
| name "::" "identifier"   {} // FIXME: Get rid of it, it's useless.
;


/*---------.
| Lvalue.  |
`---------*/

lvalue:
  name		{}
| name "'n"	{}
;

// Names as rvalues.
expr:
  rvalue        {}
;

rvalue:
  name
| "static" name	{}
| name derive   {}
| name "'e"	{}
| name "'in"	{}
| name "'out"   {}
;

%type <derive> derive;
derive:
  "'"	{ $$ = UVariableName::UDERIV;	   }
| "''"	{ $$ = UVariableName::UDERIV2;	   }
| "'d"	{ $$ = UVariableName::UTRUEDERIV;  }
| "'dd"	{ $$ = UVariableName::UTRUEDERIV2; }
;


/*-----------.
| property.  |
`-----------*/

property:
  name "->" "identifier" {}
;


/*-----------------.
| namedarguments.  |
`-----------------*/

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
| "identifier" "(" exprs ")"  { $$ = new ast::CallExp(@$, take($1), $3); }
| "%" name            {}
| "group" "identifier"    {}
;


// FIXME: import the rvalue/lvalue/name stuff.

  /*---------.
  | num expr |
  `---------*/
// The name of the operators are the name of the messages.
%token
  <symbol> TOK_BANG    "!"             
  <symbol> TOK_PERCENT "%"
  <symbol> TOK_STAR    "*"
  <symbol> TOK_PLUS    "+"
  <symbol> TOK_MINUS   "-"
  <symbol> TOK_DIV     "/"
  <symbol> TOK_EXP     "^"
;

expr:
  expr "+" expr	{ $$ = new_exp(up, @$, $2, $1, $3); }
| expr "-" expr	{ $$ = new_exp(up, @$, $2, $1, $3); }
| expr "*" expr	{ $$ = new_exp(up, @$, $2, $1, $3); }
| expr "/" expr	{ $$ = new_exp(up, @$, $2, $1, $3); }
| expr "%" expr	{ $$ = new_exp(up, @$, $2, $1, $3); }
| expr "^" expr	{ $$ = new_exp(up, @$, $2, $1, $3); }
| "-" expr     %prec NEG { $$ = new ast::NegOpExp(@$, $2); }
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
  <symbol> TOK_EQU  "=="
  <symbol> TOK_GTH  ">"
  <symbol> TOK_LEQ  "<="
  <symbol> TOK_LTH  "<"
  <symbol> TOK_PEQ  "%="
  <symbol> TOK_NEQ  "!="
  <symbol> TOK_GEQ  ">="
  <symbol> TOK_DEQ  "=~="
  <symbol> TOK_REQ  "~="
;

%token
  <symbol> TOK_LAND  "&&"
  <symbol> TOK_LOR   "||"
;

expr:
  "true"  {}
| "false" {}

| expr "=="  expr { $$ = new_exp(up, @$, $2, $1, $3); }
| expr "~="  expr { $$ = new_exp(up, @$, $2, $1, $3); }
| expr "!="  expr { $$ = new_exp(up, @$, $2, $1, $3); }
| expr ">"   expr { $$ = new_exp(up, @$, $2, $1, $3); }
| expr ">="  expr { $$ = new_exp(up, @$, $2, $1, $3); }
| expr "<"   expr { $$ = new_exp(up, @$, $2, $1, $3); }
| expr "<="  expr { $$ = new_exp(up, @$, $2, $1, $3); }
| expr "=~=" expr { /* $$ = new_exp(up, @$, ???, $1, $3); */ }
| expr "%="  expr { /* $$ = new_exp(up, @$, ???, $1, $3); */ }

| "!" expr {}

| expr "&&" expr { $$ = new_exp(up, @$, $2, $1, $3); }
| expr "||" expr { $$ = new_exp(up, @$, $2, $1, $3); }
;


/*--------------.
| Expressions.  |
`--------------*/

exprs:
  /* empty */ { $$ = new ast::exps_type; }
| exprs.1     { $$ = $1; }
;

exprs.1:
  expr             { $$ = new ast::exps_type; $$->push_back ($1); }
| exprs.1 "," expr { $$->push_back($3); }
;


/*-----------.
| softtest.  |
`-----------*/

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
  /* empty */     {}
| identifiers.1   {}
;


/*---------------------------------------------.
| class_declaration & class_declaration_list.  |
`---------------------------------------------*/

class_declaration:
  "var"      name                    {}
| "function" name formal_arguments   {}
| "event"    name formal_arguments   {}
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

/*--------.
| names.  |
`--------*/

names:
  /* empty */  {}
| name {}
| name ";" names {}
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
