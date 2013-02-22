/* A Bison parser, made by GNU Bison 2.3b.656-d4b2-dirty.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++

   Copyright (C) 2002-2011 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* First part of user declarations.  */

/* Line 369 of lalr1.cc  */
#line 38 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"

#include "ugrammar.hh"

/* User implementation prologue.  */

/* Line 374 of lalr1.cc  */
#line 45 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
/* Unqualified %code blocks.  */
/* Line 375 of lalr1.cc  */
#line 62 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"

#include <string>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <libport/cassert>
#include <libport/finally.hh>
#include <libport/format.hh>
#include <libport/separate.hh>

#include <ast/all.hh>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <ast/print.hh>

#include <parser/parse.hh>
#include <parser/parser-impl.hh>
#include <parser/utoken.hh>

#if defined __GNUC__ && __GNUC__ == 4 && __GNUC_MINOR__ == 4
# pragma GCC diagnostic ignored "-Wuninitialized"
#endif

#define MAKE(Kind, ...)                         \
  up.factory().make_ ## Kind(__VA_ARGS__)

  namespace
  {

    static void
    modifiers_add(parser::ParserImpl& up, const ast::loc& loc,
                  ast::modifiers_type& mods,
                  const ::ast::Factory::modifier_type& mod)
    {
      if (libport::has(mods, mod.first))
        up.warn(loc,
                libport::format("modifier redefined: %s", mod.first));
      mods[mod.first] = mod.second;
    }

    static void
    assocs_add(parser::ParserImpl& /*up*/, const ast::loc& /*loc*/,
               ast::dictionary_elts_type& mods,
               const ast::dictionary_elt_type& mod)
    {
      // FIXME: check for duplicate literal keys?
      // if (libport::has(mods, mod.first))
      //   up.warn(loc,
      //           libport::format("key redefined: %s", mod.first));
      mods.push_back(mod);
    }

    /// Use the scanner in the right parser::ParserImpl.
    static
    inline
    yy::parser::symbol_type
    yylex(parser::ParserImpl& up)
    {
      boost::optional< ::yy::parser::token_type>&
        initial_token(up.initial_token_get());
      if (initial_token)
      {
        ::yy::parser::token_type res = initial_token.get();
        initial_token = boost::optional< ::yy::parser::token_type>();
        return yy::parser::symbol_type(res, yy::location());
      }
      return up.scanner_.yylex();
    }

  } // anon namespace

# define REQUIRE_IDENTIFIER(Loc, Effective, Expected)                   \
  do {                                                                  \
    if (Effective != libport::Symbol(Expected))                         \
      up.error(Loc,                                                     \
               libport::format("unexpected `%s', expecting `%s'",       \
                               Effective, Expected));                   \
  } while (false)



/* Line 375 of lalr1.cc  */
#line 133 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                               \
 do                                                                    \
   if (N)                                                              \
     {                                                                 \
       (Current).begin = YYRHSLOC (Rhs, 1).begin;                      \
       (Current).end   = YYRHSLOC (Rhs, N).end;                        \
     }                                                                 \
   else                                                                \
     {                                                                 \
       (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;        \
     }                                                                 \
 while (false)
#endif

/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << std::endl;                  \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)		\
  do {					\
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);		\
  } while (false)

# define YY_STACK_PRINT()		\
  do {					\
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  static_cast<void>(0)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif /* !YYDEBUG */

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyempty = true)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {
/* Line 458 of lalr1.cc  */
#line 218 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              /* Fall through.  */
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  parser::parser (::parser::ParserImpl& up_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      up (up_yyarg)
  {
  }

  parser::~parser ()
  {
  }


  /*---------------.
  | Symbol types.  |
  `---------------*/



  // stack_symbol_type.
  parser::stack_symbol_type::stack_symbol_type ()
    : super_type ()
    , state ()
  {
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, const semantic_type& v, const location_type& l)
    : super_type (v, l)
    , state (s)
  {
  }

  int
  parser::stack_symbol_type::type_get_ () const
  {
    return yystos_[state];
  }


  template <typename Exact>
  void
  parser::yy_destroy_ (const char* yymsg,
                                       symbol_base_type<Exact>& yysym) const
  {
    int yytype = yysym.type_get ();
    YYUSE (yymsg);
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    switch (yytype)
      {
       default:
          break;
      }

    // Type destructor.
    switch (yytype)
    {
      case 43: // ","
      case 44: // ";"
      case 45: // "&"
      case 46: // "|"
      case 47: // "every"
      case 48: // "for"
      case 49: // "loop"
      case 50: // "while"
      case 51: // "at"
        yysym.value.template destroy< ast::flavor_type >();
	break;

      case 52: // "identifier"
      case 64: // "^="
      case 65: // "-="
      case 66: // "%="
      case 67: // "+="
      case 68: // "/="
      case 69: // "*="
      case 77: // "new"
      case 86: // "!"
      case 87: // "bitand"
      case 88: // "bitor"
      case 89: // "^"
      case 90: // "compl"
      case 91: // ">>"
      case 92: // "<<"
      case 93: // "-"
      case 94: // "%"
      case 95: // "+"
      case 96: // "/"
      case 97: // "*"
      case 98: // "**"
      case 99: // "=~="
      case 100: // "=="
      case 101: // "==="
      case 102: // ">="
      case 103: // ">"
      case 104: // "<="
      case 105: // "<"
      case 106: // "!="
      case 107: // "!=="
      case 108: // "~="
      case 109: // "&&"
      case 110: // "||"
      case 135: // event_or_function
      case 155: // id
      case 174: // rel-op
        yysym.value.template destroy< libport::Symbol >();
	break;

      case 118: // root
      case 119: // root_exp
      case 120: // root_exps
      case 122: // cstmt.opt
      case 123: // cstmt
      case 124: // stmt.opt
      case 125: // stmt
      case 126: // block
      case 128: // proto
      case 131: // exp
      case 140: // primary-exp
      case 142: // else.opt
      case 143: // onleave.opt
      case 150: // catch.opt
      case 151: // finally.opt
      case 156: // bitor-exp
      case 157: // new
      case 158: // float-exp
      case 168: // literal-exp
      case 171: // guard.opt
      case 172: // tilda.opt
      case 173: // unary-exp
      case 175: // rel-exp
      case 177: // exp.opt
        yysym.value.template destroy< ast::rExp >();
	break;

      case 138: // modifier
        yysym.value.template destroy< ::ast::Factory::modifier_type >();
	break;

      case 137: // k1_id
        yysym.value.template destroy< ast::rCall >();
	break;

      case 121: // stmts
      case 141: // default.opt
        yysym.value.template destroy< ast::rNary >();
	break;

      case 129: // protos.1
      case 130: // protos
      case 164: // tuple.exps
      case 165: // tuple
      case 166: // bitor-exps
      case 167: // bitor-exps.1
      case 179: // claims
      case 180: // claims.1
      case 181: // exps
      case 182: // exps.1
      case 183: // exps.2
      case 184: // args
      case 185: // args.opt
        yysym.value.template destroy< ast::exps_type* >();
	break;

      case 132: // id.0
      case 133: // id.1
        yysym.value.template destroy< ast::symbols_type >();
	break;

      case 178: // unsigned
        yysym.value.template destroy< unsigned >();
	break;

      case 136: // routine
      case 153: // detach
        yysym.value.template destroy< bool >();
	break;

      case 139: // modifiers
        yysym.value.template destroy< ast::modifiers_type >();
	break;

      case 144: // cases
        yysym.value.template destroy< ::ast::Factory::cases_type >();
	break;

      case 145: // case
        yysym.value.template destroy< ::ast::Factory::case_type >();
	break;

      case 146: // catches.1
        yysym.value.template destroy< ast::catches_type >();
	break;

      case 147: // match
      case 148: // match.opt
        yysym.value.template destroy< ast::rMatch >();
	break;

      case 149: // catch
        yysym.value.template destroy< ast::rCatch >();
	break;

      case 154: // lvalue
        yysym.value.template destroy< ast::rLValue >();
	break;

      case 78: // "angle"
      case 79: // "duration"
      case 80: // "float"
      case 159: // duration
        yysym.value.template destroy< libport::ufloat >();
	break;

      case 160: // assoc
        yysym.value.template destroy< ast::dictionary_elt_type >();
	break;

      case 161: // assocs.1
      case 162: // assocs
        yysym.value.template destroy< ast::dictionary_elts_type >();
	break;

      case 163: // dictionary
        yysym.value.template destroy< ast::rDictionary >();
	break;

      case 82: // "string"
      case 169: // string
        yysym.value.template destroy< std::string >();
	break;

      case 170: // event_match
        yysym.value.template destroy< ast::EventMatch >();
	break;

      case 176: // rel-ops
        yysym.value.template destroy< ::ast::Factory::relations_type >();
	break;

      case 186: // identifiers
        yysym.value.template destroy< ::ast::symbols_type >();
	break;

      case 187: // typespec
      case 188: // typespec.opt
        yysym.value.template destroy< ::ast::rExp >();
	break;

      case 189: // formal
        yysym.value.template destroy< ::ast::Formal >();
	break;

      case 190: // formals.1
      case 191: // formals.0
      case 192: // formals
        yysym.value.template destroy< ::ast::Formals* >();
	break;

      default:
        break;
    }

  }

#if YYDEBUG
  template <typename Exact>
  void
  parser::yy_print_ (std::ostream& yyo,
                                     const symbol_base_type<Exact>& yysym) const
  {
    int yytype = yysym.type_get ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    switch (yytype)
      {
            case 43: // ","

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 546 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 44: // ";"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 555 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 45: // "&"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 564 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 46: // "|"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 573 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 47: // "every"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 582 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 48: // "for"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 591 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 49: // "loop"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 600 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 50: // "while"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 609 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 51: // "at"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 618 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 52: // "identifier"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 627 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 64: // "^="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 636 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 65: // "-="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 645 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 66: // "%="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 654 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 67: // "+="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 663 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 68: // "/="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 672 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 69: // "*="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 681 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 77: // "new"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 690 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 78: // "angle"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 699 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 79: // "duration"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 708 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 80: // "float"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 717 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 82: // "string"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< std::string >(); }
/* Line 576 of lalr1.cc  */
#line 726 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 86: // "!"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 735 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 87: // "bitand"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 744 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 88: // "bitor"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 753 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 89: // "^"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 762 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 90: // "compl"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 771 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 91: // ">>"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 780 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 92: // "<<"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 789 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 93: // "-"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 798 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 94: // "%"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 807 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 95: // "+"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 816 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 96: // "/"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 825 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 97: // "*"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 834 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 98: // "**"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 843 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 99: // "=~="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 852 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 100: // "=="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 861 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 101: // "==="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 870 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 102: // ">="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 879 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 103: // ">"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 888 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 104: // "<="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 897 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 105: // "<"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 906 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 106: // "!="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 915 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 107: // "!=="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 924 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 108: // "~="

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 933 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 109: // "&&"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 942 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 110: // "||"

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 951 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 118: // root

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 960 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 119: // root_exp

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 969 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 120: // root_exps

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 978 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 121: // stmts

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rNary >(); }
/* Line 576 of lalr1.cc  */
#line 987 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 122: // cstmt.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 996 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 123: // cstmt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1005 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 124: // stmt.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1014 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 125: // stmt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1023 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 126: // block

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1032 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 128: // proto

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1041 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 129: // protos.1

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1050 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 130: // protos

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1059 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 131: // exp

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1068 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 132: // id.0

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::symbols_type >(); }
/* Line 576 of lalr1.cc  */
#line 1077 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 133: // id.1

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::symbols_type >(); }
/* Line 576 of lalr1.cc  */
#line 1086 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 135: // event_or_function

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 1095 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 136: // routine

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< bool >(); }
/* Line 576 of lalr1.cc  */
#line 1104 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 137: // k1_id

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rCall >(); }
/* Line 576 of lalr1.cc  */
#line 1113 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 138: // modifier

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::modifier_type >(); }
/* Line 576 of lalr1.cc  */
#line 1122 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 139: // modifiers

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::modifiers_type >(); }
/* Line 576 of lalr1.cc  */
#line 1131 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 140: // primary-exp

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1140 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 141: // default.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rNary >(); }
/* Line 576 of lalr1.cc  */
#line 1149 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 142: // else.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1158 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 143: // onleave.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1167 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 144: // cases

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::cases_type >(); }
/* Line 576 of lalr1.cc  */
#line 1176 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 145: // case

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::case_type >(); }
/* Line 576 of lalr1.cc  */
#line 1185 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 146: // catches.1

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::catches_type >(); }
/* Line 576 of lalr1.cc  */
#line 1194 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 147: // match

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rMatch >(); }
/* Line 576 of lalr1.cc  */
#line 1203 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 148: // match.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rMatch >(); }
/* Line 576 of lalr1.cc  */
#line 1212 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 149: // catch

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rCatch >(); }
/* Line 576 of lalr1.cc  */
#line 1221 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 150: // catch.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1230 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 151: // finally.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1239 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 153: // detach

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< bool >(); }
/* Line 576 of lalr1.cc  */
#line 1248 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 154: // lvalue

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rLValue >(); }
/* Line 576 of lalr1.cc  */
#line 1257 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 155: // id

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 1266 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 156: // bitor-exp

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1275 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 157: // new

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1284 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 158: // float-exp

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1293 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 159: // duration

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 1302 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 160: // assoc

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::dictionary_elt_type >(); }
/* Line 576 of lalr1.cc  */
#line 1311 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 161: // assocs.1

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::dictionary_elts_type >(); }
/* Line 576 of lalr1.cc  */
#line 1320 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 162: // assocs

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::dictionary_elts_type >(); }
/* Line 576 of lalr1.cc  */
#line 1329 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 163: // dictionary

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rDictionary >(); }
/* Line 576 of lalr1.cc  */
#line 1338 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 164: // tuple.exps

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1347 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 165: // tuple

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1356 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 166: // bitor-exps

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1365 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 167: // bitor-exps.1

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1374 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 168: // literal-exp

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1383 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 169: // string

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< std::string >(); }
/* Line 576 of lalr1.cc  */
#line 1392 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 170: // event_match

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::EventMatch >(); }
/* Line 576 of lalr1.cc  */
#line 1401 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 171: // guard.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1410 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 172: // tilda.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1419 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 173: // unary-exp

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1428 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 174: // rel-op

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 1437 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 175: // rel-exp

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1446 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 176: // rel-ops

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::relations_type >(); }
/* Line 576 of lalr1.cc  */
#line 1455 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 177: // exp.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1464 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 178: // unsigned

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< unsigned >(); }
/* Line 576 of lalr1.cc  */
#line 1473 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 179: // claims

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1482 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 180: // claims.1

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1491 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 181: // exps

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1500 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 182: // exps.1

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1509 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 183: // exps.2

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1518 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 184: // args

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1527 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 185: // args.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1536 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 186: // identifiers

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::symbols_type >(); }
/* Line 576 of lalr1.cc  */
#line 1545 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 187: // typespec

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1554 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 188: // typespec.opt

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1563 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 189: // formal

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formal >(); }
/* Line 576 of lalr1.cc  */
#line 1572 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 190: // formals.1

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formals* >(); }
/* Line 576 of lalr1.cc  */
#line 1581 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 191: // formals.0

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formals* >(); }
/* Line 576 of lalr1.cc  */
#line 1590 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 192: // formals

/* Line 576 of lalr1.cc  */
#line 207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formals* >(); }
/* Line 576 of lalr1.cc  */
#line 1599 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

       default:
	  break;
      }
    yyo << ')';
  }
#endif

  void
  parser::yypush_ (const char* m, state_type s,
                                   symbol_type& sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (stack_symbol_type (s, semantic_type(), sym.location));
      switch (yystos_[s])
    {
      case 43: // ","
      case 44: // ";"
      case 45: // "&"
      case 46: // "|"
      case 47: // "every"
      case 48: // "for"
      case 49: // "loop"
      case 50: // "while"
      case 51: // "at"
        yystack_[0].value.build< ast::flavor_type >(sym.value);
	break;

      case 52: // "identifier"
      case 64: // "^="
      case 65: // "-="
      case 66: // "%="
      case 67: // "+="
      case 68: // "/="
      case 69: // "*="
      case 77: // "new"
      case 86: // "!"
      case 87: // "bitand"
      case 88: // "bitor"
      case 89: // "^"
      case 90: // "compl"
      case 91: // ">>"
      case 92: // "<<"
      case 93: // "-"
      case 94: // "%"
      case 95: // "+"
      case 96: // "/"
      case 97: // "*"
      case 98: // "**"
      case 99: // "=~="
      case 100: // "=="
      case 101: // "==="
      case 102: // ">="
      case 103: // ">"
      case 104: // "<="
      case 105: // "<"
      case 106: // "!="
      case 107: // "!=="
      case 108: // "~="
      case 109: // "&&"
      case 110: // "||"
      case 135: // event_or_function
      case 155: // id
      case 174: // rel-op
        yystack_[0].value.build< libport::Symbol >(sym.value);
	break;

      case 118: // root
      case 119: // root_exp
      case 120: // root_exps
      case 122: // cstmt.opt
      case 123: // cstmt
      case 124: // stmt.opt
      case 125: // stmt
      case 126: // block
      case 128: // proto
      case 131: // exp
      case 140: // primary-exp
      case 142: // else.opt
      case 143: // onleave.opt
      case 150: // catch.opt
      case 151: // finally.opt
      case 156: // bitor-exp
      case 157: // new
      case 158: // float-exp
      case 168: // literal-exp
      case 171: // guard.opt
      case 172: // tilda.opt
      case 173: // unary-exp
      case 175: // rel-exp
      case 177: // exp.opt
        yystack_[0].value.build< ast::rExp >(sym.value);
	break;

      case 138: // modifier
        yystack_[0].value.build< ::ast::Factory::modifier_type >(sym.value);
	break;

      case 137: // k1_id
        yystack_[0].value.build< ast::rCall >(sym.value);
	break;

      case 121: // stmts
      case 141: // default.opt
        yystack_[0].value.build< ast::rNary >(sym.value);
	break;

      case 129: // protos.1
      case 130: // protos
      case 164: // tuple.exps
      case 165: // tuple
      case 166: // bitor-exps
      case 167: // bitor-exps.1
      case 179: // claims
      case 180: // claims.1
      case 181: // exps
      case 182: // exps.1
      case 183: // exps.2
      case 184: // args
      case 185: // args.opt
        yystack_[0].value.build< ast::exps_type* >(sym.value);
	break;

      case 132: // id.0
      case 133: // id.1
        yystack_[0].value.build< ast::symbols_type >(sym.value);
	break;

      case 178: // unsigned
        yystack_[0].value.build< unsigned >(sym.value);
	break;

      case 136: // routine
      case 153: // detach
        yystack_[0].value.build< bool >(sym.value);
	break;

      case 139: // modifiers
        yystack_[0].value.build< ast::modifiers_type >(sym.value);
	break;

      case 144: // cases
        yystack_[0].value.build< ::ast::Factory::cases_type >(sym.value);
	break;

      case 145: // case
        yystack_[0].value.build< ::ast::Factory::case_type >(sym.value);
	break;

      case 146: // catches.1
        yystack_[0].value.build< ast::catches_type >(sym.value);
	break;

      case 147: // match
      case 148: // match.opt
        yystack_[0].value.build< ast::rMatch >(sym.value);
	break;

      case 149: // catch
        yystack_[0].value.build< ast::rCatch >(sym.value);
	break;

      case 154: // lvalue
        yystack_[0].value.build< ast::rLValue >(sym.value);
	break;

      case 78: // "angle"
      case 79: // "duration"
      case 80: // "float"
      case 159: // duration
        yystack_[0].value.build< libport::ufloat >(sym.value);
	break;

      case 160: // assoc
        yystack_[0].value.build< ast::dictionary_elt_type >(sym.value);
	break;

      case 161: // assocs.1
      case 162: // assocs
        yystack_[0].value.build< ast::dictionary_elts_type >(sym.value);
	break;

      case 163: // dictionary
        yystack_[0].value.build< ast::rDictionary >(sym.value);
	break;

      case 82: // "string"
      case 169: // string
        yystack_[0].value.build< std::string >(sym.value);
	break;

      case 170: // event_match
        yystack_[0].value.build< ast::EventMatch >(sym.value);
	break;

      case 176: // rel-ops
        yystack_[0].value.build< ::ast::Factory::relations_type >(sym.value);
	break;

      case 186: // identifiers
        yystack_[0].value.build< ::ast::symbols_type >(sym.value);
	break;

      case 187: // typespec
      case 188: // typespec.opt
        yystack_[0].value.build< ::ast::rExp >(sym.value);
	break;

      case 189: // formal
        yystack_[0].value.build< ::ast::Formal >(sym.value);
	break;

      case 190: // formals.1
      case 191: // formals.0
      case 192: // formals
        yystack_[0].value.build< ::ast::Formals* >(sym.value);
	break;

      default:
        break;
    }

  }

  void
  parser::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (stack_symbol_type (s.state, semantic_type(), s.location));
      switch (yystos_[s.state])
    {
      case 43: // ","
      case 44: // ";"
      case 45: // "&"
      case 46: // "|"
      case 47: // "every"
      case 48: // "for"
      case 49: // "loop"
      case 50: // "while"
      case 51: // "at"
        yystack_[0].value.build< ast::flavor_type >(s.value);
	break;

      case 52: // "identifier"
      case 64: // "^="
      case 65: // "-="
      case 66: // "%="
      case 67: // "+="
      case 68: // "/="
      case 69: // "*="
      case 77: // "new"
      case 86: // "!"
      case 87: // "bitand"
      case 88: // "bitor"
      case 89: // "^"
      case 90: // "compl"
      case 91: // ">>"
      case 92: // "<<"
      case 93: // "-"
      case 94: // "%"
      case 95: // "+"
      case 96: // "/"
      case 97: // "*"
      case 98: // "**"
      case 99: // "=~="
      case 100: // "=="
      case 101: // "==="
      case 102: // ">="
      case 103: // ">"
      case 104: // "<="
      case 105: // "<"
      case 106: // "!="
      case 107: // "!=="
      case 108: // "~="
      case 109: // "&&"
      case 110: // "||"
      case 135: // event_or_function
      case 155: // id
      case 174: // rel-op
        yystack_[0].value.build< libport::Symbol >(s.value);
	break;

      case 118: // root
      case 119: // root_exp
      case 120: // root_exps
      case 122: // cstmt.opt
      case 123: // cstmt
      case 124: // stmt.opt
      case 125: // stmt
      case 126: // block
      case 128: // proto
      case 131: // exp
      case 140: // primary-exp
      case 142: // else.opt
      case 143: // onleave.opt
      case 150: // catch.opt
      case 151: // finally.opt
      case 156: // bitor-exp
      case 157: // new
      case 158: // float-exp
      case 168: // literal-exp
      case 171: // guard.opt
      case 172: // tilda.opt
      case 173: // unary-exp
      case 175: // rel-exp
      case 177: // exp.opt
        yystack_[0].value.build< ast::rExp >(s.value);
	break;

      case 138: // modifier
        yystack_[0].value.build< ::ast::Factory::modifier_type >(s.value);
	break;

      case 137: // k1_id
        yystack_[0].value.build< ast::rCall >(s.value);
	break;

      case 121: // stmts
      case 141: // default.opt
        yystack_[0].value.build< ast::rNary >(s.value);
	break;

      case 129: // protos.1
      case 130: // protos
      case 164: // tuple.exps
      case 165: // tuple
      case 166: // bitor-exps
      case 167: // bitor-exps.1
      case 179: // claims
      case 180: // claims.1
      case 181: // exps
      case 182: // exps.1
      case 183: // exps.2
      case 184: // args
      case 185: // args.opt
        yystack_[0].value.build< ast::exps_type* >(s.value);
	break;

      case 132: // id.0
      case 133: // id.1
        yystack_[0].value.build< ast::symbols_type >(s.value);
	break;

      case 178: // unsigned
        yystack_[0].value.build< unsigned >(s.value);
	break;

      case 136: // routine
      case 153: // detach
        yystack_[0].value.build< bool >(s.value);
	break;

      case 139: // modifiers
        yystack_[0].value.build< ast::modifiers_type >(s.value);
	break;

      case 144: // cases
        yystack_[0].value.build< ::ast::Factory::cases_type >(s.value);
	break;

      case 145: // case
        yystack_[0].value.build< ::ast::Factory::case_type >(s.value);
	break;

      case 146: // catches.1
        yystack_[0].value.build< ast::catches_type >(s.value);
	break;

      case 147: // match
      case 148: // match.opt
        yystack_[0].value.build< ast::rMatch >(s.value);
	break;

      case 149: // catch
        yystack_[0].value.build< ast::rCatch >(s.value);
	break;

      case 154: // lvalue
        yystack_[0].value.build< ast::rLValue >(s.value);
	break;

      case 78: // "angle"
      case 79: // "duration"
      case 80: // "float"
      case 159: // duration
        yystack_[0].value.build< libport::ufloat >(s.value);
	break;

      case 160: // assoc
        yystack_[0].value.build< ast::dictionary_elt_type >(s.value);
	break;

      case 161: // assocs.1
      case 162: // assocs
        yystack_[0].value.build< ast::dictionary_elts_type >(s.value);
	break;

      case 163: // dictionary
        yystack_[0].value.build< ast::rDictionary >(s.value);
	break;

      case 82: // "string"
      case 169: // string
        yystack_[0].value.build< std::string >(s.value);
	break;

      case 170: // event_match
        yystack_[0].value.build< ast::EventMatch >(s.value);
	break;

      case 176: // rel-ops
        yystack_[0].value.build< ::ast::Factory::relations_type >(s.value);
	break;

      case 186: // identifiers
        yystack_[0].value.build< ::ast::symbols_type >(s.value);
	break;

      case 187: // typespec
      case 188: // typespec.opt
        yystack_[0].value.build< ::ast::rExp >(s.value);
	break;

      case 189: // formal
        yystack_[0].value.build< ::ast::Formal >(s.value);
	break;

      case 190: // formals.1
      case 191: // formals.0
      case 192: // formals
        yystack_[0].value.build< ::ast::Formals* >(s.value);
	break;

      default:
        break;
    }

  }

  void
  parser::yypop_ (unsigned int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif

  inline parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yylhs)
  {
    int yyr = yypgoto_[yylhs - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yylhs - yyntokens_];
  }

  inline bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::parse ()
  {
    /// Whether yyla contains a lookahead.
    bool yyempty = true;

    /* State.  */
    int yyn;
    int yylen = 0;

    /* Error handling.  */
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// $$ and @$.
    stack_symbol_type yylhs;

    /// The return value of parse().
    int yyresult;

    YYCDEBUG << "Starting parse" << std::endl;


    /* User initialization code.  */
    /* Line 701 of lalr1.cc  */
#line 55 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
{
  // Saved when exiting the start symbol.
  up.scanner_.loc = up.loc_;
}
/* Line 701 of lalr1.cc  */
#line 2134 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"

    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_ = stack_type (0);
    yypush_ (0, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    /* Accept?  */
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    /* Read a lookahead token.  */
    if (yyempty)
      {
        YYCDEBUG << "Reading a token: ";
        try
        {
          yyla = yylex (up);
        }
        catch (const syntax_error& yyexc)
        {
          error (yyexc.location, yyexc.what());
          goto yyerrlab1;
        }
        yyempty = false;
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yy_table_value_is_error_ (yyn))
	  goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Discard the token being shifted.  */
    yyempty = true;

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    /* Shift the lookahead token.  */
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
    /* Variants are always initialized to an empty instance of the
       correct type. The default $$=$1 action is NOT applied when using
       variants.  */
      switch (yyr1_[yyn])
    {
      case 43: // ","
      case 44: // ";"
      case 45: // "&"
      case 46: // "|"
      case 47: // "every"
      case 48: // "for"
      case 49: // "loop"
      case 50: // "while"
      case 51: // "at"
        yylhs.value.build< ast::flavor_type >();
	break;

      case 52: // "identifier"
      case 64: // "^="
      case 65: // "-="
      case 66: // "%="
      case 67: // "+="
      case 68: // "/="
      case 69: // "*="
      case 77: // "new"
      case 86: // "!"
      case 87: // "bitand"
      case 88: // "bitor"
      case 89: // "^"
      case 90: // "compl"
      case 91: // ">>"
      case 92: // "<<"
      case 93: // "-"
      case 94: // "%"
      case 95: // "+"
      case 96: // "/"
      case 97: // "*"
      case 98: // "**"
      case 99: // "=~="
      case 100: // "=="
      case 101: // "==="
      case 102: // ">="
      case 103: // ">"
      case 104: // "<="
      case 105: // "<"
      case 106: // "!="
      case 107: // "!=="
      case 108: // "~="
      case 109: // "&&"
      case 110: // "||"
      case 135: // event_or_function
      case 155: // id
      case 174: // rel-op
        yylhs.value.build< libport::Symbol >();
	break;

      case 118: // root
      case 119: // root_exp
      case 120: // root_exps
      case 122: // cstmt.opt
      case 123: // cstmt
      case 124: // stmt.opt
      case 125: // stmt
      case 126: // block
      case 128: // proto
      case 131: // exp
      case 140: // primary-exp
      case 142: // else.opt
      case 143: // onleave.opt
      case 150: // catch.opt
      case 151: // finally.opt
      case 156: // bitor-exp
      case 157: // new
      case 158: // float-exp
      case 168: // literal-exp
      case 171: // guard.opt
      case 172: // tilda.opt
      case 173: // unary-exp
      case 175: // rel-exp
      case 177: // exp.opt
        yylhs.value.build< ast::rExp >();
	break;

      case 138: // modifier
        yylhs.value.build< ::ast::Factory::modifier_type >();
	break;

      case 137: // k1_id
        yylhs.value.build< ast::rCall >();
	break;

      case 121: // stmts
      case 141: // default.opt
        yylhs.value.build< ast::rNary >();
	break;

      case 129: // protos.1
      case 130: // protos
      case 164: // tuple.exps
      case 165: // tuple
      case 166: // bitor-exps
      case 167: // bitor-exps.1
      case 179: // claims
      case 180: // claims.1
      case 181: // exps
      case 182: // exps.1
      case 183: // exps.2
      case 184: // args
      case 185: // args.opt
        yylhs.value.build< ast::exps_type* >();
	break;

      case 132: // id.0
      case 133: // id.1
        yylhs.value.build< ast::symbols_type >();
	break;

      case 178: // unsigned
        yylhs.value.build< unsigned >();
	break;

      case 136: // routine
      case 153: // detach
        yylhs.value.build< bool >();
	break;

      case 139: // modifiers
        yylhs.value.build< ast::modifiers_type >();
	break;

      case 144: // cases
        yylhs.value.build< ::ast::Factory::cases_type >();
	break;

      case 145: // case
        yylhs.value.build< ::ast::Factory::case_type >();
	break;

      case 146: // catches.1
        yylhs.value.build< ast::catches_type >();
	break;

      case 147: // match
      case 148: // match.opt
        yylhs.value.build< ast::rMatch >();
	break;

      case 149: // catch
        yylhs.value.build< ast::rCatch >();
	break;

      case 154: // lvalue
        yylhs.value.build< ast::rLValue >();
	break;

      case 78: // "angle"
      case 79: // "duration"
      case 80: // "float"
      case 159: // duration
        yylhs.value.build< libport::ufloat >();
	break;

      case 160: // assoc
        yylhs.value.build< ast::dictionary_elt_type >();
	break;

      case 161: // assocs.1
      case 162: // assocs
        yylhs.value.build< ast::dictionary_elts_type >();
	break;

      case 163: // dictionary
        yylhs.value.build< ast::rDictionary >();
	break;

      case 82: // "string"
      case 169: // string
        yylhs.value.build< std::string >();
	break;

      case 170: // event_match
        yylhs.value.build< ast::EventMatch >();
	break;

      case 176: // rel-ops
        yylhs.value.build< ::ast::Factory::relations_type >();
	break;

      case 186: // identifiers
        yylhs.value.build< ::ast::symbols_type >();
	break;

      case 187: // typespec
      case 188: // typespec.opt
        yylhs.value.build< ::ast::rExp >();
	break;

      case 189: // formal
        yylhs.value.build< ::ast::Formal >();
	break;

      case 190: // formals.1
      case 191: // formals.0
      case 192: // formals
        yylhs.value.build< ::ast::Formals* >();
	break;

      default:
        break;
    }


    // Compute the default @$.
    {
      slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
      YYLLOC_DEFAULT (yylhs.location, slice, yylen);
    }

    // Perform the reduction.
    YY_REDUCE_PRINT (yyn);
    try
    {
      switch (yyn)
      {
  case 2:
/* Line 828 of lalr1.cc  */
#line 316 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    // Remove the reference from yystack by swaping with a 0 intrusive
    // pointer.
    aver(up.result_.get() == 0);
    std::swap(up.result_, yystack_[0].value.as< ast::rExp >());
    up.loc_ = yylhs.location;
    YYACCEPT;
  }
/* Line 828 of lalr1.cc  */
#line 2457 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 3:
/* Line 828 of lalr1.cc  */
#line 330 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2465 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 4:
/* Line 828 of lalr1.cc  */
#line 331 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2473 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 5:
/* Line 828 of lalr1.cc  */
#line 332 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2481 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 6:
/* Line 828 of lalr1.cc  */
#line 338 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Stmt(yylhs.location, yystack_[0].value.as< ast::flavor_type >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2489 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 7:
/* Line 828 of lalr1.cc  */
#line 339 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Stmt(yylhs.location, yystack_[0].value.as< ast::flavor_type >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2497 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 8:
/* Line 828 of lalr1.cc  */
#line 340 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Stmt(yylhs.location, ast::flavor_none, yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2505 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 9:
/* Line 828 of lalr1.cc  */
#line 341 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2513 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 10:
/* Line 828 of lalr1.cc  */
#line 342 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2521 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 11:
/* Line 828 of lalr1.cc  */
#line 343 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2529 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 12:
/* Line 828 of lalr1.cc  */
#line 348 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rNary >(); }
/* Line 828 of lalr1.cc  */
#line 2537 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 13:
/* Line 828 of lalr1.cc  */
#line 360 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = MAKE(nary, yylhs.location, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2545 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 14:
/* Line 828 of lalr1.cc  */
#line 361 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = MAKE(nary, yylhs.location, yystack_[2].value.as< ast::rNary >(), yystack_[1].location, yystack_[1].value.as< ast::flavor_type >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2553 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 15:
/* Line 828 of lalr1.cc  */
#line 362 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = MAKE(nary, yylhs.location, yystack_[2].value.as< ast::rNary >(), yystack_[1].location, yystack_[1].value.as< ast::flavor_type >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2561 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 16:
/* Line 828 of lalr1.cc  */
#line 370 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2569 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 17:
/* Line 828 of lalr1.cc  */
#line 371 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2577 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 18:
/* Line 828 of lalr1.cc  */
#line 372 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(bin, yylhs.location, yystack_[0].value.as< ast::flavor_type >(), yystack_[1].value.as< ast::rExp >(), MAKE(noop, yystack_[0].location)); }
/* Line 828 of lalr1.cc  */
#line 2585 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 19:
/* Line 828 of lalr1.cc  */
#line 377 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { assert(yystack_[0].value.as< ast::rExp >()); std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2593 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 20:
/* Line 828 of lalr1.cc  */
#line 378 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(bin, yylhs.location, yystack_[1].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2601 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 21:
/* Line 828 of lalr1.cc  */
#line 379 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(bin, yylhs.location, yystack_[1].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2609 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 22:
/* Line 828 of lalr1.cc  */
#line 389 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2617 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 23:
/* Line 828 of lalr1.cc  */
#line 390 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2625 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 24:
/* Line 828 of lalr1.cc  */
#line 396 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::TaggedStmt(yylhs.location, yystack_[2].value.as< ast::rExp >(), MAKE(scope, yylhs.location, yystack_[0].value.as< ast::rExp >()));
  }
/* Line 828 of lalr1.cc  */
#line 2635 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 25:
/* Line 828 of lalr1.cc  */
#line 406 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2643 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 26:
/* Line 828 of lalr1.cc  */
#line 410 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(strip, yystack_[1].value.as< ast::rNary >()); }
/* Line 828 of lalr1.cc  */
#line 2651 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 27:
/* Line 828 of lalr1.cc  */
#line 411 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2659 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 28:
/* Line 828 of lalr1.cc  */
#line 415 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2667 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 33:
/* Line 828 of lalr1.cc  */
#line 437 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2675 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 34:
/* Line 828 of lalr1.cc  */
#line 443 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2683 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 35:
/* Line 828 of lalr1.cc  */
#line 444 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 2691 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 36:
/* Line 828 of lalr1.cc  */
#line 449 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2699 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 37:
/* Line 828 of lalr1.cc  */
#line 450 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 2707 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 38:
/* Line 828 of lalr1.cc  */
#line 456 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(class, yylhs.location, yystack_[2].value.as< ast::rLValue >(), yystack_[1].value.as< ast::exps_type* >(), yystack_[0].value.as< ast::rExp >(), false);
    }
/* Line 828 of lalr1.cc  */
#line 2717 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 39:
/* Line 828 of lalr1.cc  */
#line 465 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      ast::rClass c = MAKE(class, yylhs.location, yystack_[2].value.as< ast::rLValue >(), yystack_[1].value.as< ast::exps_type* >(), yystack_[0].value.as< ast::rExp >(), true).unsafe_cast<ast::Class>();
      c->is_package_set(true);
      yylhs.value.as< ast::rExp >() = c;
    }
/* Line 828 of lalr1.cc  */
#line 2729 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 40:
/* Line 828 of lalr1.cc  */
#line 480 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {}
/* Line 828 of lalr1.cc  */
#line 2737 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 41:
/* Line 828 of lalr1.cc  */
#line 481 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::symbols_type >(), yystack_[1].value.as< ast::symbols_type >()); }
/* Line 828 of lalr1.cc  */
#line 2745 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 42:
/* Line 828 of lalr1.cc  */
#line 485 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::symbols_type >() << yystack_[0].value.as< libport::Symbol >(); }
/* Line 828 of lalr1.cc  */
#line 2753 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 43:
/* Line 828 of lalr1.cc  */
#line 486 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::symbols_type >(), yystack_[2].value.as< ast::symbols_type >()); yylhs.value.as< ast::symbols_type >() << yystack_[0].value.as< libport::Symbol >(); }
/* Line 828 of lalr1.cc  */
#line 2761 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 44:
/* Line 828 of lalr1.cc  */
#line 492 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(enum, yylhs.location, yystack_[3].value.as< libport::Symbol >(), yystack_[1].value.as< ast::symbols_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2771 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 45:
/* Line 828 of lalr1.cc  */
#line 503 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    REQUIRE_IDENTIFIER(yystack_[0].location, yystack_[0].value.as< libport::Symbol >(), "from");
  }
/* Line 828 of lalr1.cc  */
#line 2781 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 46:
/* Line 828 of lalr1.cc  */
#line 512 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< libport::Symbol >() = SYMBOL(function);
  }
/* Line 828 of lalr1.cc  */
#line 2791 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 47:
/* Line 828 of lalr1.cc  */
#line 516 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    REQUIRE_IDENTIFIER(yystack_[0].location, yystack_[0].value.as< libport::Symbol >(), "event");
    yylhs.value.as< libport::Symbol >() = SYMBOL(event);
  }
/* Line 828 of lalr1.cc  */
#line 2802 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 48:
/* Line 828 of lalr1.cc  */
#line 526 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    REQUIRE_IDENTIFIER(yystack_[1].location, yystack_[1].value.as< libport::Symbol >(), "object");
    yylhs.value.as< ast::rExp >() = MAKE(external_object, yylhs.location, yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 2813 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 49:
/* Line 828 of lalr1.cc  */
#line 532 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(external_var, yylhs.location, yystack_[4].value.as< libport::Symbol >(), yystack_[2].value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 2823 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 50:
/* Line 828 of lalr1.cc  */
#line 538 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(external_event_or_function,
              yylhs.location, yystack_[8].value.as< libport::Symbol >(), yystack_[6].value.as< unsigned >(), yystack_[4].value.as< libport::Symbol >(), yystack_[2].value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 2834 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 51:
/* Line 828 of lalr1.cc  */
#line 549 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    ast::rExp e = yystack_[0].value.as< ast::rLValue >();
    ast::rCall c = yystack_[0].value.as< ast::rLValue >().unsafe_cast<ast::Call>();
    libport::Symbol s = c->name_get();
    bool isStar = (s==SYMBOL(STAR));
    if (isStar)
    {
      e = c->target_get();
      // s is irrelevant in that case
    }
    else
    { // We must replace last call with a getslot
      ast::rExp tgt = c->target_get();
      if (c->target_implicit())
        e = MAKE(call, yylhs.location, c->target_get(), SYMBOL(findSlot),
          MAKE(string, yylhs.location, s));
      else
        e = MAKE(get_slot, yylhs.location, tgt, s);
    }
    ast::LocalDeclaration* ae
      = new ast::LocalDeclaration(yylhs.location, s, e);
    ae->is_star_set(isStar);
    ae->is_import_set(true);
    yylhs.value.as< ast::rExp >() = ae;
  }
/* Line 828 of lalr1.cc  */
#line 2866 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 52:
/* Line 828 of lalr1.cc  */
#line 590 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::Emit(yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[1].value.as< ast::exps_type* >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 2876 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 53:
/* Line 828 of lalr1.cc  */
#line 602 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = true; }
/* Line 828 of lalr1.cc  */
#line 2884 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 54:
/* Line 828 of lalr1.cc  */
#line 603 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = false; }
/* Line 828 of lalr1.cc  */
#line 2892 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 55:
/* Line 828 of lalr1.cc  */
#line 610 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      // Compiled as "var name = function args stmt".
      yylhs.value.as< ast::rExp >() = new ast::Declaration(yylhs.location, yystack_[2].value.as< ast::rCall >(),
                                MAKE(routine, yylhs.location, yystack_[3].value.as< bool >(), yystack_[1].location, yystack_[1].value.as< ::ast::Formals* >(), yystack_[0].value.as< ast::rExp >()));
    }
/* Line 828 of lalr1.cc  */
#line 2904 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 56:
/* Line 828 of lalr1.cc  */
#line 616 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      if (yystack_[3].value.as< libport::Symbol >() == SYMBOL(get) || yystack_[3].value.as< libport::Symbol >() == SYMBOL(set))
      {
        yylhs.value.as< ast::rExp >() = MAKE(define_setter_getter, yylhs.location,
          libport::Symbol("o" + std::string(yystack_[3].value.as< libport::Symbol >())), yystack_[2].value.as< libport::Symbol >(),
          MAKE(routine, yylhs.location, false, yystack_[1].location, yystack_[1].value.as< ::ast::Formals* >(), yystack_[0].value.as< ast::rExp >()));
      }
      else
      {
         yylhs.value.as< ast::rExp >() = MAKE(define_setter_getter, yylhs.location, yystack_[3].value.as< libport::Symbol >(), yystack_[2].value.as< libport::Symbol >(),
          MAKE(routine, yylhs.location, false, yystack_[1].location, yystack_[1].value.as< ::ast::Formals* >(), yystack_[0].value.as< ast::rExp >()));
      }
    }
/* Line 828 of lalr1.cc  */
#line 2924 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 57:
/* Line 828 of lalr1.cc  */
#line 662 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCall >() = MAKE(call, yylhs.location, yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2932 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 58:
/* Line 828 of lalr1.cc  */
#line 663 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCall >() = MAKE(call, yylhs.location, new ast::This(yystack_[2].location), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2940 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 59:
/* Line 828 of lalr1.cc  */
#line 664 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCall >() = MAKE(call, yylhs.location, ast::rExp(yystack_[2].value.as< ast::rCall >()), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2948 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 60:
/* Line 828 of lalr1.cc  */
#line 676 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ::ast::Factory::modifier_type >().first = yystack_[2].value.as< libport::Symbol >();
    yylhs.value.as< ::ast::Factory::modifier_type >().second = yystack_[0].value.as< ast::rExp >();
  }
/* Line 828 of lalr1.cc  */
#line 2959 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 61:
/* Line 828 of lalr1.cc  */
#line 685 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    modifiers_add(up, yystack_[0].location, yylhs.value.as< ast::modifiers_type >(), yystack_[0].value.as< ::ast::Factory::modifier_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2969 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 62:
/* Line 828 of lalr1.cc  */
#line 689 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    std::swap(yylhs.value.as< ast::modifiers_type >(), yystack_[1].value.as< ast::modifiers_type >());
    modifiers_add(up, yystack_[0].location, yylhs.value.as< ast::modifiers_type >(), yystack_[0].value.as< ::ast::Factory::modifier_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2980 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 63:
/* Line 828 of lalr1.cc  */
#line 706 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(assign, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 2990 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 64:
/* Line 828 of lalr1.cc  */
#line 710 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(assign, yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::modifiers_type >());
  }
/* Line 828 of lalr1.cc  */
#line 3000 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 65:
/* Line 828 of lalr1.cc  */
#line 725 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3008 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 66:
/* Line 828 of lalr1.cc  */
#line 726 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3016 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 67:
/* Line 828 of lalr1.cc  */
#line 727 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3024 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 68:
/* Line 828 of lalr1.cc  */
#line 728 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3032 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 69:
/* Line 828 of lalr1.cc  */
#line 729 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3040 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 70:
/* Line 828 of lalr1.cc  */
#line 730 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3048 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 71:
/* Line 828 of lalr1.cc  */
#line 738 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Decrementation(yylhs.location, yystack_[1].value.as< ast::rLValue >(), true); }
/* Line 828 of lalr1.cc  */
#line 3056 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 72:
/* Line 828 of lalr1.cc  */
#line 739 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Incrementation(yylhs.location, yystack_[1].value.as< ast::rLValue >(), true); }
/* Line 828 of lalr1.cc  */
#line 3064 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 73:
/* Line 828 of lalr1.cc  */
#line 751 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::Property(yylhs.location, yystack_[2].value.as< ast::rLValue >()->call(), yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 3074 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 74:
/* Line 828 of lalr1.cc  */
#line 762 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(at, yylhs.location, yystack_[7].location, yystack_[7].value.as< ast::flavor_type >(), yystack_[6].value.as< ::ast::symbols_type >(), yystack_[4].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >(), yystack_[3].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3084 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 75:
/* Line 828 of lalr1.cc  */
#line 766 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(at_event, yylhs.location, yystack_[6].location, yystack_[6].value.as< ast::flavor_type >(), yystack_[5].value.as< ::ast::symbols_type >(), yystack_[3].value.as< ast::EventMatch >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3094 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 76:
/* Line 828 of lalr1.cc  */
#line 770 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(every, yylhs.location, yystack_[4].location, yystack_[4].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3104 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 77:
/* Line 828 of lalr1.cc  */
#line 774 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(if, yylhs.location, yystack_[3].value.as< ast::rNary >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3114 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 78:
/* Line 828 of lalr1.cc  */
#line 778 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(freezeif, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3124 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 79:
/* Line 828 of lalr1.cc  */
#line 782 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(stopif, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3134 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 80:
/* Line 828 of lalr1.cc  */
#line 786 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(switch, yystack_[5].location, yystack_[5].value.as< ast::rExp >(), yystack_[2].value.as< ::ast::Factory::cases_type >(), yystack_[1].value.as< ast::rNary >());
    }
/* Line 828 of lalr1.cc  */
#line 3144 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 81:
/* Line 828 of lalr1.cc  */
#line 790 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(timeout, yylhs.location,
                yystack_[5].value.as< ast::rExp >(), yystack_[3].value.as< ast::rExp >(), yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3155 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 82:
/* Line 828 of lalr1.cc  */
#line 795 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = new ast::Return(yylhs.location, yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3165 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 83:
/* Line 828 of lalr1.cc  */
#line 799 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = new ast::Break(yylhs.location);
    }
/* Line 828 of lalr1.cc  */
#line 3175 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 84:
/* Line 828 of lalr1.cc  */
#line 803 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = new ast::Continue(yylhs.location);
    }
/* Line 828 of lalr1.cc  */
#line 3185 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 85:
/* Line 828 of lalr1.cc  */
#line 807 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(waituntil, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3195 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 86:
/* Line 828 of lalr1.cc  */
#line 811 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(waituntil_event, yylhs.location, yystack_[1].value.as< ast::EventMatch >());
    }
/* Line 828 of lalr1.cc  */
#line 3205 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 87:
/* Line 828 of lalr1.cc  */
#line 815 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(whenever, yylhs.location, yystack_[4].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >(), yystack_[3].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3215 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 88:
/* Line 828 of lalr1.cc  */
#line 819 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(whenever_event, yylhs.location, yystack_[3].value.as< ast::EventMatch >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3225 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 89:
/* Line 828 of lalr1.cc  */
#line 835 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = 0;            }
/* Line 828 of lalr1.cc  */
#line 3233 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 90:
/* Line 828 of lalr1.cc  */
#line 836 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rNary >(), yystack_[0].value.as< ast::rNary >()); }
/* Line 828 of lalr1.cc  */
#line 3241 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 91:
/* Line 828 of lalr1.cc  */
#line 841 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0;            }
/* Line 828 of lalr1.cc  */
#line 3249 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 92:
/* Line 828 of lalr1.cc  */
#line 842 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3257 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 93:
/* Line 828 of lalr1.cc  */
#line 848 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0;            }
/* Line 828 of lalr1.cc  */
#line 3265 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 94:
/* Line 828 of lalr1.cc  */
#line 849 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3273 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 95:
/* Line 828 of lalr1.cc  */
#line 859 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {}
/* Line 828 of lalr1.cc  */
#line 3281 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 96:
/* Line 828 of lalr1.cc  */
#line 860 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Factory::cases_type >(), yystack_[1].value.as< ::ast::Factory::cases_type >()); yylhs.value.as< ::ast::Factory::cases_type >() << yystack_[0].value.as< ::ast::Factory::case_type >(); }
/* Line 828 of lalr1.cc  */
#line 3289 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 97:
/* Line 828 of lalr1.cc  */
#line 866 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Factory::case_type >() = ::ast::Factory::case_type(yystack_[2].value.as< ast::rMatch >(), yystack_[0].value.as< ast::rNary >()); }
/* Line 828 of lalr1.cc  */
#line 3297 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 98:
/* Line 828 of lalr1.cc  */
#line 875 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::catches_type >() = ast::catches_type(); yylhs.value.as< ast::catches_type >() << yystack_[0].value.as< ast::rCatch >(); }
/* Line 828 of lalr1.cc  */
#line 3305 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 99:
/* Line 828 of lalr1.cc  */
#line 876 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::catches_type >(), yystack_[1].value.as< ast::catches_type >());        yylhs.value.as< ast::catches_type >() << yystack_[0].value.as< ast::rCatch >(); }
/* Line 828 of lalr1.cc  */
#line 3313 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 100:
/* Line 828 of lalr1.cc  */
#line 881 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rMatch >() = new ast::Match(yylhs.location, yystack_[0].value.as< ast::rExp >(), 0);  }
/* Line 828 of lalr1.cc  */
#line 3321 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 101:
/* Line 828 of lalr1.cc  */
#line 882 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rMatch >() = new ast::Match(yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3329 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 102:
/* Line 828 of lalr1.cc  */
#line 885 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rMatch >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3337 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 103:
/* Line 828 of lalr1.cc  */
#line 886 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rMatch >(), yystack_[1].value.as< ast::rMatch >()); }
/* Line 828 of lalr1.cc  */
#line 3345 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 104:
/* Line 828 of lalr1.cc  */
#line 890 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCatch >() = MAKE(catch, yylhs.location, yystack_[1].value.as< ast::rMatch >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3353 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 105:
/* Line 828 of lalr1.cc  */
#line 897 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3361 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 106:
/* Line 828 of lalr1.cc  */
#line 898 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 3369 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 107:
/* Line 828 of lalr1.cc  */
#line 904 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0;  }
/* Line 828 of lalr1.cc  */
#line 3377 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 108:
/* Line 828 of lalr1.cc  */
#line 905 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 3385 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 109:
/* Line 828 of lalr1.cc  */
#line 910 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(try, yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[2].value.as< ast::catches_type >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3395 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 110:
/* Line 828 of lalr1.cc  */
#line 914 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(finally, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3405 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 111:
/* Line 828 of lalr1.cc  */
#line 918 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(throw, yylhs.location, yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3415 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 112:
/* Line 828 of lalr1.cc  */
#line 947 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(loop, yylhs.location, yystack_[1].location, yystack_[1].value.as< ast::flavor_type >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3425 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 113:
/* Line 828 of lalr1.cc  */
#line 951 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(for, yylhs.location, yystack_[4].location, yystack_[4].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3435 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 114:
/* Line 828 of lalr1.cc  */
#line 955 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(for, yylhs.location, yystack_[8].location, yystack_[8].value.as< ast::flavor_type >(), yystack_[6].value.as< ast::rExp >(), yystack_[4].value.as< ast::rExp >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3445 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 115:
/* Line 828 of lalr1.cc  */
#line 959 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(for, yylhs.location, yystack_[7].location, yystack_[7].value.as< ast::flavor_type >(), yystack_[4].location, yystack_[4].value.as< libport::Symbol >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3455 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 116:
/* Line 828 of lalr1.cc  */
#line 963 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(while, yylhs.location, yystack_[4].location, yystack_[4].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3465 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 119:
/* Line 828 of lalr1.cc  */
#line 978 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(scope, yylhs.location, 0, yystack_[0].value.as< ast::rExp >());  }
/* Line 828 of lalr1.cc  */
#line 3473 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 120:
/* Line 828 of lalr1.cc  */
#line 979 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(scope, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3481 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 121:
/* Line 828 of lalr1.cc  */
#line 991 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = true; }
/* Line 828 of lalr1.cc  */
#line 3489 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 122:
/* Line 828 of lalr1.cc  */
#line 992 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = false; }
/* Line 828 of lalr1.cc  */
#line 3497 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 123:
/* Line 828 of lalr1.cc  */
#line 996 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(assert, yylhs.location, yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3505 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 124:
/* Line 828 of lalr1.cc  */
#line 997 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(assert, yylhs.location, yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3513 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 125:
/* Line 828 of lalr1.cc  */
#line 998 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(detach, yylhs.location, yystack_[3].value.as< bool >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3521 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 126:
/* Line 828 of lalr1.cc  */
#line 999 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(detach, yylhs.location, yystack_[1].value.as< bool >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3529 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 127:
/* Line 828 of lalr1.cc  */
#line 1000 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(isdef, yylhs.location, yystack_[1].value.as< ast::rCall >()); }
/* Line 828 of lalr1.cc  */
#line 3537 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 128:
/* Line 828 of lalr1.cc  */
#line 1001 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(watch, yylhs.location, yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3545 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 129:
/* Line 828 of lalr1.cc  */
#line 1011 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rLValue >() = MAKE(call, yylhs.location, yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3553 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 130:
/* Line 828 of lalr1.cc  */
#line 1012 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rLValue >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3561 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 131:
/* Line 828 of lalr1.cc  */
#line 1013 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rLValue >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3569 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 132:
/* Line 828 of lalr1.cc  */
#line 1017 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(get_slot, yylhs.location, yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3577 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 133:
/* Line 828 of lalr1.cc  */
#line 1018 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(get_slot, yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3585 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 134:
/* Line 828 of lalr1.cc  */
#line 1022 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3593 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 135:
/* Line 828 of lalr1.cc  */
#line 1027 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(binding, yylhs.location, false, yystack_[0].location, yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3603 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 136:
/* Line 828 of lalr1.cc  */
#line 1031 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(binding, yylhs.location, true, yystack_[0].location, yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3613 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 137:
/* Line 828 of lalr1.cc  */
#line 1038 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rLValue >();
  }
/* Line 828 of lalr1.cc  */
#line 3623 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 138:
/* Line 828 of lalr1.cc  */
#line 1042 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = yystack_[1].value.as< ast::rLValue >();
    yylhs.value.as< ast::rExp >().unchecked_cast<ast::LValueArgs>()->arguments_set(yystack_[0].value.as< ast::exps_type* >());
    yylhs.value.as< ast::rExp >()->location_set(yylhs.location);
  }
/* Line 828 of lalr1.cc  */
#line 3635 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 139:
/* Line 828 of lalr1.cc  */
#line 1055 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    // Compiled as "id . new (args)".
    ast::exps_type* args = yystack_[0].value.as< ast::exps_type* >();
    if (!args)
      args = new ast::exps_type();
    yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, MAKE(call, yylhs.location, yystack_[1].value.as< libport::Symbol >()), SYMBOL(new), args);
    up.deprecated(yylhs.location, "new Obj(x)", "Obj.new(x)");
  }
/* Line 828 of lalr1.cc  */
#line 3650 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 140:
/* Line 828 of lalr1.cc  */
#line 1066 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3658 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 141:
/* Line 828 of lalr1.cc  */
#line 1071 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3666 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 142:
/* Line 828 of lalr1.cc  */
#line 1082 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(routine, yylhs.location, yystack_[2].value.as< bool >(), yystack_[1].location, yystack_[1].value.as< ::ast::Formals* >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3676 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 143:
/* Line 828 of lalr1.cc  */
#line 1098 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(float, yylhs.location, yystack_[0].value.as< libport::ufloat >()); }
/* Line 828 of lalr1.cc  */
#line 3684 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 144:
/* Line 828 of lalr1.cc  */
#line 1108 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< libport::ufloat >() = yystack_[0].value.as< libport::ufloat >();      }
/* Line 828 of lalr1.cc  */
#line 3692 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 145:
/* Line 828 of lalr1.cc  */
#line 1109 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< libport::ufloat >() = yystack_[1].value.as< libport::ufloat >() + yystack_[0].value.as< libport::ufloat >(); }
/* Line 828 of lalr1.cc  */
#line 3700 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 146:
/* Line 828 of lalr1.cc  */
#line 1123 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::dictionary_elt_type >().first = yystack_[2].value.as< ast::rExp >();
    yylhs.value.as< ast::dictionary_elt_type >().second = yystack_[0].value.as< ast::rExp >();
  }
/* Line 828 of lalr1.cc  */
#line 3711 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 147:
/* Line 828 of lalr1.cc  */
#line 1131 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    assocs_add(up, yystack_[0].location, yylhs.value.as< ast::dictionary_elts_type >(), yystack_[0].value.as< ast::dictionary_elt_type >());
  }
/* Line 828 of lalr1.cc  */
#line 3721 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 148:
/* Line 828 of lalr1.cc  */
#line 1135 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    std::swap(yylhs.value.as< ast::dictionary_elts_type >(), yystack_[2].value.as< ast::dictionary_elts_type >());
    assocs_add(up, yystack_[0].location, yylhs.value.as< ast::dictionary_elts_type >(), yystack_[0].value.as< ast::dictionary_elt_type >());
  }
/* Line 828 of lalr1.cc  */
#line 3732 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 149:
/* Line 828 of lalr1.cc  */
#line 1142 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { /* nothing */ }
/* Line 828 of lalr1.cc  */
#line 3740 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 150:
/* Line 828 of lalr1.cc  */
#line 1143 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::dictionary_elts_type >(), yystack_[1].value.as< ast::dictionary_elts_type >()); }
/* Line 828 of lalr1.cc  */
#line 3748 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 151:
/* Line 828 of lalr1.cc  */
#line 1148 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rDictionary >() = new ast::Dictionary(yylhs.location, yystack_[1].value.as< ast::dictionary_elts_type >()); }
/* Line 828 of lalr1.cc  */
#line 3756 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 152:
/* Line 828 of lalr1.cc  */
#line 1159 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 3764 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 153:
/* Line 828 of lalr1.cc  */
#line 1160 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3772 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 154:
/* Line 828 of lalr1.cc  */
#line 1161 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3780 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 155:
/* Line 828 of lalr1.cc  */
#line 1165 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = yystack_[1].value.as< ast::exps_type* >(); }
/* Line 828 of lalr1.cc  */
#line 3788 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 156:
/* Line 828 of lalr1.cc  */
#line 1179 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 3796 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 157:
/* Line 828 of lalr1.cc  */
#line 1180 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3804 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 158:
/* Line 828 of lalr1.cc  */
#line 1184 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3812 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 159:
/* Line 828 of lalr1.cc  */
#line 1185 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >();}
/* Line 828 of lalr1.cc  */
#line 3820 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 160:
/* Line 828 of lalr1.cc  */
#line 1193 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());  }
/* Line 828 of lalr1.cc  */
#line 3828 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 161:
/* Line 828 of lalr1.cc  */
#line 1194 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(float, yylhs.location, yystack_[0].value.as< libport::ufloat >());  }
/* Line 828 of lalr1.cc  */
#line 3836 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 162:
/* Line 828 of lalr1.cc  */
#line 1195 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(float, yylhs.location, yystack_[0].value.as< libport::ufloat >());  }
/* Line 828 of lalr1.cc  */
#line 3844 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 163:
/* Line 828 of lalr1.cc  */
#line 1196 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(string, yylhs.location, yystack_[0].value.as< std::string >()); }
/* Line 828 of lalr1.cc  */
#line 3852 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 164:
/* Line 828 of lalr1.cc  */
#line 1197 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(list, yylhs.location, yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3860 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 165:
/* Line 828 of lalr1.cc  */
#line 1198 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(vector, yylhs.location, yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3868 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 166:
/* Line 828 of lalr1.cc  */
#line 1199 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(vector, yylhs.location, yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3876 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 167:
/* Line 828 of lalr1.cc  */
#line 1200 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rDictionary >(); }
/* Line 828 of lalr1.cc  */
#line 3884 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 168:
/* Line 828 of lalr1.cc  */
#line 1201 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(tuple, yylhs.location, yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3892 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 169:
/* Line 828 of lalr1.cc  */
#line 1207 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< std::string >(), yystack_[0].value.as< std::string >());  }
/* Line 828 of lalr1.cc  */
#line 3900 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 170:
/* Line 828 of lalr1.cc  */
#line 1208 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< std::string >(), yystack_[1].value.as< std::string >()); yylhs.value.as< std::string >() += yystack_[0].value.as< std::string >(); }
/* Line 828 of lalr1.cc  */
#line 3908 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 171:
/* Line 828 of lalr1.cc  */
#line 1216 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(position, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3916 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 172:
/* Line 828 of lalr1.cc  */
#line 1227 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::EventMatch >() = MAKE(event_match, yylhs.location, yystack_[4].value.as< ast::rExp >(), yystack_[2].value.as< ast::exps_type* >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3926 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 173:
/* Line 828 of lalr1.cc  */
#line 1234 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3934 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 174:
/* Line 828 of lalr1.cc  */
#line 1235 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3942 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 175:
/* Line 828 of lalr1.cc  */
#line 1240 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3950 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 176:
/* Line 828 of lalr1.cc  */
#line 1241 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3958 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 177:
/* Line 828 of lalr1.cc  */
#line 1251 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::Subscript(yylhs.location, yystack_[1].value.as< ast::exps_type* >(), yystack_[3].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3968 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 178:
/* Line 828 of lalr1.cc  */
#line 1266 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::This(yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3976 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 179:
/* Line 828 of lalr1.cc  */
#line 1267 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::CallMsg(yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3984 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 180:
/* Line 828 of lalr1.cc  */
#line 1271 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3992 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 181:
/* Line 828 of lalr1.cc  */
#line 1272 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4000 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 182:
/* Line 828 of lalr1.cc  */
#line 1273 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 4008 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 183:
/* Line 828 of lalr1.cc  */
#line 1274 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4016 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 184:
/* Line 828 of lalr1.cc  */
#line 1279 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4024 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 185:
/* Line 828 of lalr1.cc  */
#line 1280 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Decrementation(yylhs.location, yystack_[0].value.as< ast::rLValue >(), false); }
/* Line 828 of lalr1.cc  */
#line 4032 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 186:
/* Line 828 of lalr1.cc  */
#line 1281 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Incrementation(yylhs.location, yystack_[0].value.as< ast::rLValue >(), false); }
/* Line 828 of lalr1.cc  */
#line 4040 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 187:
/* Line 828 of lalr1.cc  */
#line 1282 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 4048 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 188:
/* Line 828 of lalr1.cc  */
#line 1283 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 4056 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 189:
/* Line 828 of lalr1.cc  */
#line 1284 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 4064 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 190:
/* Line 828 of lalr1.cc  */
#line 1285 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 4072 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 191:
/* Line 828 of lalr1.cc  */
#line 1310 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4080 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 192:
/* Line 828 of lalr1.cc  */
#line 1311 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4088 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 193:
/* Line 828 of lalr1.cc  */
#line 1312 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4096 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 194:
/* Line 828 of lalr1.cc  */
#line 1313 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4104 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 195:
/* Line 828 of lalr1.cc  */
#line 1314 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4112 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 196:
/* Line 828 of lalr1.cc  */
#line 1315 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4120 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 197:
/* Line 828 of lalr1.cc  */
#line 1316 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4128 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 198:
/* Line 828 of lalr1.cc  */
#line 1317 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4136 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 199:
/* Line 828 of lalr1.cc  */
#line 1318 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4144 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 200:
/* Line 828 of lalr1.cc  */
#line 1319 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4152 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 201:
/* Line 828 of lalr1.cc  */
#line 1320 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4160 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 202:
/* Line 828 of lalr1.cc  */
#line 1346 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4168 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 203:
/* Line 828 of lalr1.cc  */
#line 1347 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4176 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 204:
/* Line 828 of lalr1.cc  */
#line 1348 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4184 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 205:
/* Line 828 of lalr1.cc  */
#line 1349 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4192 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 206:
/* Line 828 of lalr1.cc  */
#line 1350 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4200 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 207:
/* Line 828 of lalr1.cc  */
#line 1351 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4208 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 208:
/* Line 828 of lalr1.cc  */
#line 1352 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4216 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 209:
/* Line 828 of lalr1.cc  */
#line 1353 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4224 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 210:
/* Line 828 of lalr1.cc  */
#line 1354 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4232 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 211:
/* Line 828 of lalr1.cc  */
#line 1355 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4240 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 212:
/* Line 828 of lalr1.cc  */
#line 1359 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(relation, yylhs.location, yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ::ast::Factory::relations_type >()); }
/* Line 828 of lalr1.cc  */
#line 4248 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 213:
/* Line 828 of lalr1.cc  */
#line 1364 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { /* empty */ }
/* Line 828 of lalr1.cc  */
#line 4256 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 214:
/* Line 828 of lalr1.cc  */
#line 1365 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Factory::relations_type >(), MAKE(relation, yystack_[2].value.as< ::ast::Factory::relations_type >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >())); }
/* Line 828 of lalr1.cc  */
#line 4264 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 215:
/* Line 828 of lalr1.cc  */
#line 1379 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4272 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 216:
/* Line 828 of lalr1.cc  */
#line 1380 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(and, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4280 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 217:
/* Line 828 of lalr1.cc  */
#line 1381 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(or,  yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4288 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 218:
/* Line 828 of lalr1.cc  */
#line 1386 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), SYMBOL(has),    yystack_[2].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4296 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 219:
/* Line 828 of lalr1.cc  */
#line 1387 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), SYMBOL(hasNot), yystack_[3].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4304 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 220:
/* Line 828 of lalr1.cc  */
#line 1391 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 4312 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 221:
/* Line 828 of lalr1.cc  */
#line 1392 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4320 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 222:
/* Line 828 of lalr1.cc  */
#line 1403 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< unsigned >() = static_cast<unsigned int>(yystack_[0].value.as< libport::ufloat >()); }
/* Line 828 of lalr1.cc  */
#line 4328 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 223:
/* Line 828 of lalr1.cc  */
#line 1412 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::Unscope(yylhs.location, yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4338 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 224:
/* Line 828 of lalr1.cc  */
#line 1424 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::MetaExp(yylhs.location, yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4348 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 225:
/* Line 828 of lalr1.cc  */
#line 1432 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::MetaLValue(yylhs.location, new ast::exps_type(), yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4358 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 226:
/* Line 828 of lalr1.cc  */
#line 1440 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::MetaId(yylhs.location, 0, yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4368 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 227:
/* Line 828 of lalr1.cc  */
#line 1444 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::MetaCall(yylhs.location, 0, yystack_[3].value.as< ast::rExp >(), yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4378 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 228:
/* Line 828 of lalr1.cc  */
#line 1452 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    assert(yystack_[4].value.as< ast::rLValue >().unsafe_cast<ast::LValueArgs>());
    assert(!yystack_[4].value.as< ast::rLValue >().unsafe_cast<ast::LValueArgs>()->arguments_get());
    yylhs.value.as< ast::rExp >() = new ast::MetaArgs(yylhs.location, yystack_[4].value.as< ast::rLValue >(), yystack_[1].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4390 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 229:
/* Line 828 of lalr1.cc  */
#line 1468 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 4398 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 230:
/* Line 828 of lalr1.cc  */
#line 1469 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4406 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 231:
/* Line 828 of lalr1.cc  */
#line 1473 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4414 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 232:
/* Line 828 of lalr1.cc  */
#line 1474 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 4422 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 233:
/* Line 828 of lalr1.cc  */
#line 1480 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 4430 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 234:
/* Line 828 of lalr1.cc  */
#line 1481 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4438 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 235:
/* Line 828 of lalr1.cc  */
#line 1485 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4446 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 236:
/* Line 828 of lalr1.cc  */
#line 1486 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 4454 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 237:
/* Line 828 of lalr1.cc  */
#line 1490 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 4462 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 238:
/* Line 828 of lalr1.cc  */
#line 1495 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4470 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 239:
/* Line 828 of lalr1.cc  */
#line 1499 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = 0; }
/* Line 828 of lalr1.cc  */
#line 4478 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 240:
/* Line 828 of lalr1.cc  */
#line 1500 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4486 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 241:
/* Line 828 of lalr1.cc  */
#line 1510 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { /* empty */ }
/* Line 828 of lalr1.cc  */
#line 4494 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 242:
/* Line 828 of lalr1.cc  */
#line 1511 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::symbols_type >(), yystack_[1].value.as< ::ast::symbols_type >()); yylhs.value.as< ::ast::symbols_type >().push_back(yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4502 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 243:
/* Line 828 of lalr1.cc  */
#line 1516 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::rExp >() = yystack_[0].value.as< ast::rExp >();}
/* Line 828 of lalr1.cc  */
#line 4510 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 244:
/* Line 828 of lalr1.cc  */
#line 1521 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::rExp >()=0;}
/* Line 828 of lalr1.cc  */
#line 4518 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 245:
/* Line 828 of lalr1.cc  */
#line 1522 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::rExp >(), yystack_[0].value.as< ::ast::rExp >());}
/* Line 828 of lalr1.cc  */
#line 4526 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 246:
/* Line 828 of lalr1.cc  */
#line 1527 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formal >() = ::ast::Formal(yystack_[1].value.as< libport::Symbol >(), 0, yystack_[0].value.as< ::ast::rExp >());  }
/* Line 828 of lalr1.cc  */
#line 4534 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 247:
/* Line 828 of lalr1.cc  */
#line 1528 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formal >() = ::ast::Formal(yystack_[3].value.as< libport::Symbol >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ::ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4542 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 248:
/* Line 828 of lalr1.cc  */
#line 1529 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formal >() = ::ast::Formal(yystack_[2].value.as< libport::Symbol >(), true); }
/* Line 828 of lalr1.cc  */
#line 4550 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 249:
/* Line 828 of lalr1.cc  */
#line 1535 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formals* >() = new ::ast::Formals(1, yystack_[0].value.as< ::ast::Formal >()); }
/* Line 828 of lalr1.cc  */
#line 4558 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 250:
/* Line 828 of lalr1.cc  */
#line 1536 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Formals* >(), yystack_[2].value.as< ::ast::Formals* >()); *yylhs.value.as< ::ast::Formals* >() << yystack_[0].value.as< ::ast::Formal >(); }
/* Line 828 of lalr1.cc  */
#line 4566 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 251:
/* Line 828 of lalr1.cc  */
#line 1541 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formals* >() = new ::ast::Formals; }
/* Line 828 of lalr1.cc  */
#line 4574 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 252:
/* Line 828 of lalr1.cc  */
#line 1542 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Formals* >(), yystack_[1].value.as< ::ast::Formals* >()); }
/* Line 828 of lalr1.cc  */
#line 4582 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 253:
/* Line 828 of lalr1.cc  */
#line 1547 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formals* >() = 0; }
/* Line 828 of lalr1.cc  */
#line 4590 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 254:
/* Line 828 of lalr1.cc  */
#line 1548 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Formals* >(), yystack_[1].value.as< ::ast::Formals* >()); }
/* Line 828 of lalr1.cc  */
#line 4598 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;


/* Line 828 of lalr1.cc  */
#line 4603 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
	default:
          break;
      }
    }
    catch (const syntax_error& yyexc)
    {
      error (yyexc.location, yyexc.what());
      YYERROR;
    }
    YY_SYMBOL_PRINT ("-> $$ =", yylhs);

    // Destroy the rhs symbols.
    for (int i = 0; i < yylen; ++i)
      // Destroy a variant which value may have been swapped with
      // yylhs.value (for instance if the action was "std::swap($$,
      // $1)").  The value of yylhs.value (hence possibly one of these
      // rhs symbols) depends on the default contruction for this
      // type.  In the case of pointers for instance, no
      // initialization is done, so the value is junk.  Therefore do
      // not try to report the value of symbols about to be destroyed
      // in the debug trace, it's possibly junk.  Hence yymsg = 0.
      // Besides, that keeps exactly the same traces as with the other
      // Bison skeletons.
      yy_destroy_ (0, yystack_[i]);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    /* Shift the result of the reduction.  */
    yypush_ (0, yylhs);
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	error (yyla.location, yysyntax_error_ (yystack_[0].state,
                                           yyempty ? yyempty_ : yyla.type));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
	/* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        /* Return failure if at end of input.  */
        if (yyla.type == yyeof_)
          YYABORT;
        else if (!yyempty)
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyempty = true;
	  }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* $$ was initialized before running the user action.  */
    yy_destroy_ ("Error: discarding", yylhs);
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      /* Shift the error token.  */
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyempty)
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (yystack_.size () != 1)
      {
	yy_destroy_ ("Cleanup: popping", yystack_[0]);
	yypop_ ();
      }

    return yyresult;
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type yystate, int yytoken)
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            /* Stay within bounds of both yycheck and yytname.  */
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = 0;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short int parser::yypact_ninf_ = -343;

  const short int parser::yytable_ninf_ = -260;

  const short int
  parser::yypact_[] =
  {
     252,  -343,   925,  1228,    19,  -343,    21,  -343,  -343,  -343,
     -13,  -343,    16,  -343,    24,    34,  1039,  1416,   785,  1494,
    1572,    44,    73,  1572,    99,    87,  1650,   127,   132,   138,
      -7,   146,   188,  1228,   206,  -343,  -343,  1884,  1884,    -7,
     166,  1884,  1884,  1884,   213,    90,  -343,  -343,   202,  -343,
    -343,  -343,  -343,  -343,  -343,  1806,  1806,  1806,  1806,  1650,
      46,    46,    46,    46,  -343,    76,   179,  -343,  -343,   162,
      -1,     2,   193,   601,    -7,   494,  -343,  -343,   182,  -343,
    -343,  -343,   195,  -343,  -343,  -343,   216,  -343,  -343,  -343,
    -343,  -343,  1650,  1494,  1228,   -36,   246,    74,     5,  -343,
      30,   258,     6,  -343,  -343,   243,   257,   259,   245,   266,
      69,   267,   248,  -343,   454,   277,   454,  -343,  1494,  1494,
    -343,  1494,    26,   -11,   494,  1494,  1494,  1494,  -343,  -343,
    1494,  1338,  -343,  1494,    -6,     6,   161,   161,   282,  -343,
     254,   261,   283,    63,    63,    63,  1494,  1494,  1494,   287,
    -343,  -343,  -343,  -343,   494,   211,   274,  -343,  -343,  -343,
    -343,  -343,  -343,  -343,  -343,  1228,  1228,  1494,  1494,   128,
    1494,  1494,    14,  -343,   293,   130,    87,  1228,  1494,   218,
    1884,  1494,  -343,  1117,  1494,  1494,  1494,  1494,  1494,  1494,
    -343,  -343,    -7,  -343,   258,  1650,  1650,  1650,  1650,  1650,
    1650,  1650,  1650,  1650,  1650,   611,  -343,  -343,  1228,  1228,
     494,   118,   100,   141,  -343,  -343,    -7,  1494,   296,  1494,
    -343,  -343,  -343,  1494,  -343,  -343,  -343,  -343,  1494,  -343,
     125,   140,   232,   316,    87,   106,  -343,    27,   311,   260,
      27,   318,   270,  1728,   300,  -343,   273,   288,  1494,  -343,
     185,    87,    87,    -7,   326,  -343,    46,   290,   454,   328,
     314,   306,  1494,  -343,  -343,  -343,  1650,  -343,  -343,   322,
     303,     4,  1494,   332,     7,    -3,  -343,  -343,   329,   343,
     333,   336,   341,    87,  -343,  -343,   348,    -7,  -343,    46,
    -343,     6,   417,    46,   349,   454,   454,   454,   454,   454,
     454,  -343,    87,   312,   587,   612,   234,   234,   187,  -343,
     187,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,  1650,  -343,  -343,  1228,  1228,  -343,   355,
     454,    30,  -343,   454,    79,  1228,   372,  1228,  1494,    87,
    -343,  1228,   379,  -343,  1494,   287,   364,  -343,  -343,   369,
    1228,  1228,    85,  1494,  1228,  1228,    27,   382,  -343,  -343,
    -343,  1494,  -343,   368,  -343,  -343,   388,   378,  -343,   373,
     396,    87,  -343,  1494,  -343,  -343,   494,   418,  -343,   383,
       4,  -343,    14,  -343,  -343,   136,  -343,  -343,  -343,  -343,
    -343,  -343,  -343,   398,  -343,  -343,   494,  -343,   419,  -343,
    -343,  -343,   427,   420,   409,  -343,  -343,    87,  -343,   454,
     332,  -343,  1228,   419,  -343,  -343,  -343,  1494,   439,  -343,
    -343,   412,  1228,   454,   185,  -343,    -7,  -343,   400,   402,
    -343,   454,  1494,  -343,  -343,  1494,  1494,   425,  -343,  -343,
    -343,  -343,   139,    87,   419,  1494,  -343,  -343,   437,   419,
    -343,   447,  1228,  1228,   434,  -343,  -343,  -343,   413,   438,
     454,   451,   454,  -343,  1494,   455,   441,  -343,  -343,   379,
     454,  1494,  -343,  -343,  1228,   444,   434,  1228,  -343,  -343,
     421,  -343,   464,  1228,  -343,  -343,   454,  -343,  1228,  -343,
    -343,   400,  1228,   216,  -343,   428,   216,  -343
  };

  const unsigned short int
  parser::yydefact_[] =
  {
       0,     3,     0,    16,     0,     2,     0,   171,    83,    53,
       0,    84,     0,    54,     0,     0,     0,   233,     0,   152,
     220,     0,     0,   220,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   241,   134,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   121,   122,   141,   161,
     144,   143,   169,   179,   178,     0,     0,     0,     0,   156,
       0,     0,     0,     0,     4,     0,    17,    19,   119,    25,
     253,   184,     0,   137,   129,   213,   140,   160,   162,   167,
     168,   180,   163,   191,   215,     5,    12,    13,     1,    11,
      10,     9,     0,     0,    16,     0,     0,     0,   129,   149,
     235,   253,   184,   129,   147,   255,     0,     0,   255,     0,
     235,     0,     0,   154,   235,     0,   221,    82,     0,     0,
     111,     0,     0,   137,   135,     0,     0,     0,   141,   132,
       0,    22,   112,     0,     0,     0,   137,   137,     0,    46,
       0,    47,     0,    51,   185,   186,     0,   229,     0,   239,
     189,   190,   188,   187,   158,     0,   255,   222,   223,   224,
     225,   226,     8,     7,     6,     0,    18,     0,     0,   239,
       0,     0,   251,    57,     0,   253,     0,     0,   233,     0,
       0,     0,   126,   233,     0,     0,     0,     0,     0,     0,
      71,    72,     0,   138,   253,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   212,   145,   170,    16,    16,
     136,     0,     0,     0,    27,    26,     0,     0,     0,   256,
     150,   151,   164,   256,   234,   182,   181,   155,   153,   165,
       0,     0,     0,   102,     0,    91,    98,   175,     0,     0,
     175,     0,     0,     0,     0,    23,    25,     0,     0,   242,
      29,     0,     0,    40,     0,    48,     0,     0,   231,     0,
     257,     0,   233,   240,   139,   166,   256,   157,    21,    20,
      63,   218,     0,   175,   216,   217,   260,   249,   255,     0,
       0,     0,     0,     0,   142,    24,     0,     0,   131,     0,
     130,   183,     0,     0,     0,    69,    66,    70,    65,    68,
      67,    73,     0,   199,   200,   197,   201,   198,   193,   196,
     192,   195,   194,   208,   206,   207,   210,   209,   205,   204,
     202,   203,   211,     0,    15,    14,     0,     0,   127,     0,
     146,     0,   148,   236,   237,     0,     0,     0,     0,     0,
     110,     0,   107,    99,     0,   239,     0,    86,   128,     0,
       0,     0,   134,     0,     0,     0,   175,     0,    30,    31,
      32,     0,    34,    37,    38,    39,     0,   255,    42,     0,
       0,     0,   124,   258,   230,   123,   159,     0,    61,    64,
     219,    52,   256,   252,   254,   244,    58,    59,    55,   177,
     133,   227,   125,     0,   238,    56,   214,    78,    91,    28,
      79,    95,   105,   100,     0,   104,    92,     0,   109,   176,
     175,    85,     0,    91,    76,   118,   117,     0,     0,   113,
     116,     0,     0,    33,    29,    44,   256,    41,     0,     0,
     120,   232,     0,    62,   250,     0,     0,     0,   245,   246,
     228,    77,    89,     0,    91,     0,   103,   108,   173,    91,
      88,     0,    22,     0,    93,    35,    43,    45,     0,     0,
      60,   244,   243,   248,     0,     0,     0,    96,   106,   107,
     101,     0,   172,    87,     0,     0,    93,     0,    75,    49,
       0,   247,     0,    16,    80,    81,   174,   115,     0,    74,
      94,     0,    16,    90,   114,     0,    97,    50
  };

  const short int
  parser::yypgoto_[] =
  {
    -343,  -343,  -343,  -343,  -343,   -14,     1,   102,    33,    10,
      -5,  -343,    58,  -343,   358,   474,  -343,  -343,     8,  -343,
      92,   401,   121,  -343,    36,  -343,  -342,    25,  -343,  -343,
    -343,    38,  -343,   272,  -343,    39,  -343,  -343,   572,    -2,
      -4,  -343,  -343,  -343,   286,  -343,  -343,  -343,   490,  -343,
    -343,  -343,  -343,  -343,  -120,  -343,  -229,   164,  -343,  -343,
    -343,   488,   -53,  -343,  -343,   -12,   255,  -343,  -137,  -165,
    -343,  -343,    51,   133,  -343,  -343,  -113,  -102,  -343,  -343
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     4,     5,    64,    85,    86,    87,    66,   244,    67,
      68,   361,   362,   363,   251,    69,   366,   367,   458,   142,
     101,   175,   378,   379,   102,   466,   342,   478,   442,   467,
     235,   404,   339,   236,   444,   408,   417,    72,    73,   103,
      75,    76,    77,    78,   104,   105,   106,    79,   111,    80,
     155,   156,    81,    82,   238,   472,   346,    83,   323,    84,
     205,   117,   158,   259,   260,   294,   108,   113,   193,   264,
     134,   438,   439,   277,   278,   279,   176,   220,   374,   280
  };

  const short int
  parser::yytable_[] =
  {
      74,    74,    97,    65,   273,   107,   224,   241,   159,   160,
     161,   349,   263,   183,    98,   177,   173,   168,   248,    88,
     122,    89,   124,   172,  -260,   178,    92,   168,   129,   178,
     179,    74,   263,   167,   179,   233,   167,   138,    71,    71,
      93,   216,   234,   132,   381,    36,   249,   168,    94,   174,
     168,   173,    71,   276,   267,   154,   441,    36,    95,   190,
     191,   192,   283,   344,    90,    91,  -259,   182,   118,    71,
     128,   450,   194,   135,   135,   167,   162,   135,   135,   135,
     212,   302,   128,   218,   174,   167,  -137,   183,   210,   168,
     218,  -137,    74,   218,    70,    70,   194,   119,   415,   168,
     180,   226,   469,   215,   180,   416,   170,   473,    70,    16,
     345,   217,   147,   218,   148,   233,   218,   208,   209,   163,
     164,   341,  -236,   121,   167,    70,   157,   421,   357,    74,
      71,   167,   327,   190,   191,   192,   170,   171,   168,   170,
     171,   245,   435,   208,   209,   168,   167,   464,   272,   436,
     326,   125,   262,   465,   172,   218,   126,   335,   282,   437,
     168,  -137,   127,    74,    74,   218,   286,    71,   167,   282,
     130,   284,   336,   328,   250,    74,   383,   290,   170,   171,
     410,   448,   168,   -36,   139,   183,    70,   285,   170,   171,
     301,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,    71,    71,   370,   218,   140,    74,    74,   263,   324,
     325,   218,   131,    71,   329,    16,   291,   181,   141,   150,
     151,   152,   153,    70,   165,   166,   218,   170,   171,   340,
     133,   190,   191,   192,   170,   171,   391,   146,   167,   124,
     393,   358,   359,   360,    71,    71,   364,   365,   169,   170,
     171,   368,   168,     1,   149,     2,     3,    70,    70,   208,
     209,   206,   376,   287,   337,   427,   167,   268,   269,    70,
      36,   170,   171,   112,   112,   214,   167,   207,   388,   167,
     168,   201,   172,   203,   204,   390,   219,   221,   223,   222,
     168,   228,   348,   168,   167,   128,   167,   395,   225,   227,
      70,    70,   351,   229,   253,   354,   254,   256,   168,   167,
     168,   262,   167,   255,   265,   288,   272,   266,   218,   396,
     355,   281,   371,   168,    74,    74,   168,   200,   201,   202,
     203,   204,   289,    74,   405,    74,   397,   398,   375,    74,
     338,   170,   171,   347,   353,   400,   218,   402,    74,    74,
     350,   406,    74,    74,   369,   377,   218,   372,   373,   169,
     413,   414,    71,    71,   419,   420,   430,   165,   344,   170,
     171,    71,   382,    71,   218,   384,   218,    71,   389,   170,
     171,   394,   170,   171,   399,   385,    71,    71,   386,   218,
      71,    71,   218,   387,   401,   407,   411,   170,   171,   170,
     171,   412,   447,   198,   199,   200,   201,   202,   203,   204,
      74,   424,   170,   171,   422,   170,   171,   425,    70,    70,
      74,   426,   449,   167,   456,   428,   167,    70,   429,    70,
     440,   432,   454,    70,   341,   377,   443,   168,   468,   445,
     168,   446,    70,    70,   453,   167,    70,    70,    71,   392,
      74,    74,   457,   167,   459,   463,   471,   167,    71,   168,
     167,   477,   245,   476,   436,   479,   480,   168,   483,   493,
     484,   168,    74,   491,   168,    74,   488,   492,   496,   474,
     497,    74,   455,   452,   487,   475,    74,   490,    71,    71,
      74,   100,   110,   114,   116,   252,   213,   116,   494,   495,
     433,   489,   482,   218,    70,   332,   218,   343,   485,   115,
      71,   120,   481,    71,    70,   434,     0,     0,     0,    71,
       0,     0,     0,     0,    71,   218,   170,   171,    71,   170,
     171,     0,     0,   218,     0,     0,     0,   218,     0,     0,
     218,     0,     0,     0,    70,    70,     0,     0,   170,   171,
       0,     0,     0,     0,     0,     0,   170,   171,     0,     0,
     170,   171,     0,   170,   171,     0,    70,   211,     0,    70,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
      70,   195,   196,   197,    70,   198,   199,   200,   201,   202,
     203,   204,   230,   231,     0,   232,     0,     0,   123,   237,
     239,   240,     0,     0,   242,   246,     0,   247,     0,   136,
     137,     0,     0,   143,   144,   145,     0,     0,     0,     0,
     257,   258,   261,     0,     0,   183,     0,   123,   123,   123,
     123,   123,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   270,   271,     0,   274,   275,     0,     0,     0,     0,
       0,     0,   114,     0,     0,   292,     0,   114,   295,   296,
     297,   298,   299,   300,   123,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   195,     0,   197,     0,   198,   199,
     200,   201,   202,   203,   204,     0,     0,     0,     0,     0,
       0,   330,     0,   331,     0,     0,     0,   333,     0,   195,
       0,     0,   334,   198,   199,   200,   201,   202,   203,   204,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
       0,     0,   356,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   114,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   380,     0,     0,     0,
       0,     0,   123,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   109,     0,     0,     0,
       7,     0,     0,     0,     0,     9,    10,     0,     0,     0,
       0,     0,     0,    13,     0,     0,    15,    16,    17,    18,
      19,     0,   403,     0,     0,   123,     0,  -152,   409,     0,
       0,     0,     0,     0,    26,     0,    28,   418,     0,     0,
      30,     0,     0,     0,     0,   423,     0,    36,   123,     0,
       0,     0,     0,     0,    37,    38,     0,   431,     0,     0,
       0,     0,     0,     0,     0,    42,    43,     0,    44,    45,
      46,    47,    48,    49,    50,    51,     0,    52,     0,    53,
      54,    55,     0,     0,     0,    56,     0,     0,    57,     0,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      59,   451,     0,     0,     0,   123,    60,    61,    62,    63,
       0,     0,     0,     0,     0,     0,   460,     0,     0,   461,
     462,     0,     0,     0,     0,     0,     0,     0,     0,   470,
       0,     0,     0,     0,     0,   -16,     6,     0,     0,     0,
       7,     0,     8,     0,     0,     9,    10,    11,   403,     0,
       0,     0,    12,    13,    14,   486,    15,    16,    17,    18,
      19,     0,     0,     0,     0,     0,    20,     0,    21,    22,
      23,     0,    24,    25,    26,    27,    28,    29,   -16,   -16,
      30,     0,    31,    32,    33,    34,    35,    36,     0,     0,
       0,     0,     0,     0,    37,    38,    39,    40,    41,     0,
       0,     0,     0,     0,     0,    42,    43,     0,    44,    45,
      46,    47,    48,    49,    50,    51,     0,    52,     0,    53,
      54,    55,     0,     0,     0,    56,     0,     0,    57,     0,
      58,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      59,     0,     0,     0,     0,     0,    60,    61,    62,    63,
      96,     0,     0,     0,     7,     0,     8,     0,     0,     9,
      10,    11,     0,     0,     0,     0,    12,    13,    14,     0,
      15,    16,    17,    18,    19,     0,     0,     0,   -16,     0,
      20,     0,    21,    22,    23,     0,    24,    25,    26,    27,
      28,    29,   -16,   -16,    30,     0,    31,    32,    33,    34,
      35,    36,     0,     0,     0,     0,     0,     0,    37,    38,
      39,    40,    41,     0,     0,     0,     0,     0,     0,    42,
      43,     0,    44,    45,    46,    47,    48,    49,    50,    51,
       0,    52,     7,    53,    54,    55,     0,     9,    10,    56,
       0,     0,    57,     0,    58,    13,     0,     0,    15,    16,
      17,    18,    19,     0,    59,     0,     0,     0,     0,     0,
      60,    61,    62,    63,     0,     0,    26,     0,    28,     0,
       0,     0,    30,     0,     0,     0,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,    37,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    42,    43,     0,
      44,    45,    46,    47,    48,    49,    50,    51,     0,    52,
       0,    53,    54,    55,     0,     0,     0,    56,     0,     0,
      57,     0,    58,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    59,     0,     0,     0,     0,     0,    60,    61,
      62,    63,   293,     7,     0,     8,     0,     0,     9,    10,
      11,     0,     0,     0,     0,    12,    13,    14,     0,    15,
      16,    17,    18,    19,     0,     0,     0,     0,     0,    20,
       0,    21,    22,    23,     0,    24,    25,    26,    27,    28,
      29,     0,     0,    30,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,    37,    38,    39,
      40,    41,     0,     0,     0,     0,     0,     0,    42,    43,
       0,    44,    45,    46,    47,    48,    49,    50,    51,     0,
      52,     0,    53,    54,    55,     0,     0,     0,    56,     0,
       0,    57,     0,    58,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    59,     0,     0,     0,     0,     0,    60,
      61,    62,    63,     7,     0,     8,     0,     0,     9,    10,
      11,     0,     0,     0,     0,    12,    13,    14,     0,    15,
      16,    17,    18,    19,     0,     0,     0,     0,     0,    20,
       0,    21,    22,    23,     0,    24,    25,   243,    27,    28,
      29,     0,     0,    30,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,     0,    37,    38,    39,
      40,    41,     0,     0,     0,     0,     0,     0,    42,    43,
       0,    44,    45,    46,    47,    48,    49,    50,    51,     0,
      52,     7,    53,    54,    55,     0,     9,    10,    56,     0,
       0,    57,     0,    58,    13,     0,     0,    15,    16,    17,
      18,    19,     0,    59,     0,     0,     0,     0,     0,    60,
      61,    62,    63,     0,     0,    26,     0,    28,     0,     0,
       0,    30,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    37,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    42,    43,     0,    44,
      45,    46,    47,    48,    49,    50,    51,    99,    52,     7,
      53,    54,    55,     0,     9,    10,    56,     0,     0,    57,
       0,    58,    13,     0,     0,    15,    16,    17,    18,    19,
       0,    59,     0,     0,     0,     0,     0,    60,    61,    62,
      63,     0,     0,    26,     0,    28,     0,     0,     0,    30,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,    37,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    42,    43,     0,    44,    45,    46,
      47,    48,    49,    50,    51,     0,    52,     7,    53,    54,
      55,     0,     9,    10,    56,     0,     0,    57,     0,    58,
      13,     0,     0,    15,    16,    17,    18,    19,     0,    59,
       0,     0,     0,     0,     0,    60,    61,    62,    63,     0,
       0,    26,     0,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,    37,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    42,    43,     0,    44,    45,    46,    47,    48,
      49,    50,    51,     0,    52,     7,    53,    54,    55,     0,
       9,    10,    56,     0,     0,    57,     0,    58,    13,     0,
       0,    15,    16,    17,    18,    19,     0,    59,     0,     0,
       0,     0,     0,    60,    61,    62,    63,     0,     0,    26,
       0,    28,     0,     0,     0,    30,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      42,    43,     0,    44,    45,    46,    47,    48,    49,    50,
      51,     0,    52,     7,    53,    54,    55,     0,     9,    10,
      56,     0,     0,    57,     0,    58,    13,     0,     0,    15,
      16,    17,    18,    19,     0,    59,     0,     0,     0,     0,
       0,     0,    61,    62,    63,     0,     0,    26,     0,    28,
       0,     0,     0,    30,     0,     0,     0,     0,     0,     0,
     352,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    42,    43,
       0,    44,    45,    46,    47,    48,    49,    50,    51,     0,
      52,     7,    53,    54,    55,     0,     9,     0,    56,     0,
       0,    57,     0,    58,    13,     0,     0,    15,    16,    17,
      18,    19,     0,    59,     0,     0,     0,     0,     0,     0,
      61,    62,    63,     0,     0,     0,     0,    28,     0,     0,
       0,    30,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    42,    43,     0,    44,
      45,    46,    47,   128,    49,    50,    51,     0,    52,     7,
      53,    54,    55,     0,     9,     0,    56,     0,     0,    57,
       0,    58,    13,     0,     0,    15,    16,    17,    18,    19,
       0,    59,     0,     0,     0,     0,     0,     0,    61,    62,
      63,     0,     0,     0,     0,    28,     0,     0,     0,    30,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    44,    45,    46,
      47,   128,    49,    50,    51,     0,    52,     0,    53,    54,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    59,
       0,     0,     0,     0,     0,     0,    61,    62,    63
  };

  const short int
  parser::yycheck_[] =
  {
       2,     3,    16,     2,   169,    17,   108,   127,    61,    62,
      63,   240,   149,    24,    16,    13,    52,    20,    24,     0,
      25,     0,    26,    24,    20,    23,    39,    20,    30,    23,
      28,    33,   169,     6,    28,     9,     6,    39,     2,     3,
      24,    36,    16,    33,   273,    52,    52,    20,    24,    85,
      20,    52,    16,    39,   156,    59,   398,    52,    24,    70,
      71,    72,   175,    36,    43,    44,    52,    72,    24,    33,
      77,   413,    74,    37,    38,     6,     0,    41,    42,    43,
      94,   194,    77,    86,    85,     6,    23,    24,    92,    20,
      86,    28,    94,    86,     2,     3,    98,    24,    13,    20,
      98,    32,   444,    29,    98,    20,   109,   449,    16,    22,
      83,    81,    22,    86,    24,     9,    86,    43,    44,    43,
      44,    15,    43,    24,     6,    33,    80,   356,   248,   131,
      94,     6,    32,    70,    71,    72,   109,   110,    20,   109,
     110,   131,     6,    43,    44,    20,     6,     8,    20,    13,
      32,    24,    24,    14,    24,    86,    24,    32,    28,    23,
      20,    98,    24,   165,   166,    86,   178,   131,     6,    28,
      24,   176,    32,    32,    13,   177,   278,   179,   109,   110,
     345,   410,    20,    22,    18,    24,    94,   177,   109,   110,
     192,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   165,   166,   256,    86,    39,   208,   209,   345,   208,
     209,    86,    24,   177,   216,    22,   180,    24,    52,    55,
      56,    57,    58,   131,    45,    46,    86,   109,   110,   234,
      24,    70,    71,    72,   109,   110,   289,    24,     6,   243,
     293,    56,    57,    58,   208,   209,   251,   252,    86,   109,
     110,   253,    20,     1,    52,     3,     4,   165,   166,    43,
      44,    79,   266,    45,    32,   367,     6,   165,   166,   177,
      52,   109,   110,    18,    19,    29,     6,    82,   283,     6,
      20,    94,    24,    96,    97,   287,    43,    30,    43,    30,
      20,    43,    32,    20,     6,    77,     6,   302,    32,    32,
     208,   209,    32,    26,    22,    32,    52,    24,    20,     6,
      20,    24,     6,    52,   103,    97,    20,    43,    86,   323,
      32,    28,    32,    20,   326,   327,    20,    93,    94,    95,
      96,    97,   114,   335,   339,   337,   326,   327,    32,   341,
      24,   109,   110,    32,    44,   335,    86,   337,   350,   351,
      32,   341,   354,   355,    28,    52,    86,    29,    44,    86,
     350,   351,   326,   327,   354,   355,   371,    45,    36,   109,
     110,   335,    43,   337,    86,    32,    86,   341,    30,   109,
     110,    32,   109,   110,    29,    52,   350,   351,    52,    86,
     354,   355,    86,    52,    22,    16,    32,   109,   110,   109,
     110,    32,   407,    91,    92,    93,    94,    95,    96,    97,
     412,    43,   109,   110,    32,   109,   110,    29,   326,   327,
     422,    43,   412,     6,   426,    52,     6,   335,    32,   337,
      32,    13,   422,   341,    15,    52,     9,    20,   443,    19,
      20,    32,   350,   351,    32,     6,   354,   355,   412,    32,
     452,   453,    52,     6,    52,    30,    19,     6,   422,    20,
       6,    27,   452,   453,    13,    52,    28,    20,    13,   483,
      29,    20,   474,    52,    20,   477,    32,    13,   492,    32,
      52,   483,   424,    44,   474,   452,   488,   477,   452,   453,
     492,    17,    18,    19,    20,   137,    95,    23,   488,   491,
     379,   476,   464,    86,   412,   219,    86,   235,   469,    19,
     474,    23,   461,   477,   422,   382,    -1,    -1,    -1,   483,
      -1,    -1,    -1,    -1,   488,    86,   109,   110,   492,   109,
     110,    -1,    -1,    86,    -1,    -1,    -1,    86,    -1,    -1,
      86,    -1,    -1,    -1,   452,   453,    -1,    -1,   109,   110,
      -1,    -1,    -1,    -1,    -1,    -1,   109,   110,    -1,    -1,
     109,   110,    -1,   109,   110,    -1,   474,    93,    -1,   477,
      -1,    -1,    -1,    -1,    -1,   483,    -1,    -1,    -1,    -1,
     488,    87,    88,    89,   492,    91,    92,    93,    94,    95,
      96,    97,   118,   119,    -1,   121,    -1,    -1,    26,   125,
     126,   127,    -1,    -1,   130,   131,    -1,   133,    -1,    37,
      38,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
     146,   147,   148,    -1,    -1,    24,    -1,    55,    56,    57,
      58,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   167,   168,    -1,   170,   171,    -1,    -1,    -1,    -1,
      -1,    -1,   178,    -1,    -1,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,    92,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    87,    -1,    89,    -1,    91,    92,
      93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,
      -1,   217,    -1,   219,    -1,    -1,    -1,   223,    -1,    87,
      -1,    -1,   228,    91,    92,    93,    94,    95,    96,    97,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
      -1,    -1,   248,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   262,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   272,    -1,    -1,    -1,
      -1,    -1,   180,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
       5,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    18,    -1,    -1,    21,    22,    23,    24,
      25,    -1,   338,    -1,    -1,   243,    -1,    32,   344,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    41,   353,    -1,    -1,
      45,    -1,    -1,    -1,    -1,   361,    -1,    52,   266,    -1,
      -1,    -1,    -1,    -1,    59,    60,    -1,   373,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    71,    -1,    73,    74,
      75,    76,    77,    78,    79,    80,    -1,    82,    -1,    84,
      85,    86,    -1,    -1,    -1,    90,    -1,    -1,    93,    -1,
      95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     105,   417,    -1,    -1,    -1,   323,   111,   112,   113,   114,
      -1,    -1,    -1,    -1,    -1,    -1,   432,    -1,    -1,   435,
     436,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   445,
      -1,    -1,    -1,    -1,    -1,     0,     1,    -1,    -1,    -1,
       5,    -1,     7,    -1,    -1,    10,    11,    12,   464,    -1,
      -1,    -1,    17,    18,    19,   471,    21,    22,    23,    24,
      25,    -1,    -1,    -1,    -1,    -1,    31,    -1,    33,    34,
      35,    -1,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    -1,    47,    48,    49,    50,    51,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    60,    61,    62,    63,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    71,    -1,    73,    74,
      75,    76,    77,    78,    79,    80,    -1,    82,    -1,    84,
      85,    86,    -1,    -1,    -1,    90,    -1,    -1,    93,    -1,
      95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     105,    -1,    -1,    -1,    -1,    -1,   111,   112,   113,   114,
       1,    -1,    -1,    -1,     5,    -1,     7,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    29,    -1,
      31,    -1,    33,    34,    35,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    -1,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
      61,    62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      71,    -1,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    82,     5,    84,    85,    86,    -1,    10,    11,    90,
      -1,    -1,    93,    -1,    95,    18,    -1,    -1,    21,    22,
      23,    24,    25,    -1,   105,    -1,    -1,    -1,    -1,    -1,
     111,   112,   113,   114,    -1,    -1,    39,    -1,    41,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    -1,
      73,    74,    75,    76,    77,    78,    79,    80,    -1,    82,
      -1,    84,    85,    86,    -1,    -1,    -1,    90,    -1,    -1,
      93,    -1,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,   111,   112,
     113,   114,   115,     5,    -1,     7,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    33,    34,    35,    -1,    37,    38,    39,    40,    41,
      42,    -1,    -1,    45,    -1,    47,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    -1,    -1,    -1,    90,    -1,
      -1,    93,    -1,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,   111,
     112,   113,   114,     5,    -1,     7,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    33,    34,    35,    -1,    37,    38,    39,    40,    41,
      42,    -1,    -1,    45,    -1,    47,    48,    49,    50,    51,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,     5,    84,    85,    86,    -1,    10,    11,    90,    -1,
      -1,    93,    -1,    95,    18,    -1,    -1,    21,    22,    23,
      24,    25,    -1,   105,    -1,    -1,    -1,    -1,    -1,   111,
     112,   113,   114,    -1,    -1,    39,    -1,    41,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    59,    60,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     5,
      84,    85,    86,    -1,    10,    11,    90,    -1,    -1,    93,
      -1,    95,    18,    -1,    -1,    21,    22,    23,    24,    25,
      -1,   105,    -1,    -1,    -1,    -1,    -1,   111,   112,   113,
     114,    -1,    -1,    39,    -1,    41,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,     5,    84,    85,
      86,    -1,    10,    11,    90,    -1,    -1,    93,    -1,    95,
      18,    -1,    -1,    21,    22,    23,    24,    25,    -1,   105,
      -1,    -1,    -1,    -1,    -1,   111,   112,   113,   114,    -1,
      -1,    39,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,     5,    84,    85,    86,    -1,
      10,    11,    90,    -1,    -1,    93,    -1,    95,    18,    -1,
      -1,    21,    22,    23,    24,    25,    -1,   105,    -1,    -1,
      -1,    -1,    -1,   111,   112,   113,   114,    -1,    -1,    39,
      -1,    41,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    71,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,     5,    84,    85,    86,    -1,    10,    11,
      90,    -1,    -1,    93,    -1,    95,    18,    -1,    -1,    21,
      22,    23,    24,    25,    -1,   105,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,    -1,    -1,    39,    -1,    41,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,     5,    84,    85,    86,    -1,    10,    -1,    90,    -1,
      -1,    93,    -1,    95,    18,    -1,    -1,    21,    22,    23,
      24,    25,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,     5,
      84,    85,    86,    -1,    10,    -1,    90,    -1,    -1,    93,
      -1,    95,    18,    -1,    -1,    21,    22,    23,    24,    25,
      -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,     1,     3,     4,   117,   118,     1,     5,     7,    10,
      11,    12,    17,    18,    19,    21,    22,    23,    24,    25,
      31,    33,    34,    35,    37,    38,    39,    40,    41,    42,
      45,    47,    48,    49,    50,    51,    52,    59,    60,    61,
      62,    63,    70,    71,    73,    74,    75,    76,    77,    78,
      79,    80,    82,    84,    85,    86,    90,    93,    95,   105,
     111,   112,   113,   114,   119,   122,   123,   125,   126,   131,
     136,   140,   153,   154,   155,   156,   157,   158,   159,   163,
     165,   168,   169,   173,   175,   120,   121,   122,     0,     0,
      43,    44,    39,    24,    24,    24,     1,   121,   155,    81,
     131,   136,   140,   155,   160,   161,   162,   181,   182,     1,
     131,   164,   182,   183,   131,   164,   131,   177,    24,    24,
     177,    24,   126,   154,   156,    24,    24,    24,    77,   155,
      24,    24,   125,    24,   186,   140,   154,   154,   155,    18,
      39,    52,   135,   154,   154,   154,    24,    22,    24,    52,
     173,   173,   173,   173,   156,   166,   167,    80,   178,   178,
     178,   178,     0,    43,    44,    45,    46,     6,    20,    86,
     109,   110,    24,    52,    85,   137,   192,    13,    23,    28,
      98,    24,   126,    24,    64,    65,    66,    67,    68,    69,
      70,    71,    72,   184,   155,    87,    88,    89,    91,    92,
      93,    94,    95,    96,    97,   176,    79,    82,    43,    44,
     156,   131,   121,   137,    29,    29,    36,    81,    86,    43,
     193,    30,    30,    43,   193,    32,    32,    32,    43,    26,
     131,   131,   131,     9,    16,   146,   149,   131,   170,   131,
     131,   170,   131,    39,   124,   125,   131,   131,    24,    52,
      13,   130,   130,    22,    52,    52,    24,   131,   131,   179,
     180,   131,    24,   184,   185,   103,    43,   193,   123,   123,
     131,   131,    20,   185,   131,   131,    39,   189,   190,   191,
     195,    28,    28,   192,   126,   125,   181,    45,    97,   114,
     155,   140,   131,   115,   181,   131,   131,   131,   131,   131,
     131,   155,   192,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   174,   122,   122,    32,    32,    32,   155,
     131,   131,   160,   131,   131,    32,    32,    32,    24,   148,
     126,    15,   142,   149,    36,    83,   172,    32,    32,   172,
      32,    32,    52,    44,    32,    32,   131,   170,    56,    57,
      58,   127,   128,   129,   126,   126,   132,   133,   155,    28,
     178,    32,    29,    44,   194,    32,   156,    52,   138,   139,
     131,   172,    43,   193,    32,    52,    52,    52,   126,    30,
     155,   178,    32,   178,    32,   126,   156,   125,   125,    29,
     125,    22,   125,   131,   147,   126,   125,    16,   151,   131,
     185,    32,    32,   125,   125,    13,    20,   152,   131,   125,
     125,   172,    32,   131,    43,    29,    43,   193,    52,    32,
     126,   131,    13,   138,   189,     6,    13,    23,   187,   188,
      32,   142,   144,     9,   150,    19,    32,   126,   172,   125,
     142,   131,    44,    32,   125,   128,   155,    52,   134,    52,
     131,   131,   131,    30,     8,    14,   141,   145,   126,   142,
     131,    19,   171,   142,    32,   124,   125,    27,   143,    52,
      28,   188,   147,    13,    29,   151,   131,   125,    32,   143,
     125,    52,    13,   121,   125,   134,   121,    52
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,   116,   117,   118,   118,   118,   119,   119,   119,   119,
     119,   119,   120,   121,   121,   121,   122,   122,   122,   123,
     123,   123,   124,   124,   125,   125,   126,   126,   126,   127,
     127,   127,   127,   128,   129,   129,   130,   130,   131,   131,
     132,   132,   133,   133,   125,   134,   135,   135,   125,   125,
     125,   125,   125,   136,   136,   125,   125,   137,   137,   137,
     138,   139,   139,   131,   131,   131,   131,   131,   131,   131,
     131,   140,   140,   140,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   125,   125,   141,
     141,   142,   142,   143,   143,   144,   144,   145,   146,   146,
     147,   147,   148,   148,   149,   150,   150,   151,   151,   125,
     125,   125,   125,   125,   125,   125,   125,   152,   152,   140,
     140,   153,   153,   140,   140,   140,   140,   140,   140,   154,
     154,   154,   140,   140,   155,   156,   156,   140,   140,   157,
     156,   155,   140,   158,   159,   159,   160,   161,   161,   162,
     162,   163,   164,   164,   164,   165,   166,   166,   167,   167,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   169,
     169,   168,   170,   171,   171,   172,   172,   154,   168,   168,
     140,   140,   140,   140,   173,   173,   173,   173,   173,   173,
     173,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   174,   174,   174,   174,   174,   174,   174,   174,
     174,   174,   175,   176,   176,   131,   131,   131,   131,   131,
     177,   177,   178,   131,   140,   154,   154,   154,   140,   179,
     179,   180,   180,   181,   181,   182,   182,   183,   184,   185,
     185,   186,   186,   187,   188,   188,   189,   189,   189,   190,
     190,   191,   191,   192,   192,   193,   193,   194,   194,   195,
     195
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     1,     3,     3,     0,     1,     2,     1,
       3,     3,     0,     1,     3,     1,     3,     3,     5,     0,
       1,     1,     1,     2,     1,     3,     0,     2,     4,     4,
       0,     2,     1,     3,     5,     1,     1,     1,     3,     7,
      10,     2,     4,     1,     1,     4,     4,     1,     3,     3,
       3,     1,     2,     3,     4,     3,     3,     3,     3,     3,
       3,     2,     2,     3,     8,     7,     5,     6,     5,     5,
       8,     8,     2,     1,     1,     5,     4,     7,     6,     0,
       3,     0,     2,     0,     2,     0,     2,     4,     1,     2,
       1,     3,     0,     3,     3,     0,     2,     0,     2,     5,
       4,     2,     2,     5,     9,     8,     5,     1,     1,     1,
       5,     1,     1,     4,     4,     4,     2,     4,     4,     1,
       3,     3,     2,     4,     1,     2,     3,     1,     2,     3,
       1,     1,     3,     1,     1,     2,     3,     1,     3,     1,
       2,     3,     0,     2,     1,     3,     0,     2,     1,     3,
       1,     1,     1,     1,     3,     3,     3,     1,     1,     1,
       2,     1,     5,     0,     2,     0,     2,     4,     1,     1,
       1,     3,     3,     3,     1,     2,     2,     2,     2,     2,
       2,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     3,     1,     3,     3,     3,     4,
       0,     1,     1,     2,     2,     2,     2,     4,     5,     0,
       2,     1,     3,     0,     2,     1,     3,     3,     3,     0,
       1,     0,     2,     2,     0,     1,     3,     5,     4,     1,
       3,     0,     2,     0,     3,     0,     1,     0,     1,     0,
       1
  };


#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const parser::yytname_[] =
  {
  "\"end of command\"", "error", "$undefined", "MODE_EXP", "MODE_EXPS",
  "\"__HERE__\"", "\"=\"", "\"break\"", "\"case\"", "\"catch\"",
  "\"closure\"", "\"const\"", "\"continue\"", "\":\"", "\"default\"",
  "\"else\"", "\"finally\"", "\"freezeif\"", "\"function\"", "\"if\"",
  "\"in\"", "\"isdef\"", "\"{\"", "\"[\"", "\"(\"", "\"[<\"", "\">]\"",
  "\"onleave\"", "\".\"", "\"}\"", "\"]\"", "\"return\"", "\")\"",
  "\"stopif\"", "\"switch\"", "\"throw\"", "\"~\"", "\"timeout\"",
  "\"try\"", "\"var\"", "\"waituntil\"", "\"watch\"", "\"whenever\"",
  "\",\"", "\";\"", "\"&\"", "\"|\"", "\"every\"", "\"for\"", "\"loop\"",
  "\"while\"", "\"at\"", "\"identifier\"", "ASSIGN", "EMPTY", "UNARY",
  "\"private\"", "\"protected\"", "\"public\"", "\"class\"", "\"package\"",
  "\"enum\"", "\"external\"", "\"import\"", "\"^=\"", "\"-=\"", "\"%=\"",
  "\"+=\"", "\"/=\"", "\"*=\"", "\"--\"", "\"++\"", "\"->\"", "\"do\"",
  "\"assert\"", "\"detach\"", "\"disown\"", "\"new\"", "\"angle\"",
  "\"duration\"", "\"float\"", "\"=>\"", "\"string\"", "\"?\"", "\"call\"",
  "\"this\"", "\"!\"", "\"bitand\"", "\"bitor\"", "\"^\"", "\"compl\"",
  "\">>\"", "\"<<\"", "\"-\"", "\"%\"", "\"+\"", "\"/\"", "\"*\"",
  "\"**\"", "\"=~=\"", "\"==\"", "\"===\"", "\">=\"", "\">\"", "\"<=\"",
  "\"<\"", "\"!=\"", "\"!==\"", "\"~=\"", "\"&&\"", "\"||\"",
  "\"%unscope:\"", "\"%exp:\"", "\"%lvalue:\"", "\"%id:\"", "\"%exps:\"",
  "$accept", "start", "root", "root_exp", "root_exps", "stmts",
  "cstmt.opt", "cstmt", "stmt.opt", "stmt", "block", "visibility", "proto",
  "protos.1", "protos", "exp", "id.0", "id.1", "from", "event_or_function",
  "routine", "k1_id", "modifier", "modifiers", "primary-exp",
  "default.opt", "else.opt", "onleave.opt", "cases", "case", "catches.1",
  "match", "match.opt", "catch", "catch.opt", "finally.opt", "in_or_colon",
  "detach", "lvalue", "id", "bitor-exp", "new", "float-exp", "duration",
  "assoc", "assocs.1", "assocs", "dictionary", "tuple.exps", "tuple",
  "bitor-exps", "bitor-exps.1", "literal-exp", "string", "event_match",
  "guard.opt", "tilda.opt", "unary-exp", "rel-op", "rel-exp", "rel-ops",
  "exp.opt", "unsigned", "claims", "claims.1", "exps", "exps.1", "exps.2",
  "args", "args.opt", "identifiers", "typespec", "typespec.opt", "formal",
  "formals.1", "formals.0", "formals", "comma.opt", "semi.opt", "var.opt", 0
  };
