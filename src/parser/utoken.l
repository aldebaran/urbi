%{
/* \file utoken.l
 *******************************************************************************

 File: utoken.l\n
 Definition of scanner used by the UParser object.
 This scanned is defined with flex, using option c++.

 This file is part of
 %URBI, version 1.0\n
 (c) Jean-Christophe Baillie, 2004.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <cstdlib>
#include <cstring>
#include <sstream>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

  // #define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include "object/object.hh"
#include "object/atom.hh"

#include "parser/uparser.hh"
#include "parser/utoken.hh"
// Shorter access to the tokens.
typedef yy::parser::token token;

/// Split yytext which matches
/// "BIN"{BLANKS}{NATURAL}({BLANKS}({ID}|{NATURAL}))*{BLANKS}";"
/// setting \a keywords to the first char of keyword, and return the
/// size.
static size_t bin_split (char* yytext, size_t yyleng, char*& keywords);

/// Create a K1Bin object.
static ast::rObject* new_k1_bin(const yy::location& loc,
				const char* keywords, const std::string& data);

// Safe conversion to a numeric value.
template <typename Out, typename In>
static Out lexical_cast (parser::UParser& up, const yy::location& loc, In t);

template <typename Out, typename In>
static Out numeric_cast (parser::UParser& up, const yy::location& loc, In t);


/*--------------------.
| Prescanner macros.  |
`--------------------*/

// Prescanner return values.
/// End of file.
static token::yytokentype pre_eof        = token::yytokentype(-1);
/// No terminator found.
static token::yytokentype pre_wants_more = token::yytokentype(0);
/// A complete sentence was found.
static token::yytokentype pre_complete   = token::yytokentype(1);

// Skip this action if in pre-parser.  Do not put this in a do-while,
// as it would capture the "continue" which is meant to yylex's
// "while(1)".
#define PRE_SKIP()				\
  if (!up)					\
  {						\
    ECHO("Skipping: {{{" << yytext << "}}}");	\
    continue;					\
  }

# define PRE_PUSH(Char)				\
  if (!up)					\
  {						\
    pre_closers.push_back(Char);		\
    continue;					\
  }

# define PRE_POP()							\
  if (!up)								\
  {									\
    if (!pre_closers.empty() && pre_closers.back() == yytext[0])	\
    {									\
      pre_closers.pop_back();						\
      continue;								\
    }									\
    else								\
    {									\
      /* Syntax error (unbalanced closers).  Return as if the sentence	\
	 was correct.  The parser will report the error itself. */	\
      ECHO("Pre: Unexpected character: " << yytext);			\
      return pre_complete;						\
    }									\
  }

# define PRE_RETURN();				\
  if (!up)					\
  {						\
    if (pre_closers.empty())			\
      return pre_complete;			\
    else					\
      continue;					\
  }

# define YY_USER_INIT							\
  yy_flex_debug = !!getenv("FLEX_DEBUG");				\
									\
  if (getenv ("UCONSOLE_MODE"))						\
  {									\
    std::cerr << "Message from the scanner: UCONSOLE_MODE enabled."	\
	      << std::endl;						\
    sc_default = SC_UCONSOLE;						\
  }									\
  else									\
    sc_default = INITIAL;						\
  BEGIN sc_default;							\
									\
  if (!up)								\
  {									\
    pre_length = 0;							\
    pre_closers.clear();						\
  }


#define YY_USER_ACTION				\
  COLUMNS(yyleng);

// Move the front cursor to the end cursor.
#define STEP()					\
  do {						\
    if (up)					\
      locp->step();				\
  } while (0)

// To avoid differences between versions of Bison, set by hand the
// initial columns when advancing lines.
#define LINES(Num)				\
  do {						\
    if (up)					\
    {						\
      locp->lines(Num);				\
      locp->end.column = 1;			\
    }						\
  } while (0)

#define COLUMNS(Num)					\
  do {							\
    if (up)						\
      /* At each match, adjust the end cursor. */	\
      locp->columns(Num);				\
    else						\
      pre_length += Num;				\
 } while (0)

// Report an error.
#define ERROR(Msg)				\
do {						\
  if (up)					\
    up->error(*locp, std::string () + Msg);	\
} while(0)

// Report an EOF error.  In that case, the prescanner must return
// non-null, since null means there is not enough data.
#define EOF_ERROR(Where)				\
  do {							\
    if (!up)						\
      return pre_eof;					\
    ERROR("unexpected end of file in " + Where);	\
  } while(0)


// Otherwise Flex returns an int instead of an enum.
#define yyterminate()				\
  return up ? token::TOK_EOF : pre_wants_more;


// valp->Field = Value.
#define SET_VAL(Field, Value)			\
  do {						\
    if (up)					\
      valp->Field = Value;			\
  } while(0)

// Extend the string value of valp.
#define GROW_VAL(Value)				\
  do {						\
    if (up)					\
      *valp->str += Value;			\
  } while(0)

// Extend the string value of valp.
#define APPEND_VAL(Arg1, Arg2)			\
  do {						\
    if (up)					\
      valp->str->append (Arg1, Arg2);		\
  } while(0)


/*----------------------------------.
| Sugar to return semantic values.  |
`----------------------------------*/

#define RETURN(Symbol)				\
  {						\
    PRE_SKIP();					\
    return token::TOK_ ## Symbol;		\
  }

#define RETURN_VALUE(Token, Field, Value)	\
  {						\
    PRE_SKIP();					\
    SET_VAL(Field, Value);			\
    RETURN(Token);				\
  }

// For ",", ";", "|", and "&" (i.e., COMMA, SEMICOLON, PIPE, and AND).
#define RETURN_FLAVOR(Symbol, Flavor)					\
  RETURN_VALUE(Symbol, flavor, ast::flavor_ ## Flavor)

/// Return an fval of type Token with Value.
#define RETURN_FVAL(Token, Value)		\
  RETURN_VALUE(Token, fval, Value)

/// Return an ival of type Token with Value.
#define RETURN_IVAL(Token, Value)		\
  RETURN_VALUE(Token, ival, Value)

// Store yytext in valp->str, and return Token.
#define RETURN_STR(Token)				\
  RETURN_VALUE(Token, str, new std::string(yytext))

// Store yytext in valp->symbol, and return Token.
#define RETURN_SYMBOL(Token)					\
  RETURN_VALUE(Token, symbol, new libport::Symbol(yytext))

// Return an operator whose string representation is kept as a Symbol.
#define RETURN_OP(Token)			\
  RETURN_SYMBOL(Token)

// UnitSize is the number of letters used for the unit.
#define RETURN_TIME_VALUE(UnitSize, Scale)				\
  {									\
    PRE_SKIP();								\
    yytext[yyleng - UnitSize] = 0;					\
    RETURN_VALUE(TIME_VALUE, fval,					\
		 boost::lexical_cast<float>(yytext) * (Scale));		\
  }

#define RETURN_FLAVORED_KEYWORD(Token)					\
  {									\
    PRE_SKIP();								\
    switch (yytext[strlen(yytext) - 1])					\
    {									\
      case '|': RETURN_FLAVOR(Token, pipe);				\
      case ',': RETURN_FLAVOR(Token, comma);				\
      case '&': RETURN_FLAVOR(Token, and);				\
      default:  RETURN_FLAVOR(Token, semicolon);			\
    }									\
  }


// We matched an obsolete token, New is now preferered.  Unput it so
// that the regular scanning rule of New is run.  The braces are
// needed to scope msg and rewritten.
#define DEPRECATED(New)							\
  {									\
    PRE_SKIP();								\
    std::string warn = std::string("`") + yytext + "' is deprecated.";	\
    std::string rewritten = New;					\
    if (!rewritten.empty())						\
      warn += " Use `" + rewritten + "' instead.";			\
    up->warn(*locp, warn);						\
    for (int i = rewritten.length() - 1; 0 <= i; --i)			\
      unput(rewritten[i]);						\
  }

%}
%option noyywrap debug

%x SC_C_COMMENT SC_STRING_OR_SYMBOL
%s SC_UCONSOLE

BLANKS   [ \t]+
EOL      \n|\r|\n\r|\r\n

  // Identifier.
ID       [a-zA-Z_][_A-Za-z0-9]*
  // Identifier suffix for magic derivation (K1 syntax).
SUFFIX   "'"(""|"'"|d|dd|e|n|in|out)


NATURAL  [0-9]+
EXPONENT [eE][-+]?{NATURAL}
FLOAT    {NATURAL}\.{NATURAL}{EXPONENT}?
ENATURAL {NATURAL}{EXPONENT}

NUMBER   {NATURAL}|{ENATURAL}|{FLOAT}

FLAVOR   [;|,&]?

%%
%{

  // The first delimiter of a string or a symbol : ' or ".
  // Used in SC_STRING_OR_SYMBOL to know if it is reading a string or a symbol.
  char delimiter = 0;

  // A stack of expected closing braces: ), ], } etc.
  std::vector<char> pre_closers;

  STEP();

%}

  /*-----------------------------------.
  | Internal rules for generated code. |
  `-----------------------------------*/
^"#"{BLANKS}?("line"|"push"){BLANKS}{NATURAL}({BLANKS}"\"".*"\"")?{BLANKS}?{EOL}	{
  PRE_SKIP();
# define SKIP_SPACES()				\
  for (++yytext; isspace (*yytext); ++yytext)	\
    continue;

  SKIP_SPACES();
  bool push = yytext[0] == 'p';
  yytext += 4; // line|push.

  SKIP_SPACES();
# undef SKIP_SPACES
  char* line = yytext;
  yytext += strspn(yytext, "0123456789");
  *yytext = 0;
  ++yytext;

  char* file = strchr (yytext, '"');
  if (file)
  {
    ++file;
    *strchr (file, '"') = 0;
  }

  if (push)
    up->synclines_.push(*locp);

  locp->initialize (!file || STREQ(file, "-") ? 0 : new libport::Symbol (file));
  locp->lines (lexical_cast<int>(*up, *locp, line) - 1);
  STEP();
}

^"#"{BLANKS}?"pop"{BLANKS}?{EOL}     {
  PRE_SKIP();
  *locp = up->synclines_.top();
  up->synclines_.pop();
}

  /*--------------------------------------------------------------.
  | Console mode: skip the [01234567] lines for expected output.  |
  `--------------------------------------------------------------*/

<SC_UCONSOLE>{
  ^\[[0-9X]{8,}.*\]\ .* /* ignore the "[timestamp:tags] whatever" messages */;
  ^\[:].*               /* ignore the "[:] whatever" messages */;
}


  /*------------.
  | The rules.  |
  `------------*/

  /*  FIXME: This is really sick. */
"+report"     RETURN_IVAL(FLAG, 1);
"+error"      RETURN_IVAL(FLAG, 2);
"+end"        RETURN_IVAL(FLAG, 3);
"+begin"      RETURN_IVAL(FLAG, 4);
"+top"        RETURN_IVAL(FLAG, 7);
"+bg"         RETURN_IVAL(FLAG, 10);
"+freeze"     RETURN_IVAL(FLAG, 11);
"+stop"       RETURN_IVAL(FLAG, 12);

{NUMBER}"d"   RETURN_TIME_VALUE(1, 1000*60*60*24);
{NUMBER}"h"   RETURN_TIME_VALUE(1, 1000*60*60);
{NUMBER}"m"   RETURN_TIME_VALUE(1, 1000*60);
{NUMBER}"s"   RETURN_TIME_VALUE(1, 1000);
{NUMBER}"ms"  RETURN_TIME_VALUE(2, 1);

{NATURAL}     RETURN_IVAL(INTEGER, lexical_cast<int>  (*up, *locp, yytext));
{FLOAT}       RETURN_FVAL(FLOAT,   lexical_cast<float>(*up, *locp, yytext));
{ENATURAL}    {
		PRE_SKIP();
		float res = lexical_cast<float>(*up, *locp, yytext);
		// Do not put semicolons here: RETURN_?VAL expands into {}.
		if (res < 1)
		  RETURN_FVAL(FLOAT, res)
		else
		  RETURN_IVAL(INTEGER, numeric_cast<int>(*up, *locp, res))
	      }

[\"\']        {
		SET_VAL(str, new std::string);
		delimiter = *yytext;
		BEGIN SC_STRING_OR_SYMBOL;
	      }

 /*
  "block"      |
  "disconnect" |
  "freeze"     |
  "killall"    |
  "stop"       |
  "unblock"    |
  "unfreeze"   RETURN_SYMBOL (OPERATOR_ID);


  "info"    |
  "undef"   RETURN_SYMBOL (OPERATOR_VAR);
 */

 /*
   FIXME: Surprisingly enough, these are strings, not symbols,
   i.e., they pretend to have quotes around them.  This is because
   they are used as properties, which are "generalized" as strings.
   It is some sort of lexical sugar that allows to write << add >>,
   instead of << "add" >>.  Needless to say that it conflicts badly
   with the rest of the language.

   "add"     |
   "cancel"  |
   "discard" |
   "mix"     |
   "normal"  |
   "queue"   RETURN_STR (STRING);
 */

"internal"  DEPRECATED("");
"external"  RETURN_SYMBOL (BINDER);

"_call"         RETURN(_CALL);
"_exp"          RETURN(_EXP);

"addgroup"      RETURN(ADDGROUP);
"alias"         RETURN(ALIAS);
"break"         RETURN(BREAK);
"class"         RETURN(CLASS);
"def"           RETURN(DEF);
"delete"        RETURN(DELETE);
"delgroup"      RETURN(DELGROUP);
"do"		RETURN(DO);
"else"          RETURN(ELSE);
"emit"          RETURN(EMIT);
"event"         RETURN(EVENT);
"every"         RETURN(EVERY);
"foreach"       DEPRECATED("for");
"freezeif"      RETURN(FREEZEIF);
"from"          RETURN(FROM);
"function"      RETURN(FUNCTION);
"group"         RETURN(GROUP);
"if"            RETURN(IF);
"in"            RETURN(IN);
"loopn"         DEPRECATED("for");
"new"           RETURN_OP(NEW);
"object"        RETURN(OBJECT);
"onleave"       RETURN(ONLEAVE);
"return"        RETURN(RETURN);
"static"        DEPRECATED("");
"stopif"        RETURN(STOPIF);
"timeout"       RETURN(TIMEOUT);
"unalias"       RETURN(UNALIAS);
"var"           RETURN(VAR);
"whenever"      RETURN(WHENEVER);

"for"{FLAVOR}   RETURN_FLAVORED_KEYWORD(FOR);
"loop"{FLAVOR}  RETURN_FLAVORED_KEYWORD(LOOP);
"while"{FLAVOR} RETURN_FLAVORED_KEYWORD(WHILE);
"at"{FLAVOR}    RETURN_FLAVORED_KEYWORD(AT);

"!"             RETURN_OP(BANG);
"%"             RETURN_OP(PERCENT);
"*"             RETURN_OP(STAR);
"*="            RETURN(STAR_EQ);
"+"             RETURN_OP(PLUS);
"+="            RETURN(PLUS_EQ);
"++"            RETURN(PLUS_PLUS);
"-"             RETURN_OP(MINUS);
"-="            RETURN(MINUS_EQ);
"--"            RETURN(MINUS_MINUS);
"/"             RETURN_OP(SLASH);
"/="            RETURN(SLASH_EQ);

"<"             RETURN_OP(LT);
"<="            RETURN_OP(LT_EQ);
"<<"            RETURN_OP(LT_LT);
"=="            RETURN_OP(EQ_EQ);
"==="           RETURN_OP(EQ_EQ_EQ);
"!="            RETURN_OP(BANG_EQ);
"!=="           RETURN_OP(BANG_EQ_EQ);
"=~="           RETURN_OP(EQ_TILDA_EQ);
"~="            RETURN_OP(TILDA_EQ);
"%="            RETURN_OP(PERCENT_EQ);
">"             RETURN_OP(GT);
">="            RETURN_OP(GT_EQ);
">>"            RETURN_OP(GT_GT);
"^"             RETURN_OP(CARET);
"**"            RETURN_OP(STAR_STAR);
"&&"            RETURN_OP(AMPERSAND_AMPERSAND);
"||"            RETURN_OP(PIPE_PIPE);

"="             RETURN(EQ);
"$"             RETURN(DOLLAR);

"->"            RETURN(MINUS_GT);
"."             RETURN(POINT);
":"             RETURN(COLON);
"::"            RETURN(COLON_COLON);
"~"             RETURN(TILDA);

"(|"            PRE_PUSH('|'); RETURN(LPAREN_PIPE);
"|)"            PRE_POP();     RETURN(PIPE_RPAREN);
"("             PRE_PUSH(')'); RETURN(LPAREN);
")"             PRE_POP();     RETURN(RPAREN);
"["             PRE_PUSH(']'); RETURN(LBRACKET);
"]"             PRE_POP();     RETURN(RBRACKET);
"{"             PRE_PUSH('}'); RETURN(LBRACE);
"}"             PRE_POP();     RETURN(RBRACE);


"&"             RETURN_FLAVOR(AMPERSAND, and);
"|"             RETURN_FLAVOR(PIPE, pipe);
";"             PRE_RETURN(); RETURN_FLAVOR(SEMICOLON, semicolon);
","             PRE_RETURN(); RETURN_FLAVOR(COMMA, comma);


  /*----------------------.
  | The infamous k1-BIN.  |
  `----------------------*/

  /* It is used only as a sort of here-doc,
     and it is just as unusable, yet necessary.

     The only supported syntax was "var foo = BIN (keywords)+ SIZE; DATA"
     with DATA being SIZE long.  Note where the ";" is...
   */
"BIN"{BLANKS}{NATURAL}({BLANKS}({ID}|{NATURAL}))*{BLANKS}?";"      {
  char* keywords;
  size_t size = bin_split (yytext, yyleng, keywords);
  // Eat the data.
  std::string data;
  for (size_t i = 0; i < size; ++i)
    switch (int c = yyinput())
    {
      case EOF:
	EOF_ERROR("BIN");
	goto out_of_loop;
      default:
	// Keep track of the locations.
	COLUMNS(1);
	data += c;
	break;
    }
  out_of_loop:
  // Use yytext before modifying it (unput).
  SET_VAL(expr, new_k1_bin (*locp, keywords, data));
  unput(';');
  if (!up)
    --pre_length;
  RETURN(K1BIN);
}

{ID}{SUFFIX}    { // Treat "n'" as "n.'\''".
  PRE_SKIP();
  // Push back "n.'\''" in the stream.
  char* quote = strchr(yytext, '\'');
  std::string s (yytext, quote - yytext);
  s += ".'\\'";
  s += quote[1] == '\'' ? "\\'" : quote + 1;
  s += '\'';
  DEPRECATED(s);
}

{ID}            RETURN_VALUE(IDENTIFIER, symbol, new libport::Symbol(yytext));


    /*--------------------.
    | Strings and symbols |
    `--------------------*/

<SC_STRING_OR_SYMBOL>{ /* Handling of the strings and symbols. */
     [\"\'] {
       if (*yytext == delimiter)
       {
	 BEGIN sc_default;
	 if (*yytext == '\'')
	   RETURN_VALUE(IDENTIFIER, symbol, new libport::Symbol(*valp->str))
	 else
	   RETURN(STRING);
       }
       else
	 GROW_VAL(*yytext);
     }

     \\[0-7]{3}		{
       long c = strtol (yytext + 1, 0, 8);
       if (c > 255)
	 ERROR("invalid escape: `" + yytext + '\'');
       else
	 GROW_VAL(c);
     }

     \\x[0-9a-fA-F]{2}	 GROW_VAL(strtol (yytext + 2, 0, 16));

     \\a	GROW_VAL('\a');
     \\b	GROW_VAL('\b');
     \\f	GROW_VAL('\f');
     \\n	GROW_VAL('\n');
     \\r	GROW_VAL('\r');
     \\t	GROW_VAL('\t');
     \\v	GROW_VAL('\v');
     \\[\\\"\'] GROW_VAL(yytext[1]);
     \\.	ERROR("invalid escape: `" + yytext + '\'');

     [^\"\'\n\r\\]+      APPEND_VAL(yytext, yyleng);

     {EOL}+    LINES(yyleng); APPEND_VAL(yyleng, '\n');

     <<EOF>> {
       EOF_ERROR(delimiter + "..." + delimiter);
       unput(delimiter);
     }
}


    /*----------------------------.
    | White spaces and comments.  |
    `----------------------------*/

("//"|"#").*   STEP(); // One-line comments.
"/*"           BEGIN SC_C_COMMENT;

{BLANKS}       STEP();
{EOL}          LINES(1); STEP();

<SC_C_COMMENT>{ /* Comments. */
  "*/" { /* End of the comment. */
    STEP();
    BEGIN sc_default;
  }

  [^*/\n\r]+	|
  [*/]		continue;
  {EOL}	        LINES(1);

  <<EOF>> {
    EOF_ERROR("a comment");
    STEP();
    BEGIN sc_default;
  }
}

<*>{
   .      {
	     ERROR("invalid character: " + yytext);
	     STEP();
	  }
}

%%

// Safe conversion to a numeric value.
template <typename Out, typename In>
Out
lexical_cast (parser::UParser& up, const yy::location& loc, In t)
{
  try
  {
    return boost::lexical_cast<Out>(t);
  }
  catch (boost::bad_lexical_cast)
  {
    up.error(loc,
	     (boost::format ("invalid numerical litteral: %1%") % t).str());
    return 0;
  }
}

template <typename Out, typename In>
Out
numeric_cast (parser::UParser& up, const yy::location& loc, In t)
{
  try
  {
    return boost::numeric_cast<Out>(t);
  }
  catch (boost::bad_numeric_cast)
  {
    up.error(loc,
	     (boost::format ("invalid numerical litteral: %1%") % t).str());
    return 0;
  }
}


/// Split yytext which matches
/// "BIN"({BLANKS}{ID})*{NATURAL}{BLANKS}";" setting \a keywords to
/// the first char of keyword, and return the size.
static
size_t
bin_split (char* yytext, size_t yyleng, char*& keywords)
{
  // Smash the semicolon.
  yytext[yyleng - 1] = 0;
  // Skip BIN.
  yytext += 3;
# define SKIP_SPACES()				\
  for (++yytext; isspace (*yytext); ++yytext)	\
    continue;
  SKIP_SPACES();

  char* size = yytext;
  while (isdigit(yytext[0]))
    ++yytext;
  *yytext = 0;
  size_t res = boost::lexical_cast<size_t>(size);

  SKIP_SPACES();
  keywords = yytext;
  return res;
}

static
ast::rObject*
new_k1_bin(const yy::location& loc,
	   const char* keywords, const std::string& data)
{
  object::rObject o = object::Object::fresh();
  o->proto_add(object::global_class->slot_get(SYMBOL(Binary)));
  o->slot_set(SYMBOL(keywords),
	      object::String::fresh(libport::Symbol(keywords)));
  o->slot_set(SYMBOL(data),
	      object::String::fresh(libport::Symbol(data)));
  return new ast::rObject (loc, o);
}

namespace parser
{
  size_t
  prescan (char* buf)
  {
    yyFlexLexer scanner;
    std::istringstream is (buf);
    scanner.switch_streams(&is, 0);

    // In prescanner mode, return 1 if complete, -1 on EOF, 0 if incomplete.
    int c = scanner.yylex();
    passert(c, c == pre_eof || c == pre_wants_more || c == pre_complete);
    // The number of read bytes.
    size_t length = scanner.pre_length;
    ECHO("res: " << c << ", length: " << length);
    if (c == pre_complete)
      return length;
    else
      // We met EOF before reaching a terminator.
      return 0;
  }
}

// Local Variables:
// mode: C++
// End:
