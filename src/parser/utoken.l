/*
 * Copyright (C) 2005-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
%{
  /// \file parser/utoken.l
  /// \brief The scanner.

#include <libport/debug.hh>

#include <libport/cstdio>
#include <libport/cstdlib>
#include <libport/cstring>
#include <sstream>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <libport/escape.hh>
#include <libport/format.hh>
#include <libport/ufloat.hh>

#include <parser/parser-impl.hh>
#include <parser/utoken.hh>

GD_CATEGORY(Urbi.Scanner);

// Shorter access to the tokens.
typedef yy::parser::token token;

// We need this type everywhere.
using libport::ufloat;

/// Split yytext which matches
/// "BIN"{BLANKS}{NATURAL}{BLANKS}[^;]*";"
/// setting \a keywords to the first char of keyword, and return the
/// size.
static size_t bin_split(char* yytext, size_t yyleng, char*& keywords);

// Safe conversion to a numeric value.
template <typename Out, typename In>
static Out lexical_cast(parser::ParserImpl* up, const yy::location& loc, In t);

// Skip the spaces in yytext.
# define SKIP_SPACES()                          \
  while (isspace(*yytext))                      \
    ++yytext;

# define YY_INPUT(Buf, Result, MaxSize)                 \
  do                                                    \
  {                                                     \
    if (yyin->eof() || yyin->fail())                    \
    {                                                   \
      Result = 0;                                       \
      break;                                            \
    }                                                   \
                                                        \
    GD_FPUSH_TRACE("LexerInput (max size = %s)",        \
                   MaxSize);                            \
    yyin->readsome(Buf, MaxSize);                       \
                                                        \
    if (!yyin->gcount() && !yyin->eof())                \
    {                                                   \
      yyin->peek();                                     \
      yyin->readsome(Buf, MaxSize);                     \
    }                                                   \
                                                        \
    if (yyin->bad())                                    \
    {                                                   \
      GD_INFO_DEBUG("BAD");                             \
      Result = -1;                                      \
      break;                                            \
    }                                                   \
                                                        \
    GD_FINFO_DEBUG("%s bytes", yyin->gcount());         \
    Result = yyin->gcount();                            \
  } while (0)

# define YY_USER_INIT                                                   \
  static bool flex_debug = getenv("URBI_SCANNER");                      \
  yy_flex_debug = flex_debug;                                           \
                                                                        \
  static bool urbi_check_mode = getenv ("URBI_CHECK_MODE");             \
                                                                        \
  /* The default start condition.  */                                   \
  sc_default =                                                          \
    parser_impl_->meta() ? SC_META                                      \
    : urbi_check_mode ? SC_URBI_CHECK                                   \
    : INITIAL;                                                          \
  BEGIN sc_default;                                                     \
                                                                        \
  static bool init = false;                                             \
  if (!init)                                                            \
  {                                                                     \
    init = true;                                                        \
    if (urbi_check_mode)                                                \
      GD_INFO("URBI_CHECK_MODE enabled.");                              \
  }

// Stupidly enough the Flex documentation recommends redefining
// LexerError, which does not seem to be possible (since it is flex
// which generates unconditionally its definition in utoken.cc).
#define YY_FATAL_ERROR(Msg)                     \
  parser_impl_->error(loc, Msg)

// Each time a rule is matched.
#define YY_USER_ACTION                          \
  COLUMNS(static_cast<unsigned>(yyleng));

// Move the front cursor to the end cursor.
#define STEP()                                  \
    loc.step()

// Called via YY_USER_ACION each time we *advance* the cursor, in
// which case Num is actually the number of bytes read.
#define COLUMNS(Num)                                        \
  loc.columns(Num)

// To avoid differences between versions of Bison, set by hand the
// initial columns when advancing lines.  Does not touch pre_length,
// since LINES is always called after COLUMNS was, which did update
// pre_length.
#define LINES(Num)                                  \
  do {                                              \
    loc.lines(Num);                                 \
    loc.end.column = 1;                             \
  } while (false)

// Report an error.
#define SCAN_ERROR(...)                                     \
  parser_impl_->error(loc, libport::format(__VA_ARGS__))

// Report an EOF error.
#define EOF_ERROR(Where)                                \
  SCAN_ERROR("unexpected end of file in %s", Where)

// Report the use of a reserved word.
#define RESERVED_ERROR                          \
  SCAN_ERROR("reserved symbol: `%s'", yytext)

// Otherwise Flex returns an int instead of an enum.
#define yyterminate()                                   \
  return yy::parser::make_EOF(loc)


// Extend the string value of valp.
#define GROW_VAL(Value)                         \
  grown_string += Value

// Extend the string value of valp.
#define APPEND_VAL(Arg1, Arg2)                  \
  grown_string.append(Arg1, Arg2)


/*----------------------------------.
| Sugar to return semantic values.  |
`----------------------------------*/

#define RETURN(Symbol)                          \
  return yy::parser::make_ ## Symbol(loc)

#define RETURN_VALUE(Symbol, Value)              \
  return yy::parser::make_ ## Symbol(Value, loc)

// UnitLength is the number of letters used for the unit.
#define RETURN_UNIT(Token, UnitLength, From, To)                        \
  do {                                                                  \
    yytext[yyleng - UnitLength] = 0;                                    \
    RETURN_VALUE(Token,                                                 \
                 lexical_cast<ufloat>(parser_impl_, loc, yytext)        \
                 / (From) * (To));                                      \
  } while (false)


#define RETURN_ANGLE(UnitLength, From, To)      \
  RETURN_UNIT(ANGLE, UnitLength, From, To)

#define RETURN_DURATION(UnitLength, Scale)      \
  RETURN_UNIT(DURATION, UnitLength, 1, Scale)


// For ",", ";", "|", and "&" (i.e., COMMA, SEMICOLON, PIPE, and AND).
#define RETURN_FLAVOR(Symbol, Flavor)           \
  RETURN_VALUE(Symbol, ast::flavor_ ## Flavor)

#define RETURN_FLAVORED_KEYWORD(Token)                  \
  switch (yytext[yyleng - 1])                           \
  {                                                     \
  case '|': RETURN_FLAVOR(Token, pipe);                 \
  case ',': RETURN_FLAVOR(Token, comma);                \
  case '&': RETURN_FLAVOR(Token, and);                  \
  case ';': RETURN_FLAVOR(Token, semicolon);            \
  default:  RETURN_FLAVOR(Token, none);                 \
  }

// Return an operator whose string representation is kept as a Symbol.
// Contrary to RETURN_SYMBOL, the symbol is resolved statically by
// precompiled-symbols.
#define RETURN_OP(Symbol)                       \
  RETURN_VALUE(Symbol, SYMBOL_(Symbol))

// Return a meta-variable type.  Only if desugaring is enabled.
#define RETURN_PERCENT(Token)                   \
  do {                                          \
    aver(parser_impl_->meta());                 \
    RETURN(PERCENT_ ## Token ## _COLON);        \
  } while (false)

// Return a std::string of kind Token with yytext as value.
#define RETURN_STR(Token)                       \
  RETURN_VALUE(Token, yytext)

// Return a Symbol of kind Token with yytext as Value.
#define RETURN_SYMBOL(Token)                    \
  RETURN_VALUE(Token, libport::Symbol(yytext))


/// Unput a large string.
//
// As this will be rescanned, length will be added.  It must
// not, otherwise we introduce an offset in the locations, so subtract
// it first.
#define UNPUT(Str)                                      \
  do {                                                  \
    std::string _rewritten = Str;                       \
    for (int i = _rewritten.length() - 1; 0 <= i; --i)  \
      unput(_rewritten[i]);                             \
    COLUMNS(-_rewritten.length());                      \
  } while (false)

// We matched an obsolete token, New is now preferered.  Unput it so
// that the regular scanning rule of New is run.  The braces are
// needed to scope msg and rewritten.
#define DEPRECATED(New)                                   \
  do {                                                    \
    std::string rewritten = New;                          \
    parser_impl_->deprecated(loc, yytext, rewritten);     \
    UNPUT(rewritten);                                     \
  } while (false)

%}
/* No default rule. */
%option nodefault
/* Allow faster implements. */
%option noreject noyymore
/* We need an interactive scanner.  Otherwise the generate scanner
   will always fetch one character of look-ahead, even when not
   needed.  This results in examples such as `lobby.received("1;");'
   not working, because we are waiting for the character after the
   (inner) `;' to process that `;'.  */
%option interactive
/* Even in -Cf mode, support 8 bit characters. */
%option 8bit

%x SC_C_COMMENT SC_STRING_OR_SYMBOL
 // Ignore lines starting with "[000000:tag]", we are fed with a chk file.
%s SC_URBI_CHECK
 // Enable meta variable support.
%s SC_META

BLANKS   [ \t]+
EOL      \n|\r|\n\r|\r\n

  // Identifier.
ID       [a-zA-Z_][_A-Za-z0-9]*
  // Identifier suffix for magic derivation (K1 syntax).
SUFFIX   "'"(""|"'"|d|dd|e|n|in|out)


NATURAL  [0-9]+
HEXA     0x[0-9a-fA-F]+
EXPONENT [eE][-+]?{NATURAL}
FLOAT    {NATURAL}\.{NATURAL}{EXPONENT}?|{NATURAL}{EXPONENT}

NUMBER   {NATURAL}|{FLOAT}

FLAVOR   [;|,&]?

SHEBANG  ^"#!"[^\n]*\n

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

  // Nesting C comments.
  unsigned int comment_level = 0;

  // Calling yylex for a new token, reset the cursor.
  STEP();
%}

  /*-----------------------------------.
  | Internal rules for generated code. |
  `-----------------------------------*/

"//#"{BLANKS}?("line"|"push"){BLANKS}{NATURAL}({BLANKS}"\"".*"\"")?{BLANKS}?{EOL}      {

  /* synclines are mainly made to get correct locations when loading a
     file: save the current location before loading the file, then
     restore it afterwards.

     This is simple to implement using cpp like synclines: "//#line",
     "//#push", and "//#pop".  The semantics of the first one is
     simple: it contains the location of the *next* line.  The
     semantics of //#push and //#pop is that the outer location is the
     same as if everything was removed from the push to the pop (both
     included).

     But we have to display errors about //#pop.  What's its location?
     It is chosen to the location of current location.  That's how the
     LINES and STEP etc. were placed as they are in the
     synclines-rules below.  */

  yytext += strlen("//#");
  SKIP_SPACES();

  // line|push.
  if (yytext[0] == 'p')
  {
    // When pushing, store the location that *precedes* this request,
    // that of the starting cursor.
    loc.end = loc.begin;
    parser_impl_->synclines_.push(loc);
  }
  yytext += 4;

  SKIP_SPACES();
  char* line = yytext;
  yytext += strspn(yytext, "0123456789");
  *yytext = 0;
  ++yytext;

  char* file = strchr(yytext, '"');
  if (file)
  {
    ++file;
    // FIXME: We do not support " in file names, nor quotes.
    *strchr(file, '"') = 0;
  }

  loc.initialize(!file || libport::streq(file, "-")
		 ? 0
		 : new libport::Symbol(file));
  loc.lines(lexical_cast<unsigned>(parser_impl_, loc, line) - 1);
  loc.step();
}

"//#"{BLANKS}?"pop"{BLANKS}?{EOL}     {
  if (parser_impl_->synclines_.empty())
  {
    SCAN_ERROR("too many //#pop");
    LINES(1);
    STEP();
  }
  else
  {
    loc = parser_impl_->synclines_.top();
    parser_impl_->synclines_.pop();
  }
}

  /*-----------------------------------------------------------------.
  | Urbi check mode: skip the [01234567] lines for expected output.  |
  `-----------------------------------------------------------------*/

<SC_URBI_CHECK>{
  ^\[[0-9X]{8,}.*\]\ .* /* ignore the "[timestamp:tags] whatever" messages */;
  ^"[:]".*               /* ignore the "[:] whatever" messages */;
}


  /*------------.
  | The rules.  |
  `------------*/

  /* Durations. */
{NUMBER}"d"   RETURN_DURATION(1, 60*60*24);
{NUMBER}"h"   RETURN_DURATION(1, 60*60);
{NUMBER}"m"   DEPRECATED(std::string(yytext) + "in");
{NUMBER}"mn"  RETURN_DURATION(2, 60);
{NUMBER}"min" RETURN_DURATION(3, 60);
{NUMBER}"s"   RETURN_DURATION(1, 1);
{NUMBER}"ms"  RETURN_DURATION(2, 0.001);

  /* Angles. */
{NUMBER}"rad"   RETURN_ANGLE(3,   1,    1);
{NUMBER}"deg"   RETURN_ANGLE(3, 180, M_PI);
{NUMBER}"grad"  RETURN_ANGLE(4, 200, M_PI);

{HEXA}        RETURN_VALUE(FLOAT, strtoll(yytext + 2, 0, 16));
{NATURAL}     |
{FLOAT}       RETURN_VALUE(FLOAT, lexical_cast<ufloat>(parser_impl_, loc, yytext));

({NUMBER}|{HEXA}){ID} SCAN_ERROR("invalid token: `%s'", yytext);

[\"\']        {
                grown_string.clear();
                delimiter = *yytext;
                BEGIN SC_STRING_OR_SYMBOL;
              }

{SHEBANG}     LINES(1);

"external"  RETURN(EXTERNAL);

<SC_META>{
  "%exp:"         RETURN_PERCENT(EXP);
  "%exps:"        RETURN_PERCENT(EXPS);
  "%id:"          RETURN_PERCENT(ID);
  "%lvalue:"      RETURN_PERCENT(LVALUE);
  "%unscope:"     RETURN_PERCENT(UNSCOPE);
}

"assert"        RETURN(ASSERT);
"break"         RETURN(BREAK);
"call"          RETURN(CALL);
"case"          RETURN(CASE);
"class"         RETURN(CLASS);
"closure"       RETURN(CLOSURE);
"const"         RETURN(CONST);
"continue"      RETURN(CONTINUE);
"default"       RETURN(DEFAULT);
"detach"        RETURN(DETACH);
"disown"        RETURN(DISOWN);
"do"            RETURN(DO);
"else"          RETURN(ELSE);
"emit"          RETURN(EMIT);
"enum"          RETURN(ENUM);
"finally"       RETURN(FINALLY);
"freezeif"      RETURN(FREEZEIF);
"function"      RETURN(FUNCTION);
"if"            RETURN(IF);
"in"            RETURN(IN);
"isdef"         RETURN(ISDEF);
  /*<no-space< New.*/
"new"           RETURN_SYMBOL(NEW);
  /*>no-space>*/
"onleave"       RETURN(ONLEAVE);
"return"        RETURN(RETURN);
"stopif"        RETURN(STOPIF);
"switch"        RETURN(SWITCH);
"this"          RETURN(THIS);
"timeout"       RETURN(TIMEOUT);
"var"           RETURN(VAR);
"waituntil"     RETURN(WAITUNTIL);
"watch"         RETURN(WATCH);
"whenever"      RETURN(WHENEVER);

"every"{FLAVOR} RETURN_FLAVORED_KEYWORD(EVERY);
"for"{FLAVOR}   RETURN_FLAVORED_KEYWORD(FOR);
"loop"{FLAVOR}  RETURN_FLAVORED_KEYWORD(LOOP);
"while"{FLAVOR} RETURN_FLAVORED_KEYWORD(WHILE);
"at"{FLAVOR}    RETURN_FLAVORED_KEYWORD(AT);

"!"|"not"       RETURN_OP(BANG);
"%"             RETURN_OP(PERCENT);
"*"             RETURN_OP(STAR);
"+"             RETURN_OP(PLUS);
"++"            RETURN(PLUS_PLUS);
"-"             RETURN_OP(MINUS);
"--"            RETURN(MINUS_MINUS);
"/"             RETURN_OP(SLASH);

"%="            RETURN_OP(PERCENT_EQ);
"*="            RETURN_OP(STAR_EQ);
"+="            RETURN_OP(PLUS_EQ);
"-="            RETURN_OP(MINUS_EQ);
"/="            RETURN_OP(SLASH_EQ);
"^="|"xor_eq"   RETURN_OP(CARET_EQ);

"<"             RETURN_OP(LT);
"<="            RETURN_OP(LT_EQ);
"<<"            RETURN_OP(LT_LT);
"=="            RETURN_OP(EQ_EQ);
"=>"            RETURN(EQ_GT);
"==="           RETURN_OP(EQ_EQ_EQ);
"!="|"not_eq"   RETURN_OP(BANG_EQ);
"!=="           RETURN_OP(BANG_EQ_EQ);
"=~="           RETURN_OP(EQ_TILDA_EQ);
"~="            RETURN_OP(TILDA_EQ);
">"             RETURN_OP(GT);
">="            RETURN_OP(GT_EQ);
">>"            RETURN_OP(GT_GT);
"^"|"xor"       RETURN_OP(CARET);
"**"            RETURN_OP(STAR_STAR);
"&&"|"and"      RETURN_OP(AMPERSAND_AMPERSAND);
"||"|"or"       RETURN_OP(PIPE_PIPE);

"?"             RETURN(QUEST_MARK);

"="             RETURN(EQ);

"->"            RETURN(MINUS_GT);
"."             RETURN(POINT);
":"             RETURN(COLON);
"~"             RETURN(TILDA);

"("             RETURN(LPAREN);
")"             RETURN(RPAREN);
"["             RETURN(LBRACKET);
"]"             RETURN(RBRACKET);
"{"             RETURN(LBRACE);
"}"             RETURN(RBRACE);

"&"             RETURN_FLAVOR(AMPERSAND, and);
"|"             RETURN_FLAVOR(PIPE, pipe);
";"             RETURN_FLAVOR(SEMICOLON, semicolon);
","             RETURN_FLAVOR(COMMA, comma);

"bitand"        RETURN_SYMBOL(BITAND);
"bitor"         RETURN_SYMBOL(BITOR);
"compl"         RETURN_SYMBOL(COMPL);

"private"       RETURN(PRIVATE);
"protected"     RETURN(PROTECTED);
"public"        RETURN(PUBLIC);

"catch"         RETURN(CATCH);
"throw"         RETURN(THROW);
"try"           RETURN(TRY);

"__HERE__"      RETURN(__HERE__);

  /*<no-space< Reserved keywords.*/
"&="               |
"and_eq"           |
"asm"              |
"auto"             |
"bool"             |
"char"             |
"const_cast"       |
"delete"           |
"double"           |
"dynamic_cast"     |
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
"or_eq"            |
"register"         |
"reinterpret_cast" |
"short"            |
"signed"           |
"sizeof"           |
"static_cast"      |
"struct"           |
"template"         |
"typedef"          |
"typeid"           |
"typename"         |
"union"            |
"unsigned"         |
"using"            |
"virtual"          |
"volatile"         |
"wchar_t"          |
"when"             |
"|="               RESERVED_ERROR;
  /*>no-space>*/

  /*----------------------.
  | The infamous k1-BIN.  |
  `----------------------*/

  /* It is used only as a sort of here-doc,
     and it is just as unusable, yet necessary.

     The only supported syntax was "var foo = BIN SIZE (keywords)+; DATA"
     with DATA being SIZE long.  Note where the ";" is...
   */
"BIN"{BLANKS}{NATURAL}[^;]*";"      {
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
  UNPUT(libport::format("Global.Binary.new(\"%s\", \"%s\");",
                        keywords, libport::escape(data, '"')));
}


  /*<no-space< Deprecated constructs. */
  /*------------------------.
  | Deprecated constructs.  |
  `------------------------*/

"foreach"     DEPRECATED("for");
"internal"    DEPRECATED("");
"loopn"       DEPRECATED("for");
"self"        DEPRECATED("this");
"static"      DEPRECATED("");

{ID}{SUFFIX}    { // Treat "n'" as "n.'\''".
  // Push back "n.'\''" in the stream.
  char* quote = strchr(yytext, '\'');
  std::string s (yytext, quote - yytext);
  s += "_quote_";
  s += quote[1] == '\'' ? "quote" : quote + 1;
  DEPRECATED(s);
}
  /*>no-space>*/


  /* operator XXX -> 'XXX'. */
{OPERATOR}    {
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
           RETURN_VALUE(IDENTIFIER, libport::Symbol(grown_string));
         else
           RETURN_VALUE(STRING, grown_string);
       }
       else
         GROW_VAL(*yytext);
     }

     \\[0-7]{1,3}         {
       long c = strtol (yytext + 1, 0, 8);
       if (c < 256)
         GROW_VAL(c);
       else
         SCAN_ERROR("invalid number after \\-escape: `%s'", yytext + 1);
     }

     \\x[0-9a-fA-F]{2}   GROW_VAL(strtol (yytext + 2, 0, 16));

     \\a        GROW_VAL('\a');
     \\b        GROW_VAL('\b');
     \\f        GROW_VAL('\f');
     \\n        GROW_VAL('\n');
     \\r        GROW_VAL('\r');
     \\t        GROW_VAL('\t');
     \\v        GROW_VAL('\v');
     \\[\\\"\'] GROW_VAL(yytext[1]);
     \\.        {
                  SCAN_ERROR("invalid character after \\-escape: `%s'",
                             libport::escape(yytext + 1));
     }

     "\\B("[0-9]+")("    {
       // Binary escape.
       size_t len = strtol(yytext+3, 0, 0);
       // In fast mode, a Flex scanner pass its internal buffer as
       // yytext, but it smashes yytext[yyleng] to '\0', so restore it
       // here: we are going to read it ourselves.
       *yy_c_buf_p = yy_hold_char;

       // How much is available in the buffer.
       size_t remain = (yy_n_chars
                        - (yy_c_buf_p - YY_CURRENT_BUFFER_LVALUE->yy_ch_buf));
       // How much we are going to eat.
       size_t eat = std::min(remain, len);

       // If we want more that available, go and fetch in the stream
       // itself.  Don't use yyinput, it is just not fast enough.
       if (remain < len)
       {
         // Add current buffer
         grown_string += std::string(yy_c_buf_p, remain);

         // Read remaining data
         size_t current_size = grown_string.length();
         grown_string.resize(current_size + len - eat);
         yyin->read(const_cast<char*>(grown_string.c_str()) + current_size,
                    len-eat);

       }
       else
       {
         // All we have to read is in Flex' buffer.
         aver(parser_impl_);
         grown_string += std::string(yy_c_buf_p, eat);
       }
       COLUMNS(len);

       // We played nasty tricks with Flex, tweaking its buffer by
       // hand.  Restore its state: first of all advance the cursor of
       // "eat" characters, then set the "hold" character to the next
       // character it will read (when coming back from an action,
       // flex reverses the effect of defining yytext, which starts by
       // yytext[yyleng] = 0, after having saved it in yy_hold_char).
       yy_cp = yy_c_buf_p += eat;
       yy_hold_char = *yy_cp;

       switch (int c = yyinput())
       {
       case EOF:
         EOF_ERROR("\\B");
         break;
       case ')':
         COLUMNS(1);
         break;
       default:
         SCAN_ERROR("unexpected '%s' for closing \\B, expected ')'",
                    libport::escape((char)c));
         break;
       }
     }

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
"/*"           ++comment_level; BEGIN SC_C_COMMENT;

{BLANKS}       STEP();
{EOL}          LINES(1); STEP();

<SC_C_COMMENT>{ /* Comments. */
  <<EOF>> {
    EOF_ERROR("a comment");
    STEP();
    BEGIN sc_default;
  }
}

<SC_C_COMMENT>{ /* Comments. */
  [^*/\n\r]+    |
  [*/]          continue;
  {EOL}         LINES(1);
  "/*"          ++comment_level;
  "*/"          {
                  if (!--comment_level)
                    {
                      STEP();
                      BEGIN sc_default;
                    }
                }
}

<*>{
   .      {
             SCAN_ERROR("invalid character: `%s'", libport::escape(yytext));
             STEP();
          }
}

%%

// Safe conversion to a numeric value.
// The name parser_impl_ is chosen so that SCAN_ERROR can be used
// from out of the scanner.
template <typename Out, typename In>
Out
lexical_cast (parser::ParserImpl* parser_impl_, const yy::location& loc, In t)
{
  try
  {
    return boost::lexical_cast<Out>(t);
  }
  catch (const boost::bad_lexical_cast&)
  {
    SCAN_ERROR("invalid numerical literal: %1%", t);
    return 0;
  }
}

/// Split yytext which matches
/// "BIN"{BLANKS}{NATURAL}[^;]*";" setting \a keywords to
/// the first char of keyword, and return the size.
static
size_t
bin_split (char* yytext, size_t yyleng, char*& keywords)
{
  // Smash the semicolon.
  yytext[--yyleng] = 0;
  // Remove all the trailing spaces.
  while (isspace(yytext[yyleng - 1]))
    yytext[--yyleng] = '\0';

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