#endif

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,   315,   315,   330,   331,   332,   338,   339,   340,   341,
     342,   343,   348,   360,   361,   362,   370,   371,   372,   377,
     378,   379,   389,   390,   395,   406,   410,   411,   415,   428,
     430,   431,   432,   437,   443,   444,   449,   450,   455,   464,
     480,   481,   485,   486,   491,   502,   511,   515,   525,   530,
     535,   548,   589,   602,   603,   609,   615,   662,   663,   664,
     675,   684,   688,   705,   709,   725,   726,   727,   728,   729,
     730,   738,   739,   750,   761,   765,   769,   773,   777,   781,
     785,   789,   794,   798,   802,   806,   810,   814,   818,   835,
     836,   841,   842,   848,   849,   859,   860,   866,   875,   876,
     881,   882,   885,   886,   890,   897,   898,   904,   905,   909,
     913,   917,   946,   950,   954,   958,   962,   968,   968,   978,
     979,   991,   992,   996,   997,   998,   999,  1000,  1001,  1011,
    1012,  1013,  1017,  1018,  1022,  1026,  1030,  1037,  1041,  1054,
    1066,  1071,  1081,  1098,  1108,  1109,  1122,  1130,  1134,  1142,
    1143,  1148,  1159,  1160,  1161,  1165,  1179,  1180,  1184,  1185,
    1193,  1194,  1195,  1196,  1197,  1198,  1199,  1200,  1201,  1207,
    1208,  1216,  1226,  1234,  1235,  1240,  1241,  1250,  1266,  1267,
    1271,  1272,  1273,  1274,  1279,  1280,  1281,  1282,  1283,  1284,
    1285,  1310,  1311,  1312,  1313,  1314,  1315,  1316,  1317,  1318,
    1319,  1320,  1346,  1347,  1348,  1349,  1350,  1351,  1352,  1353,
    1354,  1355,  1359,  1364,  1365,  1379,  1380,  1381,  1386,  1387,
    1391,  1392,  1403,  1411,  1423,  1431,  1439,  1443,  1451,  1468,
    1469,  1473,  1474,  1480,  1481,  1485,  1486,  1490,  1495,  1499,
    1500,  1510,  1511,  1516,  1521,  1522,  1527,  1528,  1529,  1535,
    1536,  1541,  1542,  1547,  1548,  1551,  1551,  1552,  1552,  1553,
    1553
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
	 i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG



} // yy
/* Line 1131 of lalr1.cc  */
#line 5630 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
/* Line 1132 of lalr1.cc  */
#line 1555 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"


// The error function that 'bison' calls.
void
yy::parser::error(const location_type& l, const std::string& m)
{
  GD_CATEGORY(Urbi.Error);
  GD_FINFO_TRACE("%s: %s", l, m);
  up.error(l, m);
}

// Local Variables:
// mode: c++
// End:
