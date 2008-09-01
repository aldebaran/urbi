%{
  /// \file parser/utoken.l
  /// \brief The scanner.

  // Unforunately the file libport/compiler.hh is included before, and
  // it is useless to define ENABLE_DEBUG_TRACES here as the guards
  // avoid the redefinition.  Use LIBPORT_ECHO instead of ECHO.
  // #define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <cstdlib>
#include <libport/cstring>
#include <sstream>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <libport/escape.hh>

#include <parser/prescan.hh>
#include <parser/parser-impl.hh>
#include <parser/utoken.hh>

// Shorter access to the tokens.
typedef yy::parser::token token;

/// Split yytext which matches
/// "BIN"{BLANKS}{NATURAL}({BLANKS}({ID}|{NATURAL}))*{BLANKS}";"
/// setting \a keywords to the first char of keyword, and return the
/// size.
static size_t bin_split(char* yytext, size_t yyleng, char*& keywords);

// Safe conversion to a numeric value.
template <typename Out, typename In>
static Out lexical_cast(parser::ParserImpl& up, const yy::location& loc, In t);

template <typename Out, typename In>
static Out numeric_cast(parser::ParserImpl& up, const yy::location& loc, In t);


/*--------------------.
| Prescanner macros.  |
`--------------------*/

// Skip the spaces in yytext.
# define SKIP_SPACES()				\
  while (isspace(*yytext))                      \
    ++yytext;

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
      return yy::parser::make_symbol<token::TOK_PRE_COMPLETE>(loc);     \
    }									\
  }

# define PRE_RETURN();                                                  \
  if (!up)                                                              \
  {                                                                     \
    if (pre_closers.empty())                                            \
      return yy::parser::make_symbol<token::TOK_PRE_COMPLETE>(loc);     \
    else                                                                \
      continue;                                                         \
  }

# define YY_USER_INIT							\
  static bool flex_debug = !!getenv("FLEX_DEBUG");                      \
  yy_flex_debug = flex_debug;                                           \
									\
  static bool uconsole_mode = getenv ("UCONSOLE_MODE");                 \
  if (uconsole_mode)                                                    \
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
      loc.step();				\
  } while (0)

// To avoid differences between versions of Bison, set by hand the
// initial columns when advancing lines.
#define LINES(Num)				\
  do {						\
    if (up)					\
    {						\
      loc.lines(Num);				\
      loc.end.column = 1;			\
    }						\
  } while (0)

#define COLUMNS(Num)					\
  do {							\
    if (up)						\
      /* At each match, adjust the end cursor. */	\
      loc.columns(Num);                                 \
    else						\
      pre_length += Num;				\
 } while (0)

// Report an error.
#define ERROR(Msg)				\
do {						\
  if (up)					\
    up->error(loc, std::string () + Msg);	\
} while(0)

// Report an EOF error.  In that case, the prescanner must return
// non-null, since null means there is not enough data.
#define EOF_ERROR(Where)                                                \
  do {                                                                  \
    if (!up)                                                            \
      return yy::parser::make_symbol<token::TOK_PRE_EOF>(loc);          \
    ERROR("unexpected end of file in " + Where);                        \
  } while(0)

// Report the use of a reserved word.
#define RESERVED_ERROR				\
  ERROR("reserved symbol `" + yytext + "'")

// Otherwise Flex returns an int instead of an enum.
#define yyterminate()                                                   \
  return (up                                                            \
          ? yy::parser::make_symbol<token::TOK_EOF>(loc)                \
          : yy::parser::make_symbol<token::TOK_PRE_WANTS_MORE>(loc));


// Extend the string value of valp.
#define GROW_VAL(Value)				\
  do {						\
    if (up)					\
      grown_string += Value;                    \
  } while(0)

// Extend the string value of valp.
#define APPEND_VAL(Arg1, Arg2)                  \
  do {                                          \
    if (up)                                     \
      grown_string.append(Arg1, Arg2);          \
  } while(0)


/*----------------------------------.
| Sugar to return semantic values.  |
`----------------------------------*/

#define RETURN(Symbol)                                          \
  {                                                             \
    PRE_SKIP();                                                 \
    return yy::parser::make_symbol<token::TOK_ ## Symbol>(loc); \
  }


#define RETURN_VALUE(Token, Value)                                      \
  {                                                                     \
    PRE_SKIP();                                                         \
    return yy::parser::make_symbol<token::TOK_ ## Token> (Value, loc);  \
  }

