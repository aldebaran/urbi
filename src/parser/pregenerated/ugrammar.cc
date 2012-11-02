/* A Bison parser, made by GNU Bison 2.3b.655-27c2.  */

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
#line 38 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"

#include "ugrammar.hh"

/* User implementation prologue.  */

/* Line 374 of lalr1.cc  */
#line 45 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
/* Unqualified %code blocks.  */
/* Line 375 of lalr1.cc  */
#line 62 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"

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
#line 133 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"

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
#line 218 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"

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
      case 41: // ","
      case 42: // ";"
      case 43: // "&"
      case 44: // "|"
      case 45: // "every"
      case 46: // "for"
      case 47: // "loop"
      case 48: // "while"
      case 49: // "at"
        yysym.value.template destroy< ast::flavor_type >();
	break;

      case 116: // root
      case 117: // root_exp
      case 118: // root_exps
      case 120: // cstmt.opt
      case 121: // cstmt
      case 122: // stmt.opt
      case 123: // stmt
      case 124: // block
      case 126: // proto
      case 129: // exp
      case 138: // primary-exp
      case 140: // else.opt
      case 141: // onleave.opt
      case 148: // catch.opt
      case 149: // finally.opt
      case 154: // bitor-exp
      case 155: // new
      case 156: // float-exp
      case 164: // literal-exp
      case 167: // guard.opt
      case 168: // tilda.opt
      case 169: // unary-exp
      case 171: // rel-exp
      case 173: // exp.opt
        yysym.value.template destroy< ast::rExp >();
	break;

      case 50: // "identifier"
      case 62: // "^="
      case 63: // "-="
      case 64: // "%="
      case 65: // "+="
      case 66: // "/="
      case 67: // "*="
      case 75: // "new"
      case 84: // "!"
      case 85: // "bitand"
      case 86: // "bitor"
      case 87: // "^"
      case 88: // "compl"
      case 89: // ">>"
      case 90: // "<<"
      case 91: // "-"
      case 92: // "%"
      case 93: // "+"
      case 94: // "/"
      case 95: // "*"
      case 96: // "**"
      case 97: // "=~="
      case 98: // "=="
      case 99: // "==="
      case 100: // ">="
      case 101: // ">"
      case 102: // "<="
      case 103: // "<"
      case 104: // "!="
      case 105: // "!=="
      case 106: // "~="
      case 107: // "&&"
      case 108: // "||"
      case 133: // event_or_function
      case 153: // id
      case 170: // rel-op
        yysym.value.template destroy< libport::Symbol >();
	break;

      case 119: // stmts
      case 139: // default.opt
        yysym.value.template destroy< ast::rNary >();
	break;

      case 127: // protos.1
      case 128: // protos
      case 162: // tuple.exps
      case 163: // tuple
      case 175: // claims
      case 176: // claims.1
      case 177: // exps
      case 178: // exps.1
      case 179: // exps.2
      case 180: // args
      case 181: // args.opt
        yysym.value.template destroy< ast::exps_type* >();
	break;

      case 130: // id.0
      case 131: // id.1
        yysym.value.template destroy< ast::symbols_type >();
	break;

      case 174: // unsigned
        yysym.value.template destroy< unsigned >();
	break;

      case 134: // routine
      case 151: // detach
        yysym.value.template destroy< bool >();
	break;

      case 135: // k1_id
        yysym.value.template destroy< ast::rCall >();
	break;

      case 136: // modifier
        yysym.value.template destroy< ::ast::Factory::modifier_type >();
	break;

      case 137: // modifiers
        yysym.value.template destroy< ast::modifiers_type >();
	break;

      case 142: // cases
        yysym.value.template destroy< ::ast::Factory::cases_type >();
	break;

      case 143: // case
        yysym.value.template destroy< ::ast::Factory::case_type >();
	break;

      case 144: // catches.1
        yysym.value.template destroy< ast::catches_type >();
	break;

      case 145: // match
      case 146: // match.opt
        yysym.value.template destroy< ast::rMatch >();
	break;

      case 147: // catch
        yysym.value.template destroy< ast::rCatch >();
	break;

      case 152: // lvalue
        yysym.value.template destroy< ast::rLValue >();
	break;

      case 76: // "angle"
      case 77: // "duration"
      case 78: // "float"
      case 157: // duration
        yysym.value.template destroy< libport::ufloat >();
	break;

      case 158: // assoc
        yysym.value.template destroy< ast::dictionary_elt_type >();
	break;

      case 159: // assocs.1
      case 160: // assocs
        yysym.value.template destroy< ast::dictionary_elts_type >();
	break;

      case 161: // dictionary
        yysym.value.template destroy< ast::rDictionary >();
	break;

      case 80: // "string"
      case 165: // string
        yysym.value.template destroy< std::string >();
	break;

      case 166: // event_match
        yysym.value.template destroy< ast::EventMatch >();
	break;

      case 172: // rel-ops
        yysym.value.template destroy< ::ast::Factory::relations_type >();
	break;

      case 182: // identifiers
        yysym.value.template destroy< ::ast::symbols_type >();
	break;

      case 183: // typespec
      case 184: // typespec.opt
        yysym.value.template destroy< ::ast::rExp >();
	break;

      case 185: // formal
        yysym.value.template destroy< ::ast::Formal >();
	break;

      case 186: // formals.1
      case 187: // formals.0
      case 188: // formals
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
            case 41: // ","

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 544 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 42: // ";"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 553 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 43: // "&"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 562 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 44: // "|"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 571 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 45: // "every"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 580 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 46: // "for"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 589 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 47: // "loop"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 598 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 48: // "while"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 607 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 49: // "at"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 616 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 50: // "identifier"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 625 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 62: // "^="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 634 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 63: // "-="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 643 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 64: // "%="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 652 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 65: // "+="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 661 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 66: // "/="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 670 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 67: // "*="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 679 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 75: // "new"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 688 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 76: // "angle"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 697 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 77: // "duration"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 706 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 78: // "float"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 715 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 80: // "string"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< std::string >(); }
/* Line 576 of lalr1.cc  */
#line 724 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 84: // "!"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 733 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 85: // "bitand"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 742 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 86: // "bitor"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 751 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 87: // "^"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 760 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 88: // "compl"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 769 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 89: // ">>"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 778 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 90: // "<<"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 787 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 91: // "-"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 796 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 92: // "%"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 805 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 93: // "+"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 814 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 94: // "/"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 823 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 95: // "*"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 832 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 96: // "**"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 841 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 97: // "=~="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 850 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 98: // "=="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 859 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 99: // "==="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 868 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 100: // ">="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 877 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 101: // ">"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 886 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 102: // "<="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 895 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 103: // "<"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 904 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 104: // "!="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 913 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 105: // "!=="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 922 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 106: // "~="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 931 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 107: // "&&"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 940 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 108: // "||"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 949 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 116: // root

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 958 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 117: // root_exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 967 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 118: // root_exps

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 976 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 119: // stmts

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rNary >(); }
/* Line 576 of lalr1.cc  */
#line 985 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 120: // cstmt.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 994 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 121: // cstmt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1003 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 122: // stmt.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1012 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 123: // stmt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1021 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 124: // block

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1030 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 126: // proto

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1039 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 127: // protos.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1048 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 128: // protos

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1057 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 129: // exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1066 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 130: // id.0

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::symbols_type >(); }
/* Line 576 of lalr1.cc  */
#line 1075 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 131: // id.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::symbols_type >(); }
/* Line 576 of lalr1.cc  */
#line 1084 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 133: // event_or_function

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 1093 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 134: // routine

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< bool >(); }
/* Line 576 of lalr1.cc  */
#line 1102 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 135: // k1_id

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rCall >(); }
/* Line 576 of lalr1.cc  */
#line 1111 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 136: // modifier

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::modifier_type >(); }
/* Line 576 of lalr1.cc  */
#line 1120 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 137: // modifiers

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::modifiers_type >(); }
/* Line 576 of lalr1.cc  */
#line 1129 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 138: // primary-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1138 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 139: // default.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rNary >(); }
/* Line 576 of lalr1.cc  */
#line 1147 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 140: // else.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1156 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 141: // onleave.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1165 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 142: // cases

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::cases_type >(); }
/* Line 576 of lalr1.cc  */
#line 1174 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 143: // case

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::case_type >(); }
/* Line 576 of lalr1.cc  */
#line 1183 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 144: // catches.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::catches_type >(); }
/* Line 576 of lalr1.cc  */
#line 1192 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 145: // match

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rMatch >(); }
/* Line 576 of lalr1.cc  */
#line 1201 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 146: // match.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rMatch >(); }
/* Line 576 of lalr1.cc  */
#line 1210 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 147: // catch

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rCatch >(); }
/* Line 576 of lalr1.cc  */
#line 1219 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 148: // catch.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1228 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 149: // finally.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1237 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 151: // detach

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< bool >(); }
/* Line 576 of lalr1.cc  */
#line 1246 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 152: // lvalue

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rLValue >(); }
/* Line 576 of lalr1.cc  */
#line 1255 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 153: // id

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 1264 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 154: // bitor-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1273 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 155: // new

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1282 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 156: // float-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1291 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 157: // duration

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 1300 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 158: // assoc

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::dictionary_elt_type >(); }
/* Line 576 of lalr1.cc  */
#line 1309 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 159: // assocs.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::dictionary_elts_type >(); }
/* Line 576 of lalr1.cc  */
#line 1318 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 160: // assocs

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::dictionary_elts_type >(); }
/* Line 576 of lalr1.cc  */
#line 1327 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 161: // dictionary

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rDictionary >(); }
/* Line 576 of lalr1.cc  */
#line 1336 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 162: // tuple.exps

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1345 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 163: // tuple

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1354 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 164: // literal-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1363 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 165: // string

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< std::string >(); }
/* Line 576 of lalr1.cc  */
#line 1372 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 166: // event_match

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::EventMatch >(); }
/* Line 576 of lalr1.cc  */
#line 1381 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 167: // guard.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1390 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 168: // tilda.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1399 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 169: // unary-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1408 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 170: // rel-op

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 1417 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 171: // rel-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1426 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 172: // rel-ops

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::relations_type >(); }
/* Line 576 of lalr1.cc  */
#line 1435 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 173: // exp.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1444 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 174: // unsigned

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< unsigned >(); }
/* Line 576 of lalr1.cc  */
#line 1453 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 175: // claims

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1462 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 176: // claims.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1471 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 177: // exps

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1480 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 178: // exps.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1489 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 179: // exps.2

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1498 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 180: // args

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1507 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 181: // args.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1516 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 182: // identifiers

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::symbols_type >(); }
/* Line 576 of lalr1.cc  */
#line 1525 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 183: // typespec

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1534 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 184: // typespec.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1543 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 185: // formal

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formal >(); }
/* Line 576 of lalr1.cc  */
#line 1552 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 186: // formals.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formals* >(); }
/* Line 576 of lalr1.cc  */
#line 1561 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 187: // formals.0

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formals* >(); }
/* Line 576 of lalr1.cc  */
#line 1570 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
        break;

            case 188: // formals

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formals* >(); }
/* Line 576 of lalr1.cc  */
#line 1579 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
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
      case 41: // ","
      case 42: // ";"
      case 43: // "&"
      case 44: // "|"
      case 45: // "every"
      case 46: // "for"
      case 47: // "loop"
      case 48: // "while"
      case 49: // "at"
        yystack_[0].value.build< ast::flavor_type >(sym.value);
	break;

      case 116: // root
      case 117: // root_exp
      case 118: // root_exps
      case 120: // cstmt.opt
      case 121: // cstmt
      case 122: // stmt.opt
      case 123: // stmt
      case 124: // block
      case 126: // proto
      case 129: // exp
      case 138: // primary-exp
      case 140: // else.opt
      case 141: // onleave.opt
      case 148: // catch.opt
      case 149: // finally.opt
      case 154: // bitor-exp
      case 155: // new
      case 156: // float-exp
      case 164: // literal-exp
      case 167: // guard.opt
      case 168: // tilda.opt
      case 169: // unary-exp
      case 171: // rel-exp
      case 173: // exp.opt
        yystack_[0].value.build< ast::rExp >(sym.value);
	break;

      case 50: // "identifier"
      case 62: // "^="
      case 63: // "-="
      case 64: // "%="
      case 65: // "+="
      case 66: // "/="
      case 67: // "*="
      case 75: // "new"
      case 84: // "!"
      case 85: // "bitand"
      case 86: // "bitor"
      case 87: // "^"
      case 88: // "compl"
      case 89: // ">>"
      case 90: // "<<"
      case 91: // "-"
      case 92: // "%"
      case 93: // "+"
      case 94: // "/"
      case 95: // "*"
      case 96: // "**"
      case 97: // "=~="
      case 98: // "=="
      case 99: // "==="
      case 100: // ">="
      case 101: // ">"
      case 102: // "<="
      case 103: // "<"
      case 104: // "!="
      case 105: // "!=="
      case 106: // "~="
      case 107: // "&&"
      case 108: // "||"
      case 133: // event_or_function
      case 153: // id
      case 170: // rel-op
        yystack_[0].value.build< libport::Symbol >(sym.value);
	break;

      case 119: // stmts
      case 139: // default.opt
        yystack_[0].value.build< ast::rNary >(sym.value);
	break;

      case 127: // protos.1
      case 128: // protos
      case 162: // tuple.exps
      case 163: // tuple
      case 175: // claims
      case 176: // claims.1
      case 177: // exps
      case 178: // exps.1
      case 179: // exps.2
      case 180: // args
      case 181: // args.opt
        yystack_[0].value.build< ast::exps_type* >(sym.value);
	break;

      case 130: // id.0
      case 131: // id.1
        yystack_[0].value.build< ast::symbols_type >(sym.value);
	break;

      case 174: // unsigned
        yystack_[0].value.build< unsigned >(sym.value);
	break;

      case 134: // routine
      case 151: // detach
        yystack_[0].value.build< bool >(sym.value);
	break;

      case 135: // k1_id
        yystack_[0].value.build< ast::rCall >(sym.value);
	break;

      case 136: // modifier
        yystack_[0].value.build< ::ast::Factory::modifier_type >(sym.value);
	break;

      case 137: // modifiers
        yystack_[0].value.build< ast::modifiers_type >(sym.value);
	break;

      case 142: // cases
        yystack_[0].value.build< ::ast::Factory::cases_type >(sym.value);
	break;

      case 143: // case
        yystack_[0].value.build< ::ast::Factory::case_type >(sym.value);
	break;

      case 144: // catches.1
        yystack_[0].value.build< ast::catches_type >(sym.value);
	break;

      case 145: // match
      case 146: // match.opt
        yystack_[0].value.build< ast::rMatch >(sym.value);
	break;

      case 147: // catch
        yystack_[0].value.build< ast::rCatch >(sym.value);
	break;

      case 152: // lvalue
        yystack_[0].value.build< ast::rLValue >(sym.value);
	break;

      case 76: // "angle"
      case 77: // "duration"
      case 78: // "float"
      case 157: // duration
        yystack_[0].value.build< libport::ufloat >(sym.value);
	break;

      case 158: // assoc
        yystack_[0].value.build< ast::dictionary_elt_type >(sym.value);
	break;

      case 159: // assocs.1
      case 160: // assocs
        yystack_[0].value.build< ast::dictionary_elts_type >(sym.value);
	break;

      case 161: // dictionary
        yystack_[0].value.build< ast::rDictionary >(sym.value);
	break;

      case 80: // "string"
      case 165: // string
        yystack_[0].value.build< std::string >(sym.value);
	break;

      case 166: // event_match
        yystack_[0].value.build< ast::EventMatch >(sym.value);
	break;

      case 172: // rel-ops
        yystack_[0].value.build< ::ast::Factory::relations_type >(sym.value);
	break;

      case 182: // identifiers
        yystack_[0].value.build< ::ast::symbols_type >(sym.value);
	break;

      case 183: // typespec
      case 184: // typespec.opt
        yystack_[0].value.build< ::ast::rExp >(sym.value);
	break;

      case 185: // formal
        yystack_[0].value.build< ::ast::Formal >(sym.value);
	break;

      case 186: // formals.1
      case 187: // formals.0
      case 188: // formals
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
      case 41: // ","
      case 42: // ";"
      case 43: // "&"
      case 44: // "|"
      case 45: // "every"
      case 46: // "for"
      case 47: // "loop"
      case 48: // "while"
      case 49: // "at"
        yystack_[0].value.build< ast::flavor_type >(s.value);
	break;

      case 116: // root
      case 117: // root_exp
      case 118: // root_exps
      case 120: // cstmt.opt
      case 121: // cstmt
      case 122: // stmt.opt
      case 123: // stmt
      case 124: // block
      case 126: // proto
      case 129: // exp
      case 138: // primary-exp
      case 140: // else.opt
      case 141: // onleave.opt
      case 148: // catch.opt
      case 149: // finally.opt
      case 154: // bitor-exp
      case 155: // new
      case 156: // float-exp
      case 164: // literal-exp
      case 167: // guard.opt
      case 168: // tilda.opt
      case 169: // unary-exp
      case 171: // rel-exp
      case 173: // exp.opt
        yystack_[0].value.build< ast::rExp >(s.value);
	break;

      case 50: // "identifier"
      case 62: // "^="
      case 63: // "-="
      case 64: // "%="
      case 65: // "+="
      case 66: // "/="
      case 67: // "*="
      case 75: // "new"
      case 84: // "!"
      case 85: // "bitand"
      case 86: // "bitor"
      case 87: // "^"
      case 88: // "compl"
      case 89: // ">>"
      case 90: // "<<"
      case 91: // "-"
      case 92: // "%"
      case 93: // "+"
      case 94: // "/"
      case 95: // "*"
      case 96: // "**"
      case 97: // "=~="
      case 98: // "=="
      case 99: // "==="
      case 100: // ">="
      case 101: // ">"
      case 102: // "<="
      case 103: // "<"
      case 104: // "!="
      case 105: // "!=="
      case 106: // "~="
      case 107: // "&&"
      case 108: // "||"
      case 133: // event_or_function
      case 153: // id
      case 170: // rel-op
        yystack_[0].value.build< libport::Symbol >(s.value);
	break;

      case 119: // stmts
      case 139: // default.opt
        yystack_[0].value.build< ast::rNary >(s.value);
	break;

      case 127: // protos.1
      case 128: // protos
      case 162: // tuple.exps
      case 163: // tuple
      case 175: // claims
      case 176: // claims.1
      case 177: // exps
      case 178: // exps.1
      case 179: // exps.2
      case 180: // args
      case 181: // args.opt
        yystack_[0].value.build< ast::exps_type* >(s.value);
	break;

      case 130: // id.0
      case 131: // id.1
        yystack_[0].value.build< ast::symbols_type >(s.value);
	break;

      case 174: // unsigned
        yystack_[0].value.build< unsigned >(s.value);
	break;

      case 134: // routine
      case 151: // detach
        yystack_[0].value.build< bool >(s.value);
	break;

      case 135: // k1_id
        yystack_[0].value.build< ast::rCall >(s.value);
	break;

      case 136: // modifier
        yystack_[0].value.build< ::ast::Factory::modifier_type >(s.value);
	break;

      case 137: // modifiers
        yystack_[0].value.build< ast::modifiers_type >(s.value);
	break;

      case 142: // cases
        yystack_[0].value.build< ::ast::Factory::cases_type >(s.value);
	break;

      case 143: // case
        yystack_[0].value.build< ::ast::Factory::case_type >(s.value);
	break;

      case 144: // catches.1
        yystack_[0].value.build< ast::catches_type >(s.value);
	break;

      case 145: // match
      case 146: // match.opt
        yystack_[0].value.build< ast::rMatch >(s.value);
	break;

      case 147: // catch
        yystack_[0].value.build< ast::rCatch >(s.value);
	break;

      case 152: // lvalue
        yystack_[0].value.build< ast::rLValue >(s.value);
	break;

      case 76: // "angle"
      case 77: // "duration"
      case 78: // "float"
      case 157: // duration
        yystack_[0].value.build< libport::ufloat >(s.value);
	break;

      case 158: // assoc
        yystack_[0].value.build< ast::dictionary_elt_type >(s.value);
	break;

      case 159: // assocs.1
      case 160: // assocs
        yystack_[0].value.build< ast::dictionary_elts_type >(s.value);
	break;

      case 161: // dictionary
        yystack_[0].value.build< ast::rDictionary >(s.value);
	break;

      case 80: // "string"
      case 165: // string
        yystack_[0].value.build< std::string >(s.value);
	break;

      case 166: // event_match
        yystack_[0].value.build< ast::EventMatch >(s.value);
	break;

      case 172: // rel-ops
        yystack_[0].value.build< ::ast::Factory::relations_type >(s.value);
	break;

      case 182: // identifiers
        yystack_[0].value.build< ::ast::symbols_type >(s.value);
	break;

      case 183: // typespec
      case 184: // typespec.opt
        yystack_[0].value.build< ::ast::rExp >(s.value);
	break;

      case 185: // formal
        yystack_[0].value.build< ::ast::Formal >(s.value);
	break;

      case 186: // formals.1
      case 187: // formals.0
      case 188: // formals
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
#line 55 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
{
  // Saved when exiting the start symbol.
  up.scanner_.loc = up.loc_;
}
/* Line 701 of lalr1.cc  */
#line 2110 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"

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
      case 41: // ","
      case 42: // ";"
      case 43: // "&"
      case 44: // "|"
      case 45: // "every"
      case 46: // "for"
      case 47: // "loop"
      case 48: // "while"
      case 49: // "at"
        yylhs.value.build< ast::flavor_type >();
	break;

      case 116: // root
      case 117: // root_exp
      case 118: // root_exps
      case 120: // cstmt.opt
      case 121: // cstmt
      case 122: // stmt.opt
      case 123: // stmt
      case 124: // block
      case 126: // proto
      case 129: // exp
      case 138: // primary-exp
      case 140: // else.opt
      case 141: // onleave.opt
      case 148: // catch.opt
      case 149: // finally.opt
      case 154: // bitor-exp
      case 155: // new
      case 156: // float-exp
      case 164: // literal-exp
      case 167: // guard.opt
      case 168: // tilda.opt
      case 169: // unary-exp
      case 171: // rel-exp
      case 173: // exp.opt
        yylhs.value.build< ast::rExp >();
	break;

      case 50: // "identifier"
      case 62: // "^="
      case 63: // "-="
      case 64: // "%="
      case 65: // "+="
      case 66: // "/="
      case 67: // "*="
      case 75: // "new"
      case 84: // "!"
      case 85: // "bitand"
      case 86: // "bitor"
      case 87: // "^"
      case 88: // "compl"
      case 89: // ">>"
      case 90: // "<<"
      case 91: // "-"
      case 92: // "%"
      case 93: // "+"
      case 94: // "/"
      case 95: // "*"
      case 96: // "**"
      case 97: // "=~="
      case 98: // "=="
      case 99: // "==="
      case 100: // ">="
      case 101: // ">"
      case 102: // "<="
      case 103: // "<"
      case 104: // "!="
      case 105: // "!=="
      case 106: // "~="
      case 107: // "&&"
      case 108: // "||"
      case 133: // event_or_function
      case 153: // id
      case 170: // rel-op
        yylhs.value.build< libport::Symbol >();
	break;

      case 119: // stmts
      case 139: // default.opt
        yylhs.value.build< ast::rNary >();
	break;

      case 127: // protos.1
      case 128: // protos
      case 162: // tuple.exps
      case 163: // tuple
      case 175: // claims
      case 176: // claims.1
      case 177: // exps
      case 178: // exps.1
      case 179: // exps.2
      case 180: // args
      case 181: // args.opt
        yylhs.value.build< ast::exps_type* >();
	break;

      case 130: // id.0
      case 131: // id.1
        yylhs.value.build< ast::symbols_type >();
	break;

      case 174: // unsigned
        yylhs.value.build< unsigned >();
	break;

      case 134: // routine
      case 151: // detach
        yylhs.value.build< bool >();
	break;

      case 135: // k1_id
        yylhs.value.build< ast::rCall >();
	break;

      case 136: // modifier
        yylhs.value.build< ::ast::Factory::modifier_type >();
	break;

      case 137: // modifiers
        yylhs.value.build< ast::modifiers_type >();
	break;

      case 142: // cases
        yylhs.value.build< ::ast::Factory::cases_type >();
	break;

      case 143: // case
        yylhs.value.build< ::ast::Factory::case_type >();
	break;

      case 144: // catches.1
        yylhs.value.build< ast::catches_type >();
	break;

      case 145: // match
      case 146: // match.opt
        yylhs.value.build< ast::rMatch >();
	break;

      case 147: // catch
        yylhs.value.build< ast::rCatch >();
	break;

      case 152: // lvalue
        yylhs.value.build< ast::rLValue >();
	break;

      case 76: // "angle"
      case 77: // "duration"
      case 78: // "float"
      case 157: // duration
        yylhs.value.build< libport::ufloat >();
	break;

      case 158: // assoc
        yylhs.value.build< ast::dictionary_elt_type >();
	break;

      case 159: // assocs.1
      case 160: // assocs
        yylhs.value.build< ast::dictionary_elts_type >();
	break;

      case 161: // dictionary
        yylhs.value.build< ast::rDictionary >();
	break;

      case 80: // "string"
      case 165: // string
        yylhs.value.build< std::string >();
	break;

      case 166: // event_match
        yylhs.value.build< ast::EventMatch >();
	break;

      case 172: // rel-ops
        yylhs.value.build< ::ast::Factory::relations_type >();
	break;

      case 182: // identifiers
        yylhs.value.build< ::ast::symbols_type >();
	break;

      case 183: // typespec
      case 184: // typespec.opt
        yylhs.value.build< ::ast::rExp >();
	break;

      case 185: // formal
        yylhs.value.build< ::ast::Formal >();
	break;

      case 186: // formals.1
      case 187: // formals.0
      case 188: // formals
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
#line 314 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    // Remove the reference from yystack by swaping with a 0 intrusive
    // pointer.
    aver(up.result_.get() == 0);
    std::swap(up.result_, yystack_[0].value.as< ast::rExp >());
    up.loc_ = yylhs.location;
    YYACCEPT;
  }
/* Line 828 of lalr1.cc  */
#line 2431 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 3:
/* Line 828 of lalr1.cc  */
#line 328 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2439 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 4:
/* Line 828 of lalr1.cc  */
#line 329 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2447 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 5:
/* Line 828 of lalr1.cc  */
#line 330 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2455 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 6:
/* Line 828 of lalr1.cc  */
#line 336 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Stmt(yylhs.location, yystack_[0].value.as< ast::flavor_type >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2463 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 7:
/* Line 828 of lalr1.cc  */
#line 337 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Stmt(yylhs.location, yystack_[0].value.as< ast::flavor_type >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2471 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 8:
/* Line 828 of lalr1.cc  */
#line 338 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Stmt(yylhs.location, ast::flavor_none, yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2479 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 9:
/* Line 828 of lalr1.cc  */
#line 339 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2487 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 10:
/* Line 828 of lalr1.cc  */
#line 340 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2495 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 11:
/* Line 828 of lalr1.cc  */
#line 341 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2503 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 12:
/* Line 828 of lalr1.cc  */
#line 346 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rNary >(); }
/* Line 828 of lalr1.cc  */
#line 2511 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 13:
/* Line 828 of lalr1.cc  */
#line 358 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = MAKE(nary, yylhs.location, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2519 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 14:
/* Line 828 of lalr1.cc  */
#line 359 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = MAKE(nary, yylhs.location, yystack_[2].value.as< ast::rNary >(), yystack_[1].location, yystack_[1].value.as< ast::flavor_type >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2527 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 15:
/* Line 828 of lalr1.cc  */
#line 360 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = MAKE(nary, yylhs.location, yystack_[2].value.as< ast::rNary >(), yystack_[1].location, yystack_[1].value.as< ast::flavor_type >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2535 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 16:
/* Line 828 of lalr1.cc  */
#line 368 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2543 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 17:
/* Line 828 of lalr1.cc  */
#line 369 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2551 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 18:
/* Line 828 of lalr1.cc  */
#line 370 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(bin, yylhs.location, yystack_[0].value.as< ast::flavor_type >(), yystack_[1].value.as< ast::rExp >(), MAKE(noop, yystack_[0].location)); }
/* Line 828 of lalr1.cc  */
#line 2559 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 19:
/* Line 828 of lalr1.cc  */
#line 375 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { assert(yystack_[0].value.as< ast::rExp >()); std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2567 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 20:
/* Line 828 of lalr1.cc  */
#line 376 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(bin, yylhs.location, yystack_[1].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2575 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 21:
/* Line 828 of lalr1.cc  */
#line 377 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(bin, yylhs.location, yystack_[1].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2583 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 22:
/* Line 828 of lalr1.cc  */
#line 387 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2591 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 23:
/* Line 828 of lalr1.cc  */
#line 388 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2599 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 24:
/* Line 828 of lalr1.cc  */
#line 394 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::TaggedStmt(yylhs.location, yystack_[2].value.as< ast::rExp >(), MAKE(scope, yylhs.location, yystack_[0].value.as< ast::rExp >()));
  }
/* Line 828 of lalr1.cc  */
#line 2609 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 25:
/* Line 828 of lalr1.cc  */
#line 404 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2617 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 26:
/* Line 828 of lalr1.cc  */
#line 408 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(strip, yystack_[1].value.as< ast::rNary >()); }
/* Line 828 of lalr1.cc  */
#line 2625 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 27:
/* Line 828 of lalr1.cc  */
#line 409 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2633 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 28:
/* Line 828 of lalr1.cc  */
#line 413 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2641 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 33:
/* Line 828 of lalr1.cc  */
#line 435 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2649 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 34:
/* Line 828 of lalr1.cc  */
#line 441 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2657 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 35:
/* Line 828 of lalr1.cc  */
#line 442 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 2665 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 36:
/* Line 828 of lalr1.cc  */
#line 447 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2673 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 37:
/* Line 828 of lalr1.cc  */
#line 448 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 2681 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 38:
/* Line 828 of lalr1.cc  */
#line 454 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(class, yylhs.location, yystack_[2].value.as< ast::rLValue >(), yystack_[1].value.as< ast::exps_type* >(), yystack_[0].value.as< ast::rExp >(), false);
    }
/* Line 828 of lalr1.cc  */
#line 2691 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 39:
/* Line 828 of lalr1.cc  */
#line 463 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      ast::rClass c = MAKE(class, yylhs.location, yystack_[2].value.as< ast::rLValue >(), yystack_[1].value.as< ast::exps_type* >(), yystack_[0].value.as< ast::rExp >(), true).unsafe_cast<ast::Class>();
      c->is_package_set(true);
      yylhs.value.as< ast::rExp >() = c;
    }
/* Line 828 of lalr1.cc  */
#line 2703 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 40:
/* Line 828 of lalr1.cc  */
#line 478 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {}
/* Line 828 of lalr1.cc  */
#line 2711 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 41:
/* Line 828 of lalr1.cc  */
#line 479 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::symbols_type >(), yystack_[1].value.as< ast::symbols_type >()); }
/* Line 828 of lalr1.cc  */
#line 2719 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 42:
/* Line 828 of lalr1.cc  */
#line 483 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::symbols_type >() << yystack_[0].value.as< libport::Symbol >(); }
/* Line 828 of lalr1.cc  */
#line 2727 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 43:
/* Line 828 of lalr1.cc  */
#line 484 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::symbols_type >(), yystack_[2].value.as< ast::symbols_type >()); yylhs.value.as< ast::symbols_type >() << yystack_[0].value.as< libport::Symbol >(); }
/* Line 828 of lalr1.cc  */
#line 2735 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 44:
/* Line 828 of lalr1.cc  */
#line 490 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(enum, yylhs.location, yystack_[3].value.as< libport::Symbol >(), yystack_[1].value.as< ast::symbols_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2745 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 45:
/* Line 828 of lalr1.cc  */
#line 501 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    REQUIRE_IDENTIFIER(yystack_[0].location, yystack_[0].value.as< libport::Symbol >(), "from");
  }
/* Line 828 of lalr1.cc  */
#line 2755 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 46:
/* Line 828 of lalr1.cc  */
#line 510 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< libport::Symbol >() = SYMBOL(function);
  }
/* Line 828 of lalr1.cc  */
#line 2765 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 47:
/* Line 828 of lalr1.cc  */
#line 514 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    REQUIRE_IDENTIFIER(yystack_[0].location, yystack_[0].value.as< libport::Symbol >(), "event");
    yylhs.value.as< libport::Symbol >() = SYMBOL(event);
  }
/* Line 828 of lalr1.cc  */
#line 2776 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 48:
/* Line 828 of lalr1.cc  */
#line 524 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    REQUIRE_IDENTIFIER(yystack_[1].location, yystack_[1].value.as< libport::Symbol >(), "object");
    yylhs.value.as< ast::rExp >() = MAKE(external_object, yylhs.location, yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 2787 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 49:
/* Line 828 of lalr1.cc  */
#line 530 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(external_var, yylhs.location, yystack_[4].value.as< libport::Symbol >(), yystack_[2].value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 2797 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 50:
/* Line 828 of lalr1.cc  */
#line 536 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(external_event_or_function,
              yylhs.location, yystack_[8].value.as< libport::Symbol >(), yystack_[6].value.as< unsigned >(), yystack_[4].value.as< libport::Symbol >(), yystack_[2].value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 2808 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 51:
/* Line 828 of lalr1.cc  */
#line 547 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
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
#line 2840 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 52:
/* Line 828 of lalr1.cc  */
#line 588 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::Emit(yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[1].value.as< ast::exps_type* >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 2850 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 53:
/* Line 828 of lalr1.cc  */
#line 600 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = true; }
/* Line 828 of lalr1.cc  */
#line 2858 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 54:
/* Line 828 of lalr1.cc  */
#line 601 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = false; }
/* Line 828 of lalr1.cc  */
#line 2866 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 55:
/* Line 828 of lalr1.cc  */
#line 608 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      // Compiled as "var name = function args stmt".
      yylhs.value.as< ast::rExp >() = new ast::Declaration(yylhs.location, yystack_[2].value.as< ast::rCall >(),
                                MAKE(routine, yylhs.location, yystack_[3].value.as< bool >(), yystack_[1].location, yystack_[1].value.as< ::ast::Formals* >(), yystack_[0].value.as< ast::rExp >()));
    }
/* Line 828 of lalr1.cc  */
#line 2878 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 56:
/* Line 828 of lalr1.cc  */
#line 614 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
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
#line 2898 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 57:
/* Line 828 of lalr1.cc  */
#line 660 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCall >() = MAKE(call, yylhs.location, yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2906 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 58:
/* Line 828 of lalr1.cc  */
#line 661 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCall >() = MAKE(call, yylhs.location, new ast::This(yystack_[2].location), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2914 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 59:
/* Line 828 of lalr1.cc  */
#line 662 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCall >() = MAKE(call, yylhs.location, ast::rExp(yystack_[2].value.as< ast::rCall >()), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2922 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 60:
/* Line 828 of lalr1.cc  */
#line 674 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ::ast::Factory::modifier_type >().first = yystack_[2].value.as< libport::Symbol >();
    yylhs.value.as< ::ast::Factory::modifier_type >().second = yystack_[0].value.as< ast::rExp >();
  }
/* Line 828 of lalr1.cc  */
#line 2933 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 61:
/* Line 828 of lalr1.cc  */
#line 683 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    modifiers_add(up, yystack_[0].location, yylhs.value.as< ast::modifiers_type >(), yystack_[0].value.as< ::ast::Factory::modifier_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2943 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 62:
/* Line 828 of lalr1.cc  */
#line 687 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    std::swap(yylhs.value.as< ast::modifiers_type >(), yystack_[1].value.as< ast::modifiers_type >());
    modifiers_add(up, yystack_[0].location, yylhs.value.as< ast::modifiers_type >(), yystack_[0].value.as< ::ast::Factory::modifier_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2954 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 63:
/* Line 828 of lalr1.cc  */
#line 704 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(assign, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 2964 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 64:
/* Line 828 of lalr1.cc  */
#line 708 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(assign, yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::modifiers_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2974 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 65:
/* Line 828 of lalr1.cc  */
#line 723 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2982 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 66:
/* Line 828 of lalr1.cc  */
#line 724 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2990 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 67:
/* Line 828 of lalr1.cc  */
#line 725 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2998 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 68:
/* Line 828 of lalr1.cc  */
#line 726 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3006 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 69:
/* Line 828 of lalr1.cc  */
#line 727 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3014 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 70:
/* Line 828 of lalr1.cc  */
#line 728 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3022 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 71:
/* Line 828 of lalr1.cc  */
#line 736 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Decrementation(yylhs.location, yystack_[1].value.as< ast::rLValue >(), true); }
/* Line 828 of lalr1.cc  */
#line 3030 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 72:
/* Line 828 of lalr1.cc  */
#line 737 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Incrementation(yylhs.location, yystack_[1].value.as< ast::rLValue >(), true); }
/* Line 828 of lalr1.cc  */
#line 3038 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 73:
/* Line 828 of lalr1.cc  */
#line 749 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::Property(yylhs.location, yystack_[2].value.as< ast::rLValue >()->call(), yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 3048 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 74:
/* Line 828 of lalr1.cc  */
#line 760 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(at, yylhs.location, yystack_[7].location, yystack_[7].value.as< ast::flavor_type >(), yystack_[6].value.as< ::ast::symbols_type >(), yystack_[4].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >(), yystack_[3].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3058 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 75:
/* Line 828 of lalr1.cc  */
#line 764 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(at_event, yylhs.location, yystack_[6].location, yystack_[6].value.as< ast::flavor_type >(), yystack_[5].value.as< ::ast::symbols_type >(), yystack_[3].value.as< ast::EventMatch >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3068 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 76:
/* Line 828 of lalr1.cc  */
#line 768 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(every, yylhs.location, yystack_[4].location, yystack_[4].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3078 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 77:
/* Line 828 of lalr1.cc  */
#line 772 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(if, yylhs.location, yystack_[3].value.as< ast::rNary >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3088 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 78:
/* Line 828 of lalr1.cc  */
#line 776 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(freezeif, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3098 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 79:
/* Line 828 of lalr1.cc  */
#line 780 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(stopif, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3108 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 80:
/* Line 828 of lalr1.cc  */
#line 784 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(switch, yystack_[5].location, yystack_[5].value.as< ast::rExp >(), yystack_[2].value.as< ::ast::Factory::cases_type >(), yystack_[1].value.as< ast::rNary >());
    }
/* Line 828 of lalr1.cc  */
#line 3118 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 81:
/* Line 828 of lalr1.cc  */
#line 788 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(timeout, yylhs.location,
                yystack_[5].value.as< ast::rExp >(), yystack_[3].value.as< ast::rExp >(), yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3129 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 82:
/* Line 828 of lalr1.cc  */
#line 793 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = new ast::Return(yylhs.location, yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3139 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 83:
/* Line 828 of lalr1.cc  */
#line 797 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = new ast::Break(yylhs.location);
    }
/* Line 828 of lalr1.cc  */
#line 3149 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 84:
/* Line 828 of lalr1.cc  */
#line 801 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = new ast::Continue(yylhs.location);
    }
/* Line 828 of lalr1.cc  */
#line 3159 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 85:
/* Line 828 of lalr1.cc  */
#line 805 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(waituntil, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3169 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 86:
/* Line 828 of lalr1.cc  */
#line 809 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(waituntil_event, yylhs.location, yystack_[1].value.as< ast::EventMatch >());
    }
/* Line 828 of lalr1.cc  */
#line 3179 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 87:
/* Line 828 of lalr1.cc  */
#line 813 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(whenever, yylhs.location, yystack_[4].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >(), yystack_[3].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3189 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 88:
/* Line 828 of lalr1.cc  */
#line 817 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(whenever_event, yylhs.location, yystack_[3].value.as< ast::EventMatch >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3199 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 89:
/* Line 828 of lalr1.cc  */
#line 833 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = 0;            }
/* Line 828 of lalr1.cc  */
#line 3207 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 90:
/* Line 828 of lalr1.cc  */
#line 834 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rNary >(), yystack_[0].value.as< ast::rNary >()); }
/* Line 828 of lalr1.cc  */
#line 3215 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 91:
/* Line 828 of lalr1.cc  */
#line 839 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0;            }
/* Line 828 of lalr1.cc  */
#line 3223 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 92:
/* Line 828 of lalr1.cc  */
#line 840 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3231 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 93:
/* Line 828 of lalr1.cc  */
#line 846 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0;            }
/* Line 828 of lalr1.cc  */
#line 3239 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 94:
/* Line 828 of lalr1.cc  */
#line 847 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3247 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 95:
/* Line 828 of lalr1.cc  */
#line 857 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {}
/* Line 828 of lalr1.cc  */
#line 3255 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 96:
/* Line 828 of lalr1.cc  */
#line 858 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Factory::cases_type >(), yystack_[1].value.as< ::ast::Factory::cases_type >()); yylhs.value.as< ::ast::Factory::cases_type >() << yystack_[0].value.as< ::ast::Factory::case_type >(); }
/* Line 828 of lalr1.cc  */
#line 3263 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 97:
/* Line 828 of lalr1.cc  */
#line 864 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Factory::case_type >() = ::ast::Factory::case_type(yystack_[2].value.as< ast::rMatch >(), yystack_[0].value.as< ast::rNary >()); }
/* Line 828 of lalr1.cc  */
#line 3271 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 98:
/* Line 828 of lalr1.cc  */
#line 873 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::catches_type >() = ast::catches_type(); yylhs.value.as< ast::catches_type >() << yystack_[0].value.as< ast::rCatch >(); }
/* Line 828 of lalr1.cc  */
#line 3279 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 99:
/* Line 828 of lalr1.cc  */
#line 874 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::catches_type >(), yystack_[1].value.as< ast::catches_type >());        yylhs.value.as< ast::catches_type >() << yystack_[0].value.as< ast::rCatch >(); }
/* Line 828 of lalr1.cc  */
#line 3287 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 100:
/* Line 828 of lalr1.cc  */
#line 879 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rMatch >() = new ast::Match(yylhs.location, yystack_[0].value.as< ast::rExp >(), 0);  }
/* Line 828 of lalr1.cc  */
#line 3295 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 101:
/* Line 828 of lalr1.cc  */
#line 880 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rMatch >() = new ast::Match(yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3303 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 102:
/* Line 828 of lalr1.cc  */
#line 883 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rMatch >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3311 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 103:
/* Line 828 of lalr1.cc  */
#line 884 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rMatch >(), yystack_[1].value.as< ast::rMatch >()); }
/* Line 828 of lalr1.cc  */
#line 3319 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 104:
/* Line 828 of lalr1.cc  */
#line 888 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCatch >() = MAKE(catch, yylhs.location, yystack_[1].value.as< ast::rMatch >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3327 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 105:
/* Line 828 of lalr1.cc  */
#line 895 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3335 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 106:
/* Line 828 of lalr1.cc  */
#line 896 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 3343 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 107:
/* Line 828 of lalr1.cc  */
#line 902 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0;  }
/* Line 828 of lalr1.cc  */
#line 3351 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 108:
/* Line 828 of lalr1.cc  */
#line 903 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 3359 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 109:
/* Line 828 of lalr1.cc  */
#line 908 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(try, yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[2].value.as< ast::catches_type >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3369 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 110:
/* Line 828 of lalr1.cc  */
#line 912 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(finally, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3379 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 111:
/* Line 828 of lalr1.cc  */
#line 916 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(throw, yylhs.location, yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3389 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 112:
/* Line 828 of lalr1.cc  */
#line 945 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(loop, yylhs.location, yystack_[1].location, yystack_[1].value.as< ast::flavor_type >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3399 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 113:
/* Line 828 of lalr1.cc  */
#line 949 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(for, yylhs.location, yystack_[4].location, yystack_[4].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3409 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 114:
/* Line 828 of lalr1.cc  */
#line 953 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(for, yylhs.location, yystack_[8].location, yystack_[8].value.as< ast::flavor_type >(), yystack_[6].value.as< ast::rExp >(), yystack_[4].value.as< ast::rExp >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3419 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 115:
/* Line 828 of lalr1.cc  */
#line 957 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(for, yylhs.location, yystack_[7].location, yystack_[7].value.as< ast::flavor_type >(), yystack_[4].location, yystack_[4].value.as< libport::Symbol >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3429 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 116:
/* Line 828 of lalr1.cc  */
#line 961 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(while, yylhs.location, yystack_[4].location, yystack_[4].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3439 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 119:
/* Line 828 of lalr1.cc  */
#line 976 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(scope, yylhs.location, 0, yystack_[0].value.as< ast::rExp >());  }
/* Line 828 of lalr1.cc  */
#line 3447 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 120:
/* Line 828 of lalr1.cc  */
#line 977 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(scope, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3455 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 121:
/* Line 828 of lalr1.cc  */
#line 989 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = true; }
/* Line 828 of lalr1.cc  */
#line 3463 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 122:
/* Line 828 of lalr1.cc  */
#line 990 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = false; }
/* Line 828 of lalr1.cc  */
#line 3471 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 123:
/* Line 828 of lalr1.cc  */
#line 994 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(assert, yylhs.location, yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3479 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 124:
/* Line 828 of lalr1.cc  */
#line 995 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(assert, yylhs.location, yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3487 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 125:
/* Line 828 of lalr1.cc  */
#line 996 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(detach, yylhs.location, yystack_[3].value.as< bool >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3495 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 126:
/* Line 828 of lalr1.cc  */
#line 997 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(detach, yylhs.location, yystack_[1].value.as< bool >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3503 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 127:
/* Line 828 of lalr1.cc  */
#line 998 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(isdef, yylhs.location, yystack_[1].value.as< ast::rCall >()); }
/* Line 828 of lalr1.cc  */
#line 3511 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 128:
/* Line 828 of lalr1.cc  */
#line 999 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(watch, yylhs.location, yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3519 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 129:
/* Line 828 of lalr1.cc  */
#line 1009 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rLValue >() = MAKE(call, yylhs.location, yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3527 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 130:
/* Line 828 of lalr1.cc  */
#line 1010 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rLValue >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3535 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 131:
/* Line 828 of lalr1.cc  */
#line 1011 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rLValue >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3543 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 132:
/* Line 828 of lalr1.cc  */
#line 1015 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(get_slot, yylhs.location, yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3551 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 133:
/* Line 828 of lalr1.cc  */
#line 1016 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(get_slot, yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3559 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 134:
/* Line 828 of lalr1.cc  */
#line 1020 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3567 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 135:
/* Line 828 of lalr1.cc  */
#line 1025 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(binding, yylhs.location, false, yystack_[0].location, yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3577 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 136:
/* Line 828 of lalr1.cc  */
#line 1029 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(binding, yylhs.location, true, yystack_[0].location, yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3587 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 137:
/* Line 828 of lalr1.cc  */
#line 1036 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rLValue >();
  }
/* Line 828 of lalr1.cc  */
#line 3597 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 138:
/* Line 828 of lalr1.cc  */
#line 1040 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = yystack_[1].value.as< ast::rLValue >();
    yylhs.value.as< ast::rExp >().unchecked_cast<ast::LValueArgs>()->arguments_set(yystack_[0].value.as< ast::exps_type* >());
    yylhs.value.as< ast::rExp >()->location_set(yylhs.location);
  }
/* Line 828 of lalr1.cc  */
#line 3609 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 139:
/* Line 828 of lalr1.cc  */
#line 1053 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    // Compiled as "id . new (args)".
    ast::exps_type* args = yystack_[0].value.as< ast::exps_type* >();
    if (!args)
      args = new ast::exps_type();
    yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, MAKE(call, yylhs.location, yystack_[1].value.as< libport::Symbol >()), SYMBOL(new), args);
    up.deprecated(yylhs.location, "new Obj(x)", "Obj.new(x)");
  }
/* Line 828 of lalr1.cc  */
#line 3624 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 140:
/* Line 828 of lalr1.cc  */
#line 1064 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3632 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 141:
/* Line 828 of lalr1.cc  */
#line 1069 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3640 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 142:
/* Line 828 of lalr1.cc  */
#line 1080 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(routine, yylhs.location, yystack_[2].value.as< bool >(), yystack_[1].location, yystack_[1].value.as< ::ast::Formals* >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3650 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 143:
/* Line 828 of lalr1.cc  */
#line 1096 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(float, yylhs.location, yystack_[0].value.as< libport::ufloat >()); }
/* Line 828 of lalr1.cc  */
#line 3658 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 144:
/* Line 828 of lalr1.cc  */
#line 1106 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< libport::ufloat >() = yystack_[0].value.as< libport::ufloat >();      }
/* Line 828 of lalr1.cc  */
#line 3666 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 145:
/* Line 828 of lalr1.cc  */
#line 1107 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< libport::ufloat >() = yystack_[1].value.as< libport::ufloat >() + yystack_[0].value.as< libport::ufloat >(); }
/* Line 828 of lalr1.cc  */
#line 3674 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 146:
/* Line 828 of lalr1.cc  */
#line 1121 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::dictionary_elt_type >().first = yystack_[2].value.as< ast::rExp >();
    yylhs.value.as< ast::dictionary_elt_type >().second = yystack_[0].value.as< ast::rExp >();
  }
/* Line 828 of lalr1.cc  */
#line 3685 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 147:
/* Line 828 of lalr1.cc  */
#line 1129 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    assocs_add(up, yystack_[0].location, yylhs.value.as< ast::dictionary_elts_type >(), yystack_[0].value.as< ast::dictionary_elt_type >());
  }
/* Line 828 of lalr1.cc  */
#line 3695 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 148:
/* Line 828 of lalr1.cc  */
#line 1133 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    std::swap(yylhs.value.as< ast::dictionary_elts_type >(), yystack_[2].value.as< ast::dictionary_elts_type >());
    assocs_add(up, yystack_[0].location, yylhs.value.as< ast::dictionary_elts_type >(), yystack_[0].value.as< ast::dictionary_elt_type >());
  }
/* Line 828 of lalr1.cc  */
#line 3706 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 149:
/* Line 828 of lalr1.cc  */
#line 1140 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { /* nothing */ }
/* Line 828 of lalr1.cc  */
#line 3714 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 150:
/* Line 828 of lalr1.cc  */
#line 1141 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::dictionary_elts_type >(), yystack_[1].value.as< ast::dictionary_elts_type >()); }
/* Line 828 of lalr1.cc  */
#line 3722 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 151:
/* Line 828 of lalr1.cc  */
#line 1146 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rDictionary >() = new ast::Dictionary(yylhs.location, yystack_[1].value.as< ast::dictionary_elts_type >()); }
/* Line 828 of lalr1.cc  */
#line 3730 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 152:
/* Line 828 of lalr1.cc  */
#line 1157 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 3738 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 153:
/* Line 828 of lalr1.cc  */
#line 1158 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3746 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 154:
/* Line 828 of lalr1.cc  */
#line 1159 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3754 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 155:
/* Line 828 of lalr1.cc  */
#line 1163 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = yystack_[1].value.as< ast::exps_type* >(); }
/* Line 828 of lalr1.cc  */
#line 3762 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 156:
/* Line 828 of lalr1.cc  */
#line 1171 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());  }
/* Line 828 of lalr1.cc  */
#line 3770 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 157:
/* Line 828 of lalr1.cc  */
#line 1172 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(float, yylhs.location, yystack_[0].value.as< libport::ufloat >());  }
/* Line 828 of lalr1.cc  */
#line 3778 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 158:
/* Line 828 of lalr1.cc  */
#line 1173 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(float, yylhs.location, yystack_[0].value.as< libport::ufloat >());  }
/* Line 828 of lalr1.cc  */
#line 3786 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 159:
/* Line 828 of lalr1.cc  */
#line 1174 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(string, yylhs.location, yystack_[0].value.as< std::string >()); }
/* Line 828 of lalr1.cc  */
#line 3794 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 160:
/* Line 828 of lalr1.cc  */
#line 1175 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(list, yylhs.location, yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3802 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 161:
/* Line 828 of lalr1.cc  */
#line 1176 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rDictionary >(); }
/* Line 828 of lalr1.cc  */
#line 3810 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 162:
/* Line 828 of lalr1.cc  */
#line 1177 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(tuple, yylhs.location, yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3818 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 163:
/* Line 828 of lalr1.cc  */
#line 1183 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< std::string >(), yystack_[0].value.as< std::string >());  }
/* Line 828 of lalr1.cc  */
#line 3826 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 164:
/* Line 828 of lalr1.cc  */
#line 1184 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< std::string >(), yystack_[1].value.as< std::string >()); yylhs.value.as< std::string >() += yystack_[0].value.as< std::string >(); }
/* Line 828 of lalr1.cc  */
#line 3834 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 165:
/* Line 828 of lalr1.cc  */
#line 1192 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(position, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3842 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 166:
/* Line 828 of lalr1.cc  */
#line 1203 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::EventMatch >() = MAKE(event_match, yylhs.location, yystack_[4].value.as< ast::rExp >(), yystack_[2].value.as< ast::exps_type* >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3852 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 167:
/* Line 828 of lalr1.cc  */
#line 1210 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3860 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 168:
/* Line 828 of lalr1.cc  */
#line 1211 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3868 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 169:
/* Line 828 of lalr1.cc  */
#line 1216 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3876 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 170:
/* Line 828 of lalr1.cc  */
#line 1217 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3884 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 171:
/* Line 828 of lalr1.cc  */
#line 1227 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::Subscript(yylhs.location, yystack_[1].value.as< ast::exps_type* >(), yystack_[3].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3894 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 172:
/* Line 828 of lalr1.cc  */
#line 1242 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::This(yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3902 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 173:
/* Line 828 of lalr1.cc  */
#line 1243 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::CallMsg(yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3910 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 174:
/* Line 828 of lalr1.cc  */
#line 1247 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3918 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 175:
/* Line 828 of lalr1.cc  */
#line 1248 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3926 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 176:
/* Line 828 of lalr1.cc  */
#line 1249 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3934 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 177:
/* Line 828 of lalr1.cc  */
#line 1250 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3942 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 178:
/* Line 828 of lalr1.cc  */
#line 1255 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3950 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 179:
/* Line 828 of lalr1.cc  */
#line 1256 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Decrementation(yylhs.location, yystack_[0].value.as< ast::rLValue >(), false); }
/* Line 828 of lalr1.cc  */
#line 3958 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 180:
/* Line 828 of lalr1.cc  */
#line 1257 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Incrementation(yylhs.location, yystack_[0].value.as< ast::rLValue >(), false); }
/* Line 828 of lalr1.cc  */
#line 3966 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 181:
/* Line 828 of lalr1.cc  */
#line 1258 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 3974 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 182:
/* Line 828 of lalr1.cc  */
#line 1259 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 3982 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 183:
/* Line 828 of lalr1.cc  */
#line 1260 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 3990 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 184:
/* Line 828 of lalr1.cc  */
#line 1261 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 3998 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 185:
/* Line 828 of lalr1.cc  */
#line 1286 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4006 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 186:
/* Line 828 of lalr1.cc  */
#line 1287 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4014 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 187:
/* Line 828 of lalr1.cc  */
#line 1288 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4022 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 188:
/* Line 828 of lalr1.cc  */
#line 1289 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4030 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 189:
/* Line 828 of lalr1.cc  */
#line 1290 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4038 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 190:
/* Line 828 of lalr1.cc  */
#line 1291 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4046 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 191:
/* Line 828 of lalr1.cc  */
#line 1292 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4054 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 192:
/* Line 828 of lalr1.cc  */
#line 1293 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4062 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 193:
/* Line 828 of lalr1.cc  */
#line 1294 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4070 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 194:
/* Line 828 of lalr1.cc  */
#line 1295 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4078 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 195:
/* Line 828 of lalr1.cc  */
#line 1296 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4086 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 196:
/* Line 828 of lalr1.cc  */
#line 1322 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4094 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 197:
/* Line 828 of lalr1.cc  */
#line 1323 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4102 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 198:
/* Line 828 of lalr1.cc  */
#line 1324 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4110 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 199:
/* Line 828 of lalr1.cc  */
#line 1325 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4118 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 200:
/* Line 828 of lalr1.cc  */
#line 1326 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4126 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 201:
/* Line 828 of lalr1.cc  */
#line 1327 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4134 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 202:
/* Line 828 of lalr1.cc  */
#line 1328 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4142 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 203:
/* Line 828 of lalr1.cc  */
#line 1329 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4150 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 204:
/* Line 828 of lalr1.cc  */
#line 1330 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4158 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 205:
/* Line 828 of lalr1.cc  */
#line 1331 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4166 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 206:
/* Line 828 of lalr1.cc  */
#line 1335 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(relation, yylhs.location, yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ::ast::Factory::relations_type >()); }
/* Line 828 of lalr1.cc  */
#line 4174 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 207:
/* Line 828 of lalr1.cc  */
#line 1340 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { /* empty */ }
/* Line 828 of lalr1.cc  */
#line 4182 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 208:
/* Line 828 of lalr1.cc  */
#line 1341 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Factory::relations_type >(), MAKE(relation, yystack_[2].value.as< ::ast::Factory::relations_type >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >())); }
/* Line 828 of lalr1.cc  */
#line 4190 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 209:
/* Line 828 of lalr1.cc  */
#line 1355 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4198 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 210:
/* Line 828 of lalr1.cc  */
#line 1356 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(and, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4206 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 211:
/* Line 828 of lalr1.cc  */
#line 1357 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(or,  yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4214 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 212:
/* Line 828 of lalr1.cc  */
#line 1362 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), SYMBOL(has),    yystack_[2].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4222 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 213:
/* Line 828 of lalr1.cc  */
#line 1363 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), SYMBOL(hasNot), yystack_[3].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4230 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 214:
/* Line 828 of lalr1.cc  */
#line 1367 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 4238 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 215:
/* Line 828 of lalr1.cc  */
#line 1368 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4246 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 216:
/* Line 828 of lalr1.cc  */
#line 1379 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< unsigned >() = static_cast<unsigned int>(yystack_[0].value.as< libport::ufloat >()); }
/* Line 828 of lalr1.cc  */
#line 4254 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 217:
/* Line 828 of lalr1.cc  */
#line 1388 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::Unscope(yylhs.location, yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4264 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 218:
/* Line 828 of lalr1.cc  */
#line 1400 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::MetaExp(yylhs.location, yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4274 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 219:
/* Line 828 of lalr1.cc  */
#line 1408 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::MetaLValue(yylhs.location, new ast::exps_type(), yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4284 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 220:
/* Line 828 of lalr1.cc  */
#line 1416 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::MetaId(yylhs.location, 0, yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4294 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 221:
/* Line 828 of lalr1.cc  */
#line 1420 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::MetaCall(yylhs.location, 0, yystack_[3].value.as< ast::rExp >(), yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4304 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 222:
/* Line 828 of lalr1.cc  */
#line 1428 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    {
    assert(yystack_[4].value.as< ast::rLValue >().unsafe_cast<ast::LValueArgs>());
    assert(!yystack_[4].value.as< ast::rLValue >().unsafe_cast<ast::LValueArgs>()->arguments_get());
    yylhs.value.as< ast::rExp >() = new ast::MetaArgs(yylhs.location, yystack_[4].value.as< ast::rLValue >(), yystack_[1].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4316 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 223:
/* Line 828 of lalr1.cc  */
#line 1444 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 4324 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 224:
/* Line 828 of lalr1.cc  */
#line 1445 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4332 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 225:
/* Line 828 of lalr1.cc  */
#line 1449 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4340 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 226:
/* Line 828 of lalr1.cc  */
#line 1450 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 4348 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 227:
/* Line 828 of lalr1.cc  */
#line 1456 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 4356 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 228:
/* Line 828 of lalr1.cc  */
#line 1457 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4364 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 229:
/* Line 828 of lalr1.cc  */
#line 1461 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4372 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 230:
/* Line 828 of lalr1.cc  */
#line 1462 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 4380 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 231:
/* Line 828 of lalr1.cc  */
#line 1466 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 4388 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 232:
/* Line 828 of lalr1.cc  */
#line 1471 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4396 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 233:
/* Line 828 of lalr1.cc  */
#line 1475 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = 0; }
/* Line 828 of lalr1.cc  */
#line 4404 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 234:
/* Line 828 of lalr1.cc  */
#line 1476 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4412 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 235:
/* Line 828 of lalr1.cc  */
#line 1486 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { /* empty */ }
/* Line 828 of lalr1.cc  */
#line 4420 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 236:
/* Line 828 of lalr1.cc  */
#line 1487 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::symbols_type >(), yystack_[1].value.as< ::ast::symbols_type >()); yylhs.value.as< ::ast::symbols_type >().push_back(yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4428 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 237:
/* Line 828 of lalr1.cc  */
#line 1492 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::rExp >() = yystack_[0].value.as< ast::rExp >();}
/* Line 828 of lalr1.cc  */
#line 4436 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 238:
/* Line 828 of lalr1.cc  */
#line 1497 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::rExp >()=0;}
/* Line 828 of lalr1.cc  */
#line 4444 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 239:
/* Line 828 of lalr1.cc  */
#line 1498 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::rExp >(), yystack_[0].value.as< ::ast::rExp >());}
/* Line 828 of lalr1.cc  */
#line 4452 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 240:
/* Line 828 of lalr1.cc  */
#line 1503 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formal >() = ::ast::Formal(yystack_[1].value.as< libport::Symbol >(), 0, yystack_[0].value.as< ::ast::rExp >());  }
/* Line 828 of lalr1.cc  */
#line 4460 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 241:
/* Line 828 of lalr1.cc  */
#line 1504 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formal >() = ::ast::Formal(yystack_[3].value.as< libport::Symbol >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ::ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4468 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 242:
/* Line 828 of lalr1.cc  */
#line 1505 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formal >() = ::ast::Formal(yystack_[2].value.as< libport::Symbol >(), true); }
/* Line 828 of lalr1.cc  */
#line 4476 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 243:
/* Line 828 of lalr1.cc  */
#line 1511 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formals* >() = new ::ast::Formals(1, yystack_[0].value.as< ::ast::Formal >()); }
/* Line 828 of lalr1.cc  */
#line 4484 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 244:
/* Line 828 of lalr1.cc  */
#line 1512 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Formals* >(), yystack_[2].value.as< ::ast::Formals* >()); *yylhs.value.as< ::ast::Formals* >() << yystack_[0].value.as< ::ast::Formal >(); }
/* Line 828 of lalr1.cc  */
#line 4492 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 245:
/* Line 828 of lalr1.cc  */
#line 1517 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formals* >() = new ::ast::Formals; }
/* Line 828 of lalr1.cc  */
#line 4500 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 246:
/* Line 828 of lalr1.cc  */
#line 1518 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Formals* >(), yystack_[1].value.as< ::ast::Formals* >()); }
/* Line 828 of lalr1.cc  */
#line 4508 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 247:
/* Line 828 of lalr1.cc  */
#line 1523 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formals* >() = 0; }
/* Line 828 of lalr1.cc  */
#line 4516 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;

  case 248:
/* Line 828 of lalr1.cc  */
#line 1524 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Formals* >(), yystack_[1].value.as< ::ast::Formals* >()); }
/* Line 828 of lalr1.cc  */
#line 4524 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
    break;


/* Line 828 of lalr1.cc  */
#line 4529 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
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


  const short int parser::yypact_ninf_ = -297;

  const short int parser::yytable_ninf_ = -254;

  const short int
  parser::yypact_[] =
  {
     162,  -297,   691,  1084,    23,  -297,    58,  -297,  -297,  -297,
       6,  -297,    77,  -297,    98,   114,   904,  1256,   988,  1408,
     127,   164,  1408,   169,   124,  1484,   193,   202,   207,   -24,
     220,   231,  1084,   233,  -297,  -297,  1678,  1678,   -24,    43,
    1678,  1678,  1678,   235,   130,  -297,  -297,   171,  -297,  -297,
    -297,  -297,  -297,  -297,  1636,  1636,  1636,  1636,   200,   200,
     200,   200,  -297,   119,   154,  -297,  -297,   427,    15,    51,
     181,   500,   -24,   524,  -297,  -297,   205,  -297,  -297,  -297,
     199,  -297,  -297,  -297,   194,  -297,  -297,  -297,  -297,  -297,
    1484,  1332,  1084,   -28,   256,   108,    -9,  -297,    36,   260,
      -7,  -297,  -297,   248,   267,   269,   252,   270,    32,   275,
     285,  -297,   442,  -297,  1332,  1332,  -297,  1332,    28,    35,
     524,  1332,  1332,  1332,  -297,  -297,  1332,  1180,  -297,  1332,
      33,    -7,   535,   535,   289,  -297,   258,   278,   307,   504,
     504,   504,  1332,  1332,  1332,   309,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  1084,  1084,
    1332,  1332,   167,  1332,  1332,     3,  -297,   311,   198,   124,
    1084,  1332,   -26,  1678,  1332,  -297,   767,  1332,  1332,  1332,
    1332,  1332,  1332,  -297,  -297,   -24,  -297,   260,  1484,  1484,
    1484,  1484,  1484,  1484,  1484,  1484,  1484,  1484,  1351,  -297,
    -297,  1084,  1084,   524,   111,    46,    66,  -297,  -297,   -24,
    1332,   320,  1332,  -297,  -297,  -297,  1332,  -297,  -297,  -297,
    -297,  1332,   139,   166,   210,   322,   124,   109,  -297,    26,
     314,   228,    26,   319,   266,  1560,   310,  -297,   271,   279,
    1332,  -297,   197,   124,   124,   -24,   328,  -297,   200,   286,
     442,   330,   317,   304,  1332,  -297,  -297,  -297,   318,   282,
       0,  1332,   326,     1,    -5,  -297,  -297,   323,   335,   331,
     332,   333,   124,  -297,  -297,   442,   341,   -24,  -297,   200,
    -297,    -7,   321,   200,   354,   442,   442,   442,   442,   442,
     442,  -297,   124,   739,   562,   573,   120,   120,   147,  -297,
     147,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  1484,  -297,  -297,  1084,  1084,  -297,   370,
     442,    36,  -297,   442,   347,  1084,   376,  1084,  1332,   124,
    -297,  1084,   384,  -297,  1332,   309,   371,  -297,  -297,   372,
    1084,  1084,   135,  1332,  1084,  1084,    26,   373,  -297,  -297,
    -297,  1332,  -297,   366,  -297,  -297,   381,   374,  -297,   364,
     387,   124,  -297,  1332,  -297,  -297,   405,  -297,   375,     0,
    -297,     3,  -297,  -297,    22,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,   392,  -297,  -297,   524,  -297,   411,  -297,  -297,
    -297,   418,   284,   400,  -297,  -297,   124,  -297,   442,   326,
    -297,  1084,   411,  -297,  -297,  -297,  1332,   415,  -297,  -297,
     404,  1084,   442,   197,  -297,   -24,  -297,   391,   393,  -297,
     442,  1332,  -297,  -297,  1332,  1332,   409,  -297,  -297,  -297,
    -297,   122,   124,   411,  1332,  -297,  -297,   425,   411,  -297,
     430,  1084,  1084,   421,  -297,  -297,  -297,   399,   433,   442,
     432,   442,  -297,  1332,   438,   426,  -297,  -297,   384,   442,
    1332,  -297,  -297,  1084,   435,   421,  1084,  -297,  -297,   406,
    -297,   450,  1084,  -297,  -297,   442,  -297,  1084,  -297,  -297,
     391,  1084,   194,  -297,   416,   194,  -297
  };

  const unsigned char
  parser::yydefact_[] =
  {
       0,     3,     0,    16,     0,     2,     0,   165,    83,    53,
       0,    84,     0,    54,     0,     0,     0,   227,     0,   214,
       0,     0,   214,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   235,   134,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   121,   122,   141,   157,   144,
     143,   163,   173,   172,     0,     0,     0,     0,     0,     0,
       0,     0,     4,     0,    17,    19,   119,    25,   247,   178,
       0,   137,   129,   207,   140,   156,   158,   161,   162,   174,
     159,   185,   209,     5,    12,    13,     1,    11,    10,     9,
       0,     0,    16,     0,     0,     0,   129,   149,   229,   247,
     178,   129,   147,   249,     0,     0,   249,     0,   229,     0,
       0,   154,   215,    82,     0,     0,   111,     0,     0,   137,
     135,     0,     0,     0,   141,   132,     0,    22,   112,     0,
       0,     0,   137,   137,     0,    46,     0,    47,     0,    51,
     179,   180,     0,   223,     0,   233,   183,   184,   182,   181,
     216,   217,   218,   219,   220,     8,     7,     6,     0,    18,
       0,     0,   233,     0,     0,   245,    57,     0,   247,     0,
       0,   227,     0,     0,     0,   126,   227,     0,     0,     0,
       0,     0,     0,    71,    72,     0,   138,   247,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   206,   145,
     164,    16,    16,   136,     0,     0,     0,    27,    26,     0,
       0,     0,   250,   150,   151,   160,   250,   228,   176,   175,
     155,   153,     0,     0,     0,   102,     0,    91,    98,   169,
       0,     0,   169,     0,     0,     0,     0,    23,    25,     0,
       0,   236,    29,     0,     0,    40,     0,    48,     0,     0,
     225,     0,   251,     0,   227,   234,   139,    21,    20,    63,
     212,     0,   169,   210,   211,   254,   243,   249,     0,     0,
       0,     0,     0,   142,    24,   229,     0,     0,   131,     0,
     130,   177,     0,     0,     0,    69,    66,    70,    65,    68,
      67,    73,     0,   193,   194,   191,   195,   192,   187,   190,
     186,   189,   188,   202,   200,   201,   204,   203,   199,   198,
     196,   197,   205,     0,    15,    14,     0,     0,   127,     0,
     146,     0,   148,   230,   230,     0,     0,     0,     0,     0,
     110,     0,   107,    99,     0,   233,     0,    86,   128,     0,
       0,     0,   134,     0,     0,     0,   169,     0,    30,    31,
      32,     0,    34,    37,    38,    39,     0,   249,    42,     0,
       0,     0,   124,   252,   224,   123,     0,    61,    64,   213,
      52,   250,   246,   248,   238,    58,    59,    55,   171,   133,
     221,   125,     0,   232,    56,   208,    78,    91,    28,    79,
      95,   105,   100,     0,   104,    92,     0,   109,   170,   169,
      85,     0,    91,    76,   118,   117,     0,     0,   113,   116,
       0,     0,    33,    29,    44,   250,    41,     0,     0,   120,
     226,     0,    62,   244,     0,     0,     0,   239,   240,   222,
      77,    89,     0,    91,     0,   103,   108,   167,    91,    88,
       0,    22,     0,    93,    35,    43,    45,     0,     0,    60,
     238,   237,   242,     0,     0,     0,    96,   106,   107,   101,
       0,   166,    87,     0,     0,    93,     0,    75,    49,     0,
     241,     0,    16,    80,    81,   168,   115,     0,    74,    94,
       0,    16,    90,   114,     0,    97,    50
  };

  const short int
  parser::yypgoto_[] =
  {
    -297,  -297,  -297,  -297,  -297,   -14,     7,   112,    27,   160,
     -16,  -297,    56,  -297,   338,   464,  -297,  -297,    -6,  -297,
      79,   385,   116,  -297,    31,  -297,  -296,    24,  -297,  -297,
    -297,    39,  -297,   261,  -297,    37,  -297,  -297,    72,    -2,
     -15,  -297,  -297,  -297,   281,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -119,  -297,  -214,   265,  -297,  -297,  -297,   474,
     -54,  -297,  -297,    -4,   480,  -297,  -133,  -151,  -297,  -297,
      52,   136,  -297,  -297,  -137,  -103,  -297,  -297
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     4,     5,    62,    83,    84,    85,    64,   236,    65,
      66,   351,   352,   353,   243,    67,   356,   357,   447,   138,
      99,   168,   367,   368,   100,   455,   332,   467,   431,   456,
     227,   393,   329,   228,   433,   397,   406,    70,    71,   101,
      73,    74,    75,    76,   102,   103,   104,    77,   109,    78,
      79,    80,   230,   461,   336,    81,   313,    82,   198,   113,
     151,   251,   252,   284,   106,   111,   186,   256,   130,   427,
     428,   266,   267,   268,   169,   213,   364,   269
  };

  const short int
  parser::yytable_[] =
  {
      72,    72,    95,   217,   233,   152,   153,   154,   118,    63,
     120,   262,   255,   105,    96,   161,   171,   277,   339,   172,
    -254,   161,   166,    86,    35,   209,    35,   125,   424,   255,
      72,   272,   160,    69,    69,   425,   134,   225,   160,   165,
     265,    35,   160,    90,   226,   426,   161,    69,   370,   124,
     292,   124,   161,  -253,   175,   167,   161,   240,    87,   176,
     334,   135,   219,    69,   170,   166,   124,   131,   131,   278,
     187,   131,   131,   131,   171,   203,   317,   172,   205,   211,
     136,    68,    68,   241,   211,   211,   279,   201,   202,   173,
      72,   430,   271,   137,   187,    68,   318,   119,   167,    88,
      89,    91,   163,   183,   184,   185,   439,   335,   132,   133,
     211,    68,   139,   140,   141,   210,   211,   160,   225,   155,
     211,   347,    92,    69,   331,    72,   119,   119,   119,   119,
     453,   161,   410,   163,   164,   208,   454,   458,    93,   163,
     164,   316,   462,   163,   164,   160,    16,   173,   404,   201,
     202,   114,   143,   273,   144,   405,    72,    72,    69,   161,
     156,   157,   119,     1,   372,     2,     3,   276,    72,   325,
     280,    68,   160,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   291,   399,   437,   161,   261,   115,    69,
      69,   254,   128,   117,   360,   211,   326,   158,   159,    72,
      72,    69,   255,    16,   281,   174,    68,   319,   314,   315,
     330,   193,   194,   195,   196,   197,   160,   121,   163,   164,
     120,   145,   165,   211,   271,   380,   122,   354,   355,   382,
     161,   123,    69,    69,   160,   201,   202,    68,    68,   194,
     327,   196,   197,   358,   126,   119,   163,   164,   161,    68,
     211,   348,   349,   350,   416,   127,   377,   129,   338,   142,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     257,   258,   160,   163,   164,   379,   384,   160,   150,   200,
      68,    68,   199,   207,   165,   160,   161,   237,   160,   212,
     160,   161,   160,   216,   211,   214,   341,   215,   385,   161,
     218,   344,   161,   434,   161,   220,   161,   119,   246,   345,
     160,   245,   211,   394,    72,    72,   361,   163,   164,   146,
     147,   148,   149,    72,   161,    72,   221,   160,   247,    72,
     274,   248,   366,   254,   365,   163,   164,   270,    72,    72,
     261,   161,    72,    72,   337,   419,   328,    69,    69,   340,
     211,   381,   343,   160,   359,   162,    69,   362,    69,   363,
     334,   158,    69,   211,   371,   373,   211,   161,   211,   378,
     211,    69,    69,   163,   164,    69,    69,  -231,   163,   164,
     436,   374,   375,   376,   383,   119,   163,   164,   211,   163,
     164,   163,   164,   163,   164,    68,    68,   388,   390,    72,
     396,   400,   401,   411,    68,   211,    68,   413,   414,    72,
      68,   163,   164,   445,   417,   415,   457,   418,   421,    68,
      68,   160,   429,    68,    68,   366,   331,   432,   163,   164,
     435,   211,    69,   160,   442,   161,   160,   452,   160,    72,
      72,   446,    69,   448,   460,   425,   466,   161,   160,   468,
     161,   472,   161,   473,   163,   164,   480,   441,   482,   469,
     463,    72,   161,   481,    72,   477,   486,   485,   464,   444,
      72,   244,    69,    69,   484,    72,   386,   387,   206,    72,
      68,    98,   108,   112,   422,   389,   112,   391,   333,   478,
      68,   395,   471,   322,    69,   474,   116,    69,   110,   211,
     402,   403,   470,    69,   408,   409,     0,   423,    69,     0,
       0,   162,    69,     0,   211,     0,   211,     0,     0,     0,
      68,    68,   163,   164,   176,     0,   211,  -137,   176,     0,
    -137,     0,     0,     0,   163,   164,     0,   163,   164,   163,
     164,     0,    68,     0,     0,    68,     0,     0,   242,   163,
     164,    68,     0,     0,     0,   204,    68,   -36,     0,   176,
      68,   438,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   443,   183,   184,   185,     0,     0,     0,   222,   223,
       0,   224,     0,     0,     0,   229,   231,   232,     0,     0,
     234,   238,     0,   239,     0,     0,     0,     0,     0,     0,
    -137,   237,   465,   183,   184,   185,   249,   250,   253,   188,
     189,   190,     0,   191,   192,   193,   194,   195,   196,   197,
       0,     0,     0,   476,   259,   260,   479,   263,   264,     0,
       0,     0,     0,     0,     0,   275,     0,   483,   282,     0,
     275,   285,   286,   287,   288,   289,   290,   188,     0,   190,
       0,   191,   192,   193,   194,   195,   196,   197,   188,     0,
       0,     0,   191,   192,   193,   194,   195,   196,   197,     0,
       0,     0,     0,     0,   320,     0,   321,     0,     0,     0,
     323,     0,     0,     0,     0,   324,     0,     0,     0,     0,
       0,   -16,     6,     0,     0,     0,     7,     0,     8,     0,
       0,     9,    10,    11,   346,     0,     0,     0,    12,    13,
      14,     0,    15,    16,    17,    18,     0,     0,   275,     0,
      19,     0,    20,    21,    22,   369,    23,    24,    25,    26,
      27,    28,   -16,   -16,    29,     0,    30,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,     0,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,    41,
      42,     0,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,     7,    52,    53,    54,     0,     9,    10,    55,
       0,     0,    56,     0,    57,    13,     0,     0,    15,    16,
      17,    18,   392,     0,     0,     0,     0,     0,   398,     0,
      58,    59,    60,    61,    25,     0,    27,   407,     0,     0,
      29,     0,     0,     0,     0,   412,     0,    35,     0,     0,
       0,     0,     0,     0,    36,    37,     0,   420,   191,   192,
     193,   194,   195,   196,   197,    41,    42,     0,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,     0,    52,
      53,    54,     0,     0,     0,    55,     0,     0,    56,     0,
      57,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     440,     0,     0,     0,     0,     0,    58,    59,    60,    61,
     283,     0,     0,     0,     0,   449,     0,     0,   450,   451,
       0,     0,     0,     0,     0,     0,     0,     0,   459,     0,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     7,
       0,     8,     0,     0,     9,    10,    11,   392,     0,     0,
       0,    12,    13,    14,   475,    15,    16,    17,    18,     0,
       0,   -16,     0,    19,     0,    20,    21,    22,     0,    23,
      24,    25,    26,    27,    28,   -16,   -16,    29,     0,    30,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
       0,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    41,    42,     0,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,     0,    52,    53,    54,   107,
       0,     0,    55,     7,     0,    56,     0,    57,     9,    10,
       0,     0,     0,     0,     0,     0,    13,     0,     0,    15,
      16,    17,    18,    58,    59,    60,    61,     0,  -152,     0,
       0,     0,     0,     0,     0,    25,     0,    27,     0,     0,
       0,    29,     0,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,     0,     0,    36,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    41,    42,     0,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,     0,
      52,    53,    54,     0,     0,     0,    55,     0,     0,    56,
       0,    57,     0,     0,     0,     0,     0,     0,     0,     7,
       0,     8,     0,     0,     9,    10,    11,    58,    59,    60,
      61,    12,    13,    14,     0,    15,    16,    17,    18,     0,
       0,     0,     0,    19,     0,    20,    21,    22,     0,    23,
      24,    25,    26,    27,    28,     0,     0,    29,     0,    30,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
       0,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    41,    42,     0,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,     0,    52,    53,    54,     0,
       0,     0,    55,     0,     0,    56,     0,    57,     0,     0,
       0,     0,     0,     0,     0,     7,     0,     8,     0,     0,
       9,    10,    11,    58,    59,    60,    61,    12,    13,    14,
       0,    15,    16,    17,    18,     0,     0,     0,     0,    19,
       0,    20,    21,    22,     0,    23,    24,   235,    26,    27,
      28,     0,     0,    29,     0,    30,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,     0,    36,    37,    38,
      39,    40,     0,     0,     0,     0,     0,     0,    41,    42,
       0,    43,    44,    45,    46,    47,    48,    49,    50,     0,
      51,     7,    52,    53,    54,     0,     9,    10,    55,     0,
       0,    56,     0,    57,    13,     0,     0,    15,    16,    17,
      18,     0,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,    61,    25,     0,    27,     0,     0,     0,    29,
       0,     0,     0,     0,     0,     0,    35,     0,     0,     0,
       0,     0,     0,    36,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    41,    42,     0,    43,    44,    45,
      46,    47,    48,    49,    50,    97,    51,     7,    52,    53,
      54,     0,     9,    10,    55,     0,     0,    56,     0,    57,
      13,     0,     0,    15,    16,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,    61,    25,
       0,    27,     0,     0,     0,    29,     0,     0,     0,     0,
       0,     0,    35,     0,     0,     0,     0,     0,     0,    36,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      41,    42,     0,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,     7,    52,    53,    54,     0,     9,    10,
      55,     0,     0,    56,     0,    57,    13,     0,     0,    15,
      16,    17,    18,     0,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,    61,    25,     0,    27,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,    35,     0,
       0,     0,     0,     0,     0,    36,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    41,    42,     0,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,     7,
      52,    53,    54,     0,     9,    10,    55,     0,     0,    56,
       0,    57,    13,     0,     0,    15,    16,    17,    18,     0,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
      61,    25,     0,    27,     0,     0,     0,    29,     0,     0,
       0,     0,     0,     0,    35,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    41,    42,     0,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,     7,    52,    53,    54,     0,
       9,    10,    55,     0,     0,    56,     0,    57,    13,     0,
       0,    15,    16,    17,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    59,    60,    61,    25,     0,    27,
       0,     0,     0,    29,     0,     0,     0,     0,     0,     0,
     342,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    41,    42,
       0,    43,    44,    45,    46,    47,    48,    49,    50,     0,
      51,     7,    52,    53,    54,     0,     9,     0,    55,     0,
       0,    56,     0,    57,    13,     0,     0,    15,    16,    17,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,     0,     0,    27,     0,     0,     0,    29,
       0,     0,     0,     7,     0,     0,    35,     0,     9,     0,
       0,     0,     0,     0,     0,     0,    13,     0,     0,    15,
      16,    17,    18,     0,    41,    42,     0,    43,    44,    45,
      46,   124,    48,    49,    50,     0,    51,    27,    52,    53,
      54,    29,     0,     0,    55,     0,     0,    56,    35,    57,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,    43,
      44,    45,    46,   124,    48,    49,    50,     0,    51,     0,
      52,    53,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61
  };

  const short int
  parser::yycheck_[] =
  {
       2,     3,    16,   106,   123,    59,    60,    61,    24,     2,
      25,   162,   145,    17,    16,    20,    23,    43,   232,    26,
      20,    20,    50,     0,    50,    34,    50,    29,     6,   162,
      32,   168,     6,     2,     3,    13,    38,     9,     6,    24,
      37,    50,     6,    37,    16,    23,    20,    16,   262,    75,
     187,    75,    20,    50,    70,    83,    20,    24,     0,    24,
      34,    18,    30,    32,    13,    50,    75,    36,    37,    95,
      72,    40,    41,    42,    23,    90,    30,    26,    92,    84,
      37,     2,     3,    50,    84,    84,   112,    41,    42,    96,
      92,   387,    26,    50,    96,    16,    30,    25,    83,    41,
      42,    24,   107,    68,    69,    70,   402,    81,    36,    37,
      84,    32,    40,    41,    42,    79,    84,     6,     9,     0,
      84,   240,    24,    92,    15,   127,    54,    55,    56,    57,
       8,    20,   346,   107,   108,    27,    14,   433,    24,   107,
     108,    30,   438,   107,   108,     6,    22,    96,    13,    41,
      42,    24,    22,   169,    24,    20,   158,   159,   127,    20,
      41,    42,    90,     1,   267,     3,     4,   171,   170,    30,
     172,    92,     6,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   185,   335,   399,    20,    20,    24,   158,
     159,    24,    32,    24,   248,    84,    30,    43,    44,   201,
     202,   170,   335,    22,   173,    24,   127,   209,   201,   202,
     226,    91,    92,    93,    94,    95,     6,    24,   107,   108,
     235,    50,    24,    84,    26,   279,    24,   243,   244,   283,
      20,    24,   201,   202,     6,    41,    42,   158,   159,    92,
      30,    94,    95,   245,    24,   173,   107,   108,    20,   170,
      84,    54,    55,    56,   357,    24,   272,    24,    30,    24,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     158,   159,     6,   107,   108,   277,   292,     6,    78,    80,
     201,   202,    77,    27,    24,     6,    20,   127,     6,    41,
       6,    20,     6,    41,    84,    28,    30,    28,   313,    20,
      30,    30,    20,    19,    20,    30,    20,   235,    50,    30,
       6,    22,    84,   329,   316,   317,    30,   107,   108,    54,
      55,    56,    57,   325,    20,   327,    41,     6,    50,   331,
     170,    24,    50,    24,    30,   107,   108,    26,   340,   341,
      20,    20,   344,   345,    30,   361,    24,   316,   317,    30,
      84,    30,    42,     6,    26,    84,   325,    27,   327,    42,
      34,    43,   331,    84,    41,    30,    84,    20,    84,    28,
      84,   340,   341,   107,   108,   344,   345,    30,   107,   108,
     396,    50,    50,    50,    30,   313,   107,   108,    84,   107,
     108,   107,   108,   107,   108,   316,   317,    27,    22,   401,
      16,    30,    30,    30,   325,    84,   327,    41,    27,   411,
     331,   107,   108,   415,    50,    41,   432,    30,    13,   340,
     341,     6,    30,   344,   345,    50,    15,     9,   107,   108,
      30,    84,   401,     6,    30,    20,     6,    28,     6,   441,
     442,    50,   411,    50,    19,    13,    25,    20,     6,    50,
      20,    13,    20,    27,   107,   108,    50,    42,   472,    26,
      30,   463,    20,    13,   466,    30,    50,   481,   441,   413,
     472,   133,   441,   442,   480,   477,   316,   317,    93,   481,
     401,    17,    18,    19,   368,   325,    22,   327,   227,   465,
     411,   331,   453,   212,   463,   458,    22,   466,    18,    84,
     340,   341,   450,   472,   344,   345,    -1,   371,   477,    -1,
      -1,    84,   481,    -1,    84,    -1,    84,    -1,    -1,    -1,
     441,   442,   107,   108,    24,    -1,    84,    23,    24,    -1,
      26,    -1,    -1,    -1,   107,   108,    -1,   107,   108,   107,
     108,    -1,   463,    -1,    -1,   466,    -1,    -1,    13,   107,
     108,   472,    -1,    -1,    -1,    91,   477,    22,    -1,    24,
     481,   401,    62,    63,    64,    65,    66,    67,    68,    69,
      70,   411,    68,    69,    70,    -1,    -1,    -1,   114,   115,
      -1,   117,    -1,    -1,    -1,   121,   122,   123,    -1,    -1,
     126,   127,    -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      96,   441,   442,    68,    69,    70,   142,   143,   144,    85,
      86,    87,    -1,    89,    90,    91,    92,    93,    94,    95,
      -1,    -1,    -1,   463,   160,   161,   466,   163,   164,    -1,
      -1,    -1,    -1,    -1,    -1,   171,    -1,   477,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,    85,    -1,    87,
      -1,    89,    90,    91,    92,    93,    94,    95,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      -1,    -1,    -1,    -1,   210,    -1,   212,    -1,    -1,    -1,
     216,    -1,    -1,    -1,    -1,   221,    -1,    -1,    -1,    -1,
      -1,     0,     1,    -1,    -1,    -1,     5,    -1,     7,    -1,
      -1,    10,    11,    12,   240,    -1,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    -1,    -1,   254,    -1,
      29,    -1,    31,    32,    33,   261,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    -1,    57,    58,
      59,    60,    61,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,     5,    82,    83,    84,    -1,    10,    11,    88,
      -1,    -1,    91,    -1,    93,    18,    -1,    -1,    21,    22,
      23,    24,   328,    -1,    -1,    -1,    -1,    -1,   334,    -1,
     109,   110,   111,   112,    37,    -1,    39,   343,    -1,    -1,
      43,    -1,    -1,    -1,    -1,   351,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    58,    -1,   363,    89,    90,
      91,    92,    93,    94,    95,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    -1,    -1,    88,    -1,    -1,    91,    -1,
      93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     406,    -1,    -1,    -1,    -1,    -1,   109,   110,   111,   112,
     113,    -1,    -1,    -1,    -1,   421,    -1,    -1,   424,   425,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   434,    -1,
      -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,     5,
      -1,     7,    -1,    -1,    10,    11,    12,   453,    -1,    -1,
      -1,    17,    18,    19,   460,    21,    22,    23,    24,    -1,
      -1,    27,    -1,    29,    -1,    31,    32,    33,    -1,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    45,
      46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    58,    59,    60,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    -1,    80,    -1,    82,    83,    84,     1,
      -1,    -1,    88,     5,    -1,    91,    -1,    93,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    18,    -1,    -1,    21,
      22,    23,    24,   109,   110,   111,   112,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    37,    -1,    39,    -1,    -1,
      -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    71,
      72,    73,    74,    75,    76,    77,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    -1,    -1,    88,    -1,    -1,    91,
      -1,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,
      -1,     7,    -1,    -1,    10,    11,    12,   109,   110,   111,
     112,    17,    18,    19,    -1,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    -1,    35,
      36,    37,    38,    39,    40,    -1,    -1,    43,    -1,    45,
      46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    58,    59,    60,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      -1,    -1,    88,    -1,    -1,    91,    -1,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     5,    -1,     7,    -1,    -1,
      10,    11,    12,   109,   110,   111,   112,    17,    18,    19,
      -1,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    -1,    35,    36,    37,    38,    39,
      40,    -1,    -1,    43,    -1,    45,    46,    47,    48,    49,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    57,    58,    59,
      60,    61,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    -1,
      80,     5,    82,    83,    84,    -1,    10,    11,    88,    -1,
      -1,    91,    -1,    93,    18,    -1,    -1,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,
     110,   111,   112,    37,    -1,    39,    -1,    -1,    -1,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,     5,    82,    83,
      84,    -1,    10,    11,    88,    -1,    -1,    91,    -1,    93,
      18,    -1,    -1,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   109,   110,   111,   112,    37,
      -1,    39,    -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    -1,    71,    72,    73,    74,    75,    76,    77,
      78,    -1,    80,     5,    82,    83,    84,    -1,    10,    11,
      88,    -1,    -1,    91,    -1,    93,    18,    -1,    -1,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   109,   110,   111,   112,    37,    -1,    39,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    71,
      72,    73,    74,    75,    76,    77,    78,    -1,    80,     5,
      82,    83,    84,    -1,    10,    11,    88,    -1,    -1,    91,
      -1,    93,    18,    -1,    -1,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,   110,   111,
     112,    37,    -1,    39,    -1,    -1,    -1,    43,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    -1,    80,     5,    82,    83,    84,    -1,
      10,    11,    88,    -1,    -1,    91,    -1,    93,    18,    -1,
      -1,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   110,   111,   112,    37,    -1,    39,
      -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    -1,
      80,     5,    82,    83,    84,    -1,    10,    -1,    88,    -1,
      -1,    91,    -1,    93,    18,    -1,    -1,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     110,   111,   112,    -1,    -1,    39,    -1,    -1,    -1,    43,
      -1,    -1,    -1,     5,    -1,    -1,    50,    -1,    10,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    18,    -1,    -1,    21,
      22,    23,    24,    -1,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    39,    82,    83,
      84,    43,    -1,    -1,    88,    -1,    -1,    91,    50,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   110,   111,   112,    71,
      72,    73,    74,    75,    76,    77,    78,    -1,    80,    -1,
      82,    83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,
     112
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,     1,     3,     4,   115,   116,     1,     5,     7,    10,
      11,    12,    17,    18,    19,    21,    22,    23,    24,    29,
      31,    32,    33,    35,    36,    37,    38,    39,    40,    43,
      45,    46,    47,    48,    49,    50,    57,    58,    59,    60,
      61,    68,    69,    71,    72,    73,    74,    75,    76,    77,
      78,    80,    82,    83,    84,    88,    91,    93,   109,   110,
     111,   112,   117,   120,   121,   123,   124,   129,   134,   138,
     151,   152,   153,   154,   155,   156,   157,   161,   163,   164,
     165,   169,   171,   118,   119,   120,     0,     0,    41,    42,
      37,    24,    24,    24,     1,   119,   153,    79,   129,   134,
     138,   153,   158,   159,   160,   177,   178,     1,   129,   162,
     178,   179,   129,   173,    24,    24,   173,    24,   124,   152,
     154,    24,    24,    24,    75,   153,    24,    24,   123,    24,
     182,   138,   152,   152,   153,    18,    37,    50,   133,   152,
     152,   152,    24,    22,    24,    50,   169,   169,   169,   169,
      78,   174,   174,   174,   174,     0,    41,    42,    43,    44,
       6,    20,    84,   107,   108,    24,    50,    83,   135,   188,
      13,    23,    26,    96,    24,   124,    24,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   180,   153,    85,    86,
      87,    89,    90,    91,    92,    93,    94,    95,   172,    77,
      80,    41,    42,   154,   129,   119,   135,    27,    27,    34,
      79,    84,    41,   189,    28,    28,    41,   189,    30,    30,
      30,    41,   129,   129,   129,     9,    16,   144,   147,   129,
     166,   129,   129,   166,   129,    37,   122,   123,   129,   129,
      24,    50,    13,   128,   128,    22,    50,    50,    24,   129,
     129,   175,   176,   129,    24,   180,   181,   121,   121,   129,
     129,    20,   181,   129,   129,    37,   185,   186,   187,   191,
      26,    26,   188,   124,   123,   129,   177,    43,    95,   112,
     153,   138,   129,   113,   177,   129,   129,   129,   129,   129,
     129,   153,   188,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   170,   120,   120,    30,    30,    30,   153,
     129,   129,   158,   129,   129,    30,    30,    30,    24,   146,
     124,    15,   140,   147,    34,    81,   168,    30,    30,   168,
      30,    30,    50,    42,    30,    30,   129,   166,    54,    55,
      56,   125,   126,   127,   124,   124,   130,   131,   153,    26,
     174,    30,    27,    42,   190,    30,    50,   136,   137,   129,
     168,    41,   189,    30,    50,    50,    50,   124,    28,   153,
     174,    30,   174,    30,   124,   154,   123,   123,    27,   123,
      22,   123,   129,   145,   124,   123,    16,   149,   129,   181,
      30,    30,   123,   123,    13,    20,   150,   129,   123,   123,
     168,    30,   129,    41,    27,    41,   189,    50,    30,   124,
     129,    13,   136,   185,     6,    13,    23,   183,   184,    30,
     140,   142,     9,   148,    19,    30,   124,   168,   123,   140,
     129,    42,    30,   123,   126,   153,    50,   132,    50,   129,
     129,   129,    28,     8,    14,   139,   143,   124,   140,   129,
      19,   167,   140,    30,   122,   123,    25,   141,    50,    26,
     184,   145,    13,    27,   149,   129,   123,    30,   141,   123,
      50,    13,   119,   123,   132,   119,    50
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,   114,   115,   116,   116,   116,   117,   117,   117,   117,
     117,   117,   118,   119,   119,   119,   120,   120,   120,   121,
     121,   121,   122,   122,   123,   123,   124,   124,   124,   125,
     125,   125,   125,   126,   127,   127,   128,   128,   129,   129,
     130,   130,   131,   131,   123,   132,   133,   133,   123,   123,
     123,   123,   123,   134,   134,   123,   123,   135,   135,   135,
     136,   137,   137,   129,   129,   129,   129,   129,   129,   129,
     129,   138,   138,   138,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   139,
     139,   140,   140,   141,   141,   142,   142,   143,   144,   144,
     145,   145,   146,   146,   147,   148,   148,   149,   149,   123,
     123,   123,   123,   123,   123,   123,   123,   150,   150,   138,
     138,   151,   151,   138,   138,   138,   138,   138,   138,   152,
     152,   152,   138,   138,   153,   154,   154,   138,   138,   155,
     154,   153,   138,   156,   157,   157,   158,   159,   159,   160,
     160,   161,   162,   162,   162,   163,   164,   164,   164,   164,
     164,   164,   164,   165,   165,   164,   166,   167,   167,   168,
     168,   152,   164,   164,   138,   138,   138,   138,   169,   169,
     169,   169,   169,   169,   169,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   171,   172,   172,   129,
     129,   129,   129,   129,   173,   173,   174,   129,   138,   152,
     152,   152,   138,   175,   175,   176,   176,   177,   177,   178,
     178,   179,   180,   181,   181,   182,   182,   183,   184,   184,
     185,   185,   185,   186,   186,   187,   187,   188,   188,   189,
     189,   190,   190,   191,   191
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
       2,     3,     0,     2,     1,     3,     1,     1,     1,     1,
       3,     1,     1,     1,     2,     1,     5,     0,     2,     0,
       2,     4,     1,     1,     1,     3,     3,     3,     1,     2,
       2,     2,     2,     2,     2,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     0,     3,     1,
       3,     3,     3,     4,     0,     1,     1,     2,     2,     2,
       2,     4,     5,     0,     2,     1,     3,     0,     2,     1,
       3,     3,     3,     0,     1,     0,     2,     2,     0,     1,
       3,     5,     4,     1,     3,     0,     2,     0,     3,     0,
       1,     0,     1,     0,     1
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
  "\"in\"", "\"isdef\"", "\"{\"", "\"[\"", "\"(\"", "\"onleave\"", "\".\"",
  "\"}\"", "\"]\"", "\"return\"", "\")\"", "\"stopif\"", "\"switch\"",
  "\"throw\"", "\"~\"", "\"timeout\"", "\"try\"", "\"var\"",
  "\"waituntil\"", "\"watch\"", "\"whenever\"", "\",\"", "\";\"", "\"&\"",
  "\"|\"", "\"every\"", "\"for\"", "\"loop\"", "\"while\"", "\"at\"",
  "\"identifier\"", "ASSIGN", "EMPTY", "UNARY", "\"private\"",
  "\"protected\"", "\"public\"", "\"class\"", "\"package\"", "\"enum\"",
  "\"external\"", "\"import\"", "\"^=\"", "\"-=\"", "\"%=\"", "\"+=\"",
  "\"/=\"", "\"*=\"", "\"--\"", "\"++\"", "\"->\"", "\"do\"", "\"assert\"",
  "\"detach\"", "\"disown\"", "\"new\"", "\"angle\"", "\"duration\"",
  "\"float\"", "\"=>\"", "\"string\"", "\"?\"", "\"call\"", "\"this\"",
  "\"!\"", "\"bitand\"", "\"bitor\"", "\"^\"", "\"compl\"", "\">>\"",
  "\"<<\"", "\"-\"", "\"%\"", "\"+\"", "\"/\"", "\"*\"", "\"**\"",
  "\"=~=\"", "\"==\"", "\"===\"", "\">=\"", "\">\"", "\"<=\"", "\"<\"",
  "\"!=\"", "\"!==\"", "\"~=\"", "\"&&\"", "\"||\"", "\"%unscope:\"",
  "\"%exp:\"", "\"%lvalue:\"", "\"%id:\"", "\"%exps:\"", "$accept",
  "start", "root", "root_exp", "root_exps", "stmts", "cstmt.opt", "cstmt",
  "stmt.opt", "stmt", "block", "visibility", "proto", "protos.1", "protos",
  "exp", "id.0", "id.1", "from", "event_or_function", "routine", "k1_id",
  "modifier", "modifiers", "primary-exp", "default.opt", "else.opt",
  "onleave.opt", "cases", "case", "catches.1", "match", "match.opt",
  "catch", "catch.opt", "finally.opt", "in_or_colon", "detach", "lvalue",
  "id", "bitor-exp", "new", "float-exp", "duration", "assoc", "assocs.1",
  "assocs", "dictionary", "tuple.exps", "tuple", "literal-exp", "string",
  "event_match", "guard.opt", "tilda.opt", "unary-exp", "rel-op",
  "rel-exp", "rel-ops", "exp.opt", "unsigned", "claims", "claims.1",
  "exps", "exps.1", "exps.2", "args", "args.opt", "identifiers",
  "typespec", "typespec.opt", "formal", "formals.1", "formals.0",
  "formals", "comma.opt", "semi.opt", "var.opt", 0
  };
#endif

#if YYDEBUG
  const unsigned short int
  parser::yyrline_[] =
  {
       0,   313,   313,   328,   329,   330,   336,   337,   338,   339,
     340,   341,   346,   358,   359,   360,   368,   369,   370,   375,
     376,   377,   387,   388,   393,   404,   408,   409,   413,   426,
     428,   429,   430,   435,   441,   442,   447,   448,   453,   462,
     478,   479,   483,   484,   489,   500,   509,   513,   523,   528,
     533,   546,   587,   600,   601,   607,   613,   660,   661,   662,
     673,   682,   686,   703,   707,   723,   724,   725,   726,   727,
     728,   736,   737,   748,   759,   763,   767,   771,   775,   779,
     783,   787,   792,   796,   800,   804,   808,   812,   816,   833,
     834,   839,   840,   846,   847,   857,   858,   864,   873,   874,
     879,   880,   883,   884,   888,   895,   896,   902,   903,   907,
     911,   915,   944,   948,   952,   956,   960,   966,   966,   976,
     977,   989,   990,   994,   995,   996,   997,   998,   999,  1009,
    1010,  1011,  1015,  1016,  1020,  1024,  1028,  1035,  1039,  1052,
    1064,  1069,  1079,  1096,  1106,  1107,  1120,  1128,  1132,  1140,
    1141,  1146,  1157,  1158,  1159,  1163,  1171,  1172,  1173,  1174,
    1175,  1176,  1177,  1183,  1184,  1192,  1202,  1210,  1211,  1216,
    1217,  1226,  1242,  1243,  1247,  1248,  1249,  1250,  1255,  1256,
    1257,  1258,  1259,  1260,  1261,  1286,  1287,  1288,  1289,  1290,
    1291,  1292,  1293,  1294,  1295,  1296,  1322,  1323,  1324,  1325,
    1326,  1327,  1328,  1329,  1330,  1331,  1335,  1340,  1341,  1355,
    1356,  1357,  1362,  1363,  1367,  1368,  1379,  1387,  1399,  1407,
    1415,  1419,  1427,  1444,  1445,  1449,  1450,  1456,  1457,  1461,
    1462,  1466,  1471,  1475,  1476,  1486,  1487,  1492,  1497,  1498,
    1503,  1504,  1505,  1511,  1512,  1517,  1518,  1523,  1524,  1527,
    1527,  1528,  1528,  1529,  1529
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
#line 5510 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.cc"
/* Line 1132 of lalr1.cc  */
#line 1531 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"


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