// For ",", ";", "|", and "&" (i.e., COMMA, SEMICOLON, PIPE, and AND).
#define RETURN_FLAVOR(Symbol, Flavor)					\
  RETURN_VALUE(Symbol, ast::flavor_ ## Flavor)

/// Return a float of kind Token with Value.
#define RETURN_FVAL(Token, Value)		\
  RETURN_VALUE(Token, Value)

/// Return an int of kind Token with Value.
#define RETURN_IVAL(Token, Value)		\
  RETURN_VALUE(Token, Value)

// Return a std::string of kind Token with yytext as value.
#define RETURN_STR(Token)                       \
  RETURN_VALUE(Token, yytext)

// Return a Symbol of kind Token with Value.
#define RETURN_SOME_SYMBOL(Token, Value)                        \
  RETURN_VALUE(Token, libport::Symbol(Value))

// Return a Symbol of kind Token with yytext as Value.
#define RETURN_SYMBOL(Token)                    \
  RETURN_SOME_SYMBOL(Token, yytext)

// Return an operator whose string representation is kept as a Symbol.
#define RETURN_OP(Token)			\
  RETURN_SYMBOL(Token)

// Return an assignment operator: its value is the base operator.
#define RETURN_ASSIGN_OP(Token)                 \
  PRE_SKIP();                                   \
  yytext[1] = 0;                                \
  RETURN_OP(Token);

// UnitLength is the number of letters used for the unit.
#define RETURN_DURATION(UnitLength, Scale)				\
  {									\
    PRE_SKIP();								\
    yytext[yyleng - UnitLength] = 0;					\
    RETURN_VALUE(DURATION, boost::lexical_cast<float>(yytext) * (Scale)); \
  }

#define RETURN_FLAVORED_KEYWORD(Token)                  \
  {                                                     \
    PRE_SKIP();                                         \
    switch (yytext[yyleng - 1])                         \
    {                                                   \
      case '|': RETURN_FLAVOR(Token, pipe);             \
      case ',': RETURN_FLAVOR(Token, comma);            \
      case '&': RETURN_FLAVOR(Token, and);              \
      default:  RETURN_FLAVOR(Token, semicolon);        \
    }                                                   \
  }


/// Unput a large string.
//
// Do not forget to remove from the pre-length the length of Str when
// in prescanner mode, otherwise the prescanner will count too many
// characters: those of the original string, and those of the unput
// string.
#define UNPUT(Str)					\
  do {							\
    std::string _rewritten = Str;			\
    for (int i = _rewritten.length() - 1; 0 <= i; --i)	\
      unput(_rewritten[i]);				\
    if (!up)						\
      pre_length -= _rewritten.length();		\
  } while (0)

// We matched an alternative token, rescan with Str instead.
#define SYNONYM(Str)                            \
  {                                             \
    PRE_SKIP();                                 \
    UNPUT(Str);                                 \
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
    up->warn(loc, warn);						\
    UNPUT(rewritten);							\
  }

%}
%option noyywrap debug

%x SC_C_COMMENT SC_DOC_COMMENT SC_STRING_OR_SYMBOL
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

  /* See [over.oper] in the C++ standard. */
OPERATORS   "new"|"delete"|"new[]"|"delete[]"|"+"|"-"|"*"|"/"|"%"|"^"|"&"|"|"|"~"|"!"|"="|"<"|">"|"+="|"-="|"*="|"/="|"%="|"^="|"&="|"|="|"<<"|">>"|">>="|"<<="|"=="|"!="|"<="|">="|"&&"|"||"|"++"|"--"|","|"->*"|"->"|"()"|"[]"
OPERATOR "operator"{BLANKS}?{OPERATORS}

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
^"//#"{BLANKS}?("line"|"push"){BLANKS}{NATURAL}({BLANKS}"\"".*"\"")?{BLANKS}?{EOL}	{
  PRE_SKIP();
  yytext += strlen("//#");
  SKIP_SPACES();
  bool push = yytext[0] == 'p';
  yytext += 4; // line|push.

  ++yytext;
  SKIP_SPACES();
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
    up->synclines_.push(loc);

  loc.initialize(!file || STREQ(file, "-") ? 0 : new libport::Symbol(file));
  loc.lines (lexical_cast<int>(*up, loc, line) - 1);
  STEP();
}

^"//#"{BLANKS}?"pop"{BLANKS}?{EOL}     {
  PRE_SKIP();
  loc = up->synclines_.top();
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

{NUMBER}"d"   RETURN_DURATION(1, 1000*60*60*24);
{NUMBER}"h"   RETURN_DURATION(1, 1000*60*60);
{NUMBER}"m"   RETURN_DURATION(1, 1000*60);
{NUMBER}"s"   RETURN_DURATION(1, 1000);
{NUMBER}"ms"  RETURN_DURATION(2, 1);

{NATURAL}     RETURN_IVAL(INTEGER, lexical_cast<int>  (*up, loc, yytext));
{FLOAT}       RETURN_FVAL(FLOAT,   lexical_cast<float>(*up, loc, yytext));
{ENATURAL}    {
		PRE_SKIP();
		float res = lexical_cast<float>(*up, loc, yytext);
		// Do not put semicolons here: RETURN_?VAL expands into {}.
		if (res < 1)
		  RETURN_FVAL(FLOAT, res)
		else
		  RETURN_IVAL(INTEGER, numeric_cast<int>(*up, loc, res))
	      }

[\"\']        {
                grown_string.clear();
		delimiter = *yytext;
		BEGIN SC_STRING_OR_SYMBOL;
	      }

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
"external"  RETURN(EXTERNAL);

 /* Meta variables. */
"_call"         RETURN(_CALL);
"_exp"          RETURN(_EXP);
"_exps"         RETURN(_EXPS);
"_formals"      RETURN(_FORMALS);

"%exp:"		RETURN(PERCENT_EXP_COLON);
"%exps:"	RETURN(PERCENT_EXPS_COLON);
"%id:"          RETURN(PERCENT_ID_COLON);
"%lvalue:"	RETURN(PERCENT_LVALUE_COLON);

"break"         RETURN(BREAK);
"call"          RETURN(CALL);
"case"          RETURN(CASE);
"class"         RETURN(CLASS);
"closure"       RETURN(CLOSURE);
"continue"      RETURN(CONTINUE);
"delete"        RETURN(DELETE);
"do"		RETURN(DO);
"else"          RETURN(ELSE);
"emit"          RETURN(EMIT);
"event"         RETURN(EVENT);
"every"         RETURN(EVERY);
"foreach"       DEPRECATED("for");
"freezeif"      RETURN(FREEZEIF);
"from"          RETURN(FROM);
"function"      RETURN(FUNCTION);
"if"            RETURN(IF);
"in"            RETURN(IN);
"loopn"         DEPRECATED("for");
"new"           RETURN_OP(NEW);
"object"        RETURN(OBJECT);
"onleave"       RETURN(ONLEAVE);
"return"        RETURN(RETURN);
"self"          DEPRECATED("this");
"static"        DEPRECATED("");
"stopif"        RETURN(STOPIF);
"switch"        RETURN(SWITCH);
"this"          RETURN(THIS);
"timeout"       RETURN(TIMEOUT);
"var"           RETURN(VAR);
"waituntil"     RETURN(WAITUNTIL);
"whenever"      RETURN(WHENEVER);

"for"{FLAVOR}   RETURN_FLAVORED_KEYWORD(FOR);
"loop"{FLAVOR}  RETURN_FLAVORED_KEYWORD(LOOP);
"while"{FLAVOR} RETURN_FLAVORED_KEYWORD(WHILE);
"at"{FLAVOR}    RETURN_FLAVORED_KEYWORD(AT);

"!"             RETURN_OP(BANG);
"%"             RETURN_OP(PERCENT);
"*"             RETURN_OP(STAR);
"+"             RETURN_OP(PLUS);
"++"            RETURN(PLUS_PLUS);
"-"             RETURN_OP(MINUS);
"--"            RETURN(MINUS_MINUS);
"/"             RETURN_OP(SLASH);

"*="            RETURN_ASSIGN_OP(STAR_EQ);
"+="            RETURN_ASSIGN_OP(PLUS_EQ);
"-="            RETURN_ASSIGN_OP(MINUS_EQ);
"/="            RETURN_ASSIGN_OP(SLASH_EQ);
"^="            RETURN_ASSIGN_OP(CARET_EQ);

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

"?"             RETURN(QUEST_MARK);

"="             RETURN(EQ);

"->"            RETURN(MINUS_GT);
"."             RETURN(POINT);
":"             RETURN(COLON);
"~"             RETURN(TILDA);

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

"and"           SYNONYM("&&");
"and_eq"        SYNONYM("&=");
"bitand"        RETURN_OP(BITAND);
"bitor"         RETURN_OP(BITOR);
"compl"         RETURN_OP(COMPL);
"not"           SYNONYM("!");
"not_eq"        SYNONYM("!=");
"or"            SYNONYM("||");
"or_eq"         SYNONYM("|=");
"xor"           SYNONYM("^");
"xor_eq"        SYNONYM("^=");

"private"       RETURN(PRIVATE);
"protected"     RETURN(PROTECTED);
"public"        RETURN(PUBLIC);

"asm"              |
"auto"             |
"bool"             |
"catch"            |
"char"             |
"const"            |
"const_cast"       |
"default"          |
"double"           |
"dynamic_cast"     |
"enum"             |
"explicit"         |
"export"           |
"extern"           |
"float"            |
"friend"           |
"goto"             |
"inline"           |
"int"              |
"long"             |
"mutable"          |
"namespace"        |
"register"         |
"reinterpret_cast" |
"short"            |
"signed"           |
"sizeof"           |
"static_cast"      |
"struct"           |
"template"         |
"throw"            |
"try"              |
"typedef"          |
"typeid"           |
"typename"         |
"union"            |
"unsigned"         |
"using"            |
"virtual"          |
"volatile"         |
"wchar_t"          RESERVED_ERROR;

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
  std::ostringstream rewritten;
  rewritten << "Global.Binary.new (\"" << keywords << "\", "
	    <<                   " \"" << libport::escape(data) << "\");";
  UNPUT(rewritten.str());
}

{ID}{SUFFIX}    { // Treat "n'" as "n.'\''".
  PRE_SKIP();
  // Push back "n.'\''" in the stream.
  char* quote = strchr(yytext, '\'');
  std::string s (yytext, quote - yytext);
  s += "_quote_";
  s += quote[1] == '\'' ? "quote" : quote + 1;
  DEPRECATED(s);
}

  /* operator XXX -> 'XXX'. */
{OPERATOR}    {
                PRE_SKIP();
                yytext += strlen("operator");
                SKIP_SPACES();
                RETURN_SYMBOL(IDENTIFIER);
              }

{ID}            RETURN_SYMBOL(IDENTIFIER);


    /*--------------------.
    | Strings and symbols |
    `--------------------*/

<SC_STRING_OR_SYMBOL>{ /* Handling of the strings and symbols. */
     [\"\'] {
       if (*yytext == delimiter)
       {
	 BEGIN sc_default;
	 if (*yytext == '\'')
	 {
	   PRE_SKIP();
	   RETURN_SOME_SYMBOL(IDENTIFIER, grown_string);
	 }
	 else
	   RETURN_VALUE(STRING, grown_string);
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

"//".*         STEP(); // One-line comment.
"/*"           BEGIN SC_C_COMMENT;
"/*!"          grown_string.clear(); BEGIN SC_DOC_COMMENT;

{BLANKS}       STEP();
{EOL}          LINES(1); STEP();

<SC_C_COMMENT,SC_DOC_COMMENT>{ /* Comments. */
  <<EOF>> {
    EOF_ERROR("a comment");
    STEP();
    BEGIN sc_default;
  }
}

<SC_C_COMMENT>{ /* Comments. */
  [^*/\n\r]+	|
  [*/]		continue;
  {EOL}	        LINES(1);
  "*/" { /* End of the comment. */
    STEP();
    BEGIN sc_default;
  }
}

<SC_DOC_COMMENT>{
  [^*/\n\r]+	|
  [*/]          GROW_VAL(yytext); continue;
  {EOL}	        GROW_VAL(yytext); LINES(1);
  "*/" { /* End of the comment. */
    STEP();
    BEGIN sc_default;
    RETURN_VALUE(DOC, grown_string);
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
lexical_cast (parser::ParserImpl& up, const yy::location& loc, In t)
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
numeric_cast (parser::ParserImpl& up, const yy::location& loc, In t)
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
  ++yytext;
  SKIP_SPACES();

  char* size = yytext;
  while (isdigit(yytext[0]))
    ++yytext;
  *yytext = 0;
  size_t res = boost::lexical_cast<size_t>(size);
  ++yytext;
  SKIP_SPACES();
  keywords = yytext;
  return res;
}

// Local Variables:
// mode: C++
// End:
