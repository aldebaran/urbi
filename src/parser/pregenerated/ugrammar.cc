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
      case 172: // rel-op
        yysym.value.template destroy< libport::Symbol >();
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
      case 166: // literal-exp
      case 169: // guard.opt
      case 170: // tilda.opt
      case 171: // unary-exp
      case 173: // rel-exp
      case 175: // exp.opt
        yysym.value.template destroy< ast::rExp >();
	break;

      case 136: // modifier
        yysym.value.template destroy< ::ast::Factory::modifier_type >();
	break;

      case 135: // k1_id
        yysym.value.template destroy< ast::rCall >();
	break;

      case 119: // stmts
      case 139: // default.opt
        yysym.value.template destroy< ast::rNary >();
	break;

      case 127: // protos.1
      case 128: // protos
      case 162: // tuple.exps
      case 163: // tuple
      case 164: // bitor-exps
      case 165: // bitor-exps.1
      case 177: // claims
      case 178: // claims.1
      case 179: // exps
      case 180: // exps.1
      case 181: // exps.2
      case 182: // args
      case 183: // args.opt
        yysym.value.template destroy< ast::exps_type* >();
	break;

      case 130: // id.0
      case 131: // id.1
        yysym.value.template destroy< ast::symbols_type >();
	break;

      case 176: // unsigned
        yysym.value.template destroy< unsigned >();
	break;

      case 134: // routine
      case 151: // detach
        yysym.value.template destroy< bool >();
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
      case 167: // string
        yysym.value.template destroy< std::string >();
	break;

      case 168: // event_match
        yysym.value.template destroy< ast::EventMatch >();
	break;

      case 174: // rel-ops
        yysym.value.template destroy< ::ast::Factory::relations_type >();
	break;

      case 184: // identifiers
        yysym.value.template destroy< ::ast::symbols_type >();
	break;

      case 185: // typespec
      case 186: // typespec.opt
        yysym.value.template destroy< ::ast::rExp >();
	break;

      case 187: // formal
        yysym.value.template destroy< ::ast::Formal >();
	break;

      case 188: // formals.1
      case 189: // formals.0
      case 190: // formals
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
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 546 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 42: // ";"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 555 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 43: // "&"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 564 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 44: // "|"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 573 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 45: // "every"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 582 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 46: // "for"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 591 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 47: // "loop"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 600 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 48: // "while"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 609 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 49: // "at"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::flavor_type >(); }
/* Line 576 of lalr1.cc  */
#line 618 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 50: // "identifier"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 627 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 62: // "^="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 636 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 63: // "-="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 645 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 64: // "%="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 654 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 65: // "+="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 663 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 66: // "/="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 672 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 67: // "*="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 681 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 75: // "new"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 690 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 76: // "angle"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 699 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 77: // "duration"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 708 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 78: // "float"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 717 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 80: // "string"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< std::string >(); }
/* Line 576 of lalr1.cc  */
#line 726 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 84: // "!"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 735 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 85: // "bitand"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 744 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 86: // "bitor"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 753 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 87: // "^"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 762 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 88: // "compl"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 771 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 89: // ">>"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 780 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 90: // "<<"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 789 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 91: // "-"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 798 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 92: // "%"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 807 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 93: // "+"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 816 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 94: // "/"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 825 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 95: // "*"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 834 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 96: // "**"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 843 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 97: // "=~="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 852 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 98: // "=="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 861 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 99: // "==="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 870 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 100: // ">="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 879 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 101: // ">"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 888 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 102: // "<="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 897 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 103: // "<"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 906 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 104: // "!="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 915 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 105: // "!=="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 924 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 106: // "~="

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 933 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 107: // "&&"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 942 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 108: // "||"

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 951 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 116: // root

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 960 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 117: // root_exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 969 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 118: // root_exps

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 978 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 119: // stmts

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rNary >(); }
/* Line 576 of lalr1.cc  */
#line 987 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 120: // cstmt.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 996 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 121: // cstmt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1005 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 122: // stmt.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1014 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 123: // stmt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1023 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 124: // block

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1032 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 126: // proto

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1041 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 127: // protos.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1050 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 128: // protos

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1059 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 129: // exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1068 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 130: // id.0

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::symbols_type >(); }
/* Line 576 of lalr1.cc  */
#line 1077 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 131: // id.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::symbols_type >(); }
/* Line 576 of lalr1.cc  */
#line 1086 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 133: // event_or_function

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 1095 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 134: // routine

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< bool >(); }
/* Line 576 of lalr1.cc  */
#line 1104 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 135: // k1_id

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rCall >(); }
/* Line 576 of lalr1.cc  */
#line 1113 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 136: // modifier

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::modifier_type >(); }
/* Line 576 of lalr1.cc  */
#line 1122 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 137: // modifiers

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::modifiers_type >(); }
/* Line 576 of lalr1.cc  */
#line 1131 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 138: // primary-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1140 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 139: // default.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rNary >(); }
/* Line 576 of lalr1.cc  */
#line 1149 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 140: // else.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1158 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 141: // onleave.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1167 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 142: // cases

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::cases_type >(); }
/* Line 576 of lalr1.cc  */
#line 1176 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 143: // case

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::case_type >(); }
/* Line 576 of lalr1.cc  */
#line 1185 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 144: // catches.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::catches_type >(); }
/* Line 576 of lalr1.cc  */
#line 1194 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 145: // match

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rMatch >(); }
/* Line 576 of lalr1.cc  */
#line 1203 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 146: // match.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rMatch >(); }
/* Line 576 of lalr1.cc  */
#line 1212 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 147: // catch

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rCatch >(); }
/* Line 576 of lalr1.cc  */
#line 1221 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 148: // catch.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1230 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 149: // finally.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1239 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 151: // detach

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< bool >(); }
/* Line 576 of lalr1.cc  */
#line 1248 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 152: // lvalue

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rLValue >(); }
/* Line 576 of lalr1.cc  */
#line 1257 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 153: // id

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 1266 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 154: // bitor-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1275 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 155: // new

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1284 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 156: // float-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1293 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 157: // duration

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::ufloat >(); }
/* Line 576 of lalr1.cc  */
#line 1302 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 158: // assoc

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::dictionary_elt_type >(); }
/* Line 576 of lalr1.cc  */
#line 1311 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 159: // assocs.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::dictionary_elts_type >(); }
/* Line 576 of lalr1.cc  */
#line 1320 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 160: // assocs

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::dictionary_elts_type >(); }
/* Line 576 of lalr1.cc  */
#line 1329 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 161: // dictionary

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rDictionary >(); }
/* Line 576 of lalr1.cc  */
#line 1338 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 162: // tuple.exps

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1347 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 163: // tuple

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1356 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 164: // bitor-exps

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1365 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 165: // bitor-exps.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1374 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 166: // literal-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1383 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 167: // string

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< std::string >(); }
/* Line 576 of lalr1.cc  */
#line 1392 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 168: // event_match

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::EventMatch >(); }
/* Line 576 of lalr1.cc  */
#line 1401 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 169: // guard.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1410 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 170: // tilda.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1419 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 171: // unary-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1428 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 172: // rel-op

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< libport::Symbol >(); }
/* Line 576 of lalr1.cc  */
#line 1437 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 173: // rel-exp

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1446 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 174: // rel-ops

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Factory::relations_type >(); }
/* Line 576 of lalr1.cc  */
#line 1455 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 175: // exp.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1464 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 176: // unsigned

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< unsigned >(); }
/* Line 576 of lalr1.cc  */
#line 1473 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 177: // claims

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1482 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 178: // claims.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1491 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 179: // exps

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1500 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 180: // exps.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1509 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 181: // exps.2

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1518 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 182: // args

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1527 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 183: // args.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ast::exps_type* >(); }
/* Line 576 of lalr1.cc  */
#line 1536 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 184: // identifiers

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::symbols_type >(); }
/* Line 576 of lalr1.cc  */
#line 1545 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 185: // typespec

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1554 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 186: // typespec.opt

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::rExp >(); }
/* Line 576 of lalr1.cc  */
#line 1563 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 187: // formal

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formal >(); }
/* Line 576 of lalr1.cc  */
#line 1572 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 188: // formals.1

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formals* >(); }
/* Line 576 of lalr1.cc  */
#line 1581 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 189: // formals.0

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
        { debug_stream() << libport::deref << yysym.value.template as< ::ast::Formals* >(); }
/* Line 576 of lalr1.cc  */
#line 1590 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
        break;

            case 190: // formals

/* Line 576 of lalr1.cc  */
#line 205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
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
      case 172: // rel-op
        yystack_[0].value.build< libport::Symbol >(sym.value);
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
      case 166: // literal-exp
      case 169: // guard.opt
      case 170: // tilda.opt
      case 171: // unary-exp
      case 173: // rel-exp
      case 175: // exp.opt
        yystack_[0].value.build< ast::rExp >(sym.value);
	break;

      case 136: // modifier
        yystack_[0].value.build< ::ast::Factory::modifier_type >(sym.value);
	break;

      case 135: // k1_id
        yystack_[0].value.build< ast::rCall >(sym.value);
	break;

      case 119: // stmts
      case 139: // default.opt
        yystack_[0].value.build< ast::rNary >(sym.value);
	break;

      case 127: // protos.1
      case 128: // protos
      case 162: // tuple.exps
      case 163: // tuple
      case 164: // bitor-exps
      case 165: // bitor-exps.1
      case 177: // claims
      case 178: // claims.1
      case 179: // exps
      case 180: // exps.1
      case 181: // exps.2
      case 182: // args
      case 183: // args.opt
        yystack_[0].value.build< ast::exps_type* >(sym.value);
	break;

      case 130: // id.0
      case 131: // id.1
        yystack_[0].value.build< ast::symbols_type >(sym.value);
	break;

      case 176: // unsigned
        yystack_[0].value.build< unsigned >(sym.value);
	break;

      case 134: // routine
      case 151: // detach
        yystack_[0].value.build< bool >(sym.value);
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
      case 167: // string
        yystack_[0].value.build< std::string >(sym.value);
	break;

      case 168: // event_match
        yystack_[0].value.build< ast::EventMatch >(sym.value);
	break;

      case 174: // rel-ops
        yystack_[0].value.build< ::ast::Factory::relations_type >(sym.value);
	break;

      case 184: // identifiers
        yystack_[0].value.build< ::ast::symbols_type >(sym.value);
	break;

      case 185: // typespec
      case 186: // typespec.opt
        yystack_[0].value.build< ::ast::rExp >(sym.value);
	break;

      case 187: // formal
        yystack_[0].value.build< ::ast::Formal >(sym.value);
	break;

      case 188: // formals.1
      case 189: // formals.0
      case 190: // formals
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
      case 172: // rel-op
        yystack_[0].value.build< libport::Symbol >(s.value);
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
      case 166: // literal-exp
      case 169: // guard.opt
      case 170: // tilda.opt
      case 171: // unary-exp
      case 173: // rel-exp
      case 175: // exp.opt
        yystack_[0].value.build< ast::rExp >(s.value);
	break;

      case 136: // modifier
        yystack_[0].value.build< ::ast::Factory::modifier_type >(s.value);
	break;

      case 135: // k1_id
        yystack_[0].value.build< ast::rCall >(s.value);
	break;

      case 119: // stmts
      case 139: // default.opt
        yystack_[0].value.build< ast::rNary >(s.value);
	break;

      case 127: // protos.1
      case 128: // protos
      case 162: // tuple.exps
      case 163: // tuple
      case 164: // bitor-exps
      case 165: // bitor-exps.1
      case 177: // claims
      case 178: // claims.1
      case 179: // exps
      case 180: // exps.1
      case 181: // exps.2
      case 182: // args
      case 183: // args.opt
        yystack_[0].value.build< ast::exps_type* >(s.value);
	break;

      case 130: // id.0
      case 131: // id.1
        yystack_[0].value.build< ast::symbols_type >(s.value);
	break;

      case 176: // unsigned
        yystack_[0].value.build< unsigned >(s.value);
	break;

      case 134: // routine
      case 151: // detach
        yystack_[0].value.build< bool >(s.value);
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
      case 167: // string
        yystack_[0].value.build< std::string >(s.value);
	break;

      case 168: // event_match
        yystack_[0].value.build< ast::EventMatch >(s.value);
	break;

      case 174: // rel-ops
        yystack_[0].value.build< ::ast::Factory::relations_type >(s.value);
	break;

      case 184: // identifiers
        yystack_[0].value.build< ::ast::symbols_type >(s.value);
	break;

      case 185: // typespec
      case 186: // typespec.opt
        yystack_[0].value.build< ::ast::rExp >(s.value);
	break;

      case 187: // formal
        yystack_[0].value.build< ::ast::Formal >(s.value);
	break;

      case 188: // formals.1
      case 189: // formals.0
      case 190: // formals
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
      case 172: // rel-op
        yylhs.value.build< libport::Symbol >();
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
      case 166: // literal-exp
      case 169: // guard.opt
      case 170: // tilda.opt
      case 171: // unary-exp
      case 173: // rel-exp
      case 175: // exp.opt
        yylhs.value.build< ast::rExp >();
	break;

      case 136: // modifier
        yylhs.value.build< ::ast::Factory::modifier_type >();
	break;

      case 135: // k1_id
        yylhs.value.build< ast::rCall >();
	break;

      case 119: // stmts
      case 139: // default.opt
        yylhs.value.build< ast::rNary >();
	break;

      case 127: // protos.1
      case 128: // protos
      case 162: // tuple.exps
      case 163: // tuple
      case 164: // bitor-exps
      case 165: // bitor-exps.1
      case 177: // claims
      case 178: // claims.1
      case 179: // exps
      case 180: // exps.1
      case 181: // exps.2
      case 182: // args
      case 183: // args.opt
        yylhs.value.build< ast::exps_type* >();
	break;

      case 130: // id.0
      case 131: // id.1
        yylhs.value.build< ast::symbols_type >();
	break;

      case 176: // unsigned
        yylhs.value.build< unsigned >();
	break;

      case 134: // routine
      case 151: // detach
        yylhs.value.build< bool >();
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
      case 167: // string
        yylhs.value.build< std::string >();
	break;

      case 168: // event_match
        yylhs.value.build< ast::EventMatch >();
	break;

      case 174: // rel-ops
        yylhs.value.build< ::ast::Factory::relations_type >();
	break;

      case 184: // identifiers
        yylhs.value.build< ::ast::symbols_type >();
	break;

      case 185: // typespec
      case 186: // typespec.opt
        yylhs.value.build< ::ast::rExp >();
	break;

      case 187: // formal
        yylhs.value.build< ::ast::Formal >();
	break;

      case 188: // formals.1
      case 189: // formals.0
      case 190: // formals
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
#line 314 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
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
#line 328 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2465 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 4:
/* Line 828 of lalr1.cc  */
#line 329 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2473 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 5:
/* Line 828 of lalr1.cc  */
#line 330 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2481 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 6:
/* Line 828 of lalr1.cc  */
#line 336 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Stmt(yylhs.location, yystack_[0].value.as< ast::flavor_type >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2489 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 7:
/* Line 828 of lalr1.cc  */
#line 337 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Stmt(yylhs.location, yystack_[0].value.as< ast::flavor_type >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2497 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 8:
/* Line 828 of lalr1.cc  */
#line 338 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Stmt(yylhs.location, ast::flavor_none, yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2505 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 9:
/* Line 828 of lalr1.cc  */
#line 339 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2513 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 10:
/* Line 828 of lalr1.cc  */
#line 340 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2521 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 11:
/* Line 828 of lalr1.cc  */
#line 341 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2529 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 12:
/* Line 828 of lalr1.cc  */
#line 346 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rNary >(); }
/* Line 828 of lalr1.cc  */
#line 2537 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 13:
/* Line 828 of lalr1.cc  */
#line 358 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = MAKE(nary, yylhs.location, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2545 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 14:
/* Line 828 of lalr1.cc  */
#line 359 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = MAKE(nary, yylhs.location, yystack_[2].value.as< ast::rNary >(), yystack_[1].location, yystack_[1].value.as< ast::flavor_type >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2553 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 15:
/* Line 828 of lalr1.cc  */
#line 360 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = MAKE(nary, yylhs.location, yystack_[2].value.as< ast::rNary >(), yystack_[1].location, yystack_[1].value.as< ast::flavor_type >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2561 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 16:
/* Line 828 of lalr1.cc  */
#line 368 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2569 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 17:
/* Line 828 of lalr1.cc  */
#line 369 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2577 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 18:
/* Line 828 of lalr1.cc  */
#line 370 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(bin, yylhs.location, yystack_[0].value.as< ast::flavor_type >(), yystack_[1].value.as< ast::rExp >(), MAKE(noop, yystack_[0].location)); }
/* Line 828 of lalr1.cc  */
#line 2585 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 19:
/* Line 828 of lalr1.cc  */
#line 375 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { assert(yystack_[0].value.as< ast::rExp >()); std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2593 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 20:
/* Line 828 of lalr1.cc  */
#line 376 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(bin, yylhs.location, yystack_[1].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2601 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 21:
/* Line 828 of lalr1.cc  */
#line 377 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(bin, yylhs.location, yystack_[1].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2609 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 22:
/* Line 828 of lalr1.cc  */
#line 387 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2617 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 23:
/* Line 828 of lalr1.cc  */
#line 388 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2625 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 24:
/* Line 828 of lalr1.cc  */
#line 394 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::TaggedStmt(yylhs.location, yystack_[2].value.as< ast::rExp >(), MAKE(scope, yylhs.location, yystack_[0].value.as< ast::rExp >()));
  }
/* Line 828 of lalr1.cc  */
#line 2635 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 25:
/* Line 828 of lalr1.cc  */
#line 404 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2643 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 26:
/* Line 828 of lalr1.cc  */
#line 408 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(strip, yystack_[1].value.as< ast::rNary >()); }
/* Line 828 of lalr1.cc  */
#line 2651 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 27:
/* Line 828 of lalr1.cc  */
#line 409 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2659 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 28:
/* Line 828 of lalr1.cc  */
#line 413 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 2667 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 33:
/* Line 828 of lalr1.cc  */
#line 435 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2675 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 34:
/* Line 828 of lalr1.cc  */
#line 441 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 2683 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 35:
/* Line 828 of lalr1.cc  */
#line 442 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 2691 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 36:
/* Line 828 of lalr1.cc  */
#line 447 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = 0; }
/* Line 828 of lalr1.cc  */
#line 2699 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 37:
/* Line 828 of lalr1.cc  */
#line 448 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 2707 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 38:
/* Line 828 of lalr1.cc  */
#line 454 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(class, yylhs.location, yystack_[2].value.as< ast::rLValue >(), yystack_[1].value.as< ast::exps_type* >(), yystack_[0].value.as< ast::rExp >(), false);
    }
/* Line 828 of lalr1.cc  */
#line 2717 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 39:
/* Line 828 of lalr1.cc  */
#line 463 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
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
#line 478 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {}
/* Line 828 of lalr1.cc  */
#line 2737 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 41:
/* Line 828 of lalr1.cc  */
#line 479 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::symbols_type >(), yystack_[1].value.as< ast::symbols_type >()); }
/* Line 828 of lalr1.cc  */
#line 2745 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 42:
/* Line 828 of lalr1.cc  */
#line 483 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::symbols_type >() << yystack_[0].value.as< libport::Symbol >(); }
/* Line 828 of lalr1.cc  */
#line 2753 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 43:
/* Line 828 of lalr1.cc  */
#line 484 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::symbols_type >(), yystack_[2].value.as< ast::symbols_type >()); yylhs.value.as< ast::symbols_type >() << yystack_[0].value.as< libport::Symbol >(); }
/* Line 828 of lalr1.cc  */
#line 2761 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 44:
/* Line 828 of lalr1.cc  */
#line 490 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(enum, yylhs.location, yystack_[3].value.as< libport::Symbol >(), yystack_[1].value.as< ast::symbols_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2771 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 45:
/* Line 828 of lalr1.cc  */
#line 501 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    REQUIRE_IDENTIFIER(yystack_[0].location, yystack_[0].value.as< libport::Symbol >(), "from");
  }
/* Line 828 of lalr1.cc  */
#line 2781 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 46:
/* Line 828 of lalr1.cc  */
#line 510 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< libport::Symbol >() = SYMBOL(function);
  }
/* Line 828 of lalr1.cc  */
#line 2791 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 47:
/* Line 828 of lalr1.cc  */
#line 514 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    REQUIRE_IDENTIFIER(yystack_[0].location, yystack_[0].value.as< libport::Symbol >(), "event");
    yylhs.value.as< libport::Symbol >() = SYMBOL(event);
  }
/* Line 828 of lalr1.cc  */
#line 2802 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 48:
/* Line 828 of lalr1.cc  */
#line 524 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    REQUIRE_IDENTIFIER(yystack_[1].location, yystack_[1].value.as< libport::Symbol >(), "object");
    yylhs.value.as< ast::rExp >() = MAKE(external_object, yylhs.location, yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 2813 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 49:
/* Line 828 of lalr1.cc  */
#line 530 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(external_var, yylhs.location, yystack_[4].value.as< libport::Symbol >(), yystack_[2].value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 2823 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 50:
/* Line 828 of lalr1.cc  */
#line 536 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(external_event_or_function,
              yylhs.location, yystack_[8].value.as< libport::Symbol >(), yystack_[6].value.as< unsigned >(), yystack_[4].value.as< libport::Symbol >(), yystack_[2].value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 2834 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 51:
/* Line 828 of lalr1.cc  */
#line 547 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
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
#line 588 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::Emit(yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[1].value.as< ast::exps_type* >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 2876 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 53:
/* Line 828 of lalr1.cc  */
#line 600 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = true; }
/* Line 828 of lalr1.cc  */
#line 2884 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 54:
/* Line 828 of lalr1.cc  */
#line 601 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = false; }
/* Line 828 of lalr1.cc  */
#line 2892 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 55:
/* Line 828 of lalr1.cc  */
#line 608 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
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
#line 614 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      if (yystack_[3].value.as< libport::Symbol >() == SYMBOL(get) || yystack_[3].value.as< libport::Symbol >() == SYMBOL(set))
      {
        yylhs.value.as< ast::rExp >() = MAKE(define_setter_getter, yylhs.location,
          libport::Symbol("o" + std::string(yystack_[3].value.as< libport::Symbol >())), yystack_[2].value.as< libport::Symbol >(),
          MAKE(routine, yylhs.location, false, yystack_[1].location, yystack_[1].value.as< ::ast::Formals* >(), yystack_[0].value.as< ast::rExp >()));
      }
      else
      {
        REQUIRE_IDENTIFIER(yylhs.location, yystack_[3].value.as< libport::Symbol >(), "get or set");
        yylhs.value.as< ast::rExp >() = MAKE(nil);
      }
    }
/* Line 828 of lalr1.cc  */
#line 2924 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 57:
/* Line 828 of lalr1.cc  */
#line 660 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCall >() = MAKE(call, yylhs.location, yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2932 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 58:
/* Line 828 of lalr1.cc  */
#line 661 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCall >() = MAKE(call, yylhs.location, new ast::This(yystack_[2].location), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2940 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 59:
/* Line 828 of lalr1.cc  */
#line 662 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCall >() = MAKE(call, yylhs.location, ast::rExp(yystack_[2].value.as< ast::rCall >()), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 2948 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 60:
/* Line 828 of lalr1.cc  */
#line 674 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ::ast::Factory::modifier_type >().first = yystack_[2].value.as< libport::Symbol >();
    yylhs.value.as< ::ast::Factory::modifier_type >().second = yystack_[0].value.as< ast::rExp >();
  }
/* Line 828 of lalr1.cc  */
#line 2959 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 61:
/* Line 828 of lalr1.cc  */
#line 683 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    modifiers_add(up, yystack_[0].location, yylhs.value.as< ast::modifiers_type >(), yystack_[0].value.as< ::ast::Factory::modifier_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2969 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 62:
/* Line 828 of lalr1.cc  */
#line 687 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    std::swap(yylhs.value.as< ast::modifiers_type >(), yystack_[1].value.as< ast::modifiers_type >());
    modifiers_add(up, yystack_[0].location, yylhs.value.as< ast::modifiers_type >(), yystack_[0].value.as< ::ast::Factory::modifier_type >());
  }
/* Line 828 of lalr1.cc  */
#line 2980 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 63:
/* Line 828 of lalr1.cc  */
#line 704 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(assign, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 2990 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 64:
/* Line 828 of lalr1.cc  */
#line 708 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(assign, yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::modifiers_type >());
  }
/* Line 828 of lalr1.cc  */
#line 3000 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 65:
/* Line 828 of lalr1.cc  */
#line 723 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3008 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 66:
/* Line 828 of lalr1.cc  */
#line 724 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3016 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 67:
/* Line 828 of lalr1.cc  */
#line 725 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3024 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 68:
/* Line 828 of lalr1.cc  */
#line 726 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3032 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 69:
/* Line 828 of lalr1.cc  */
#line 727 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3040 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 70:
/* Line 828 of lalr1.cc  */
#line 728 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::OpAssignment(yystack_[1].location, yystack_[2].value.as< ast::rLValue >(), yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3048 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 71:
/* Line 828 of lalr1.cc  */
#line 736 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Decrementation(yylhs.location, yystack_[1].value.as< ast::rLValue >(), true); }
/* Line 828 of lalr1.cc  */
#line 3056 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 72:
/* Line 828 of lalr1.cc  */
#line 737 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Incrementation(yylhs.location, yystack_[1].value.as< ast::rLValue >(), true); }
/* Line 828 of lalr1.cc  */
#line 3064 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 73:
/* Line 828 of lalr1.cc  */
#line 749 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::Property(yylhs.location, yystack_[2].value.as< ast::rLValue >()->call(), yystack_[0].value.as< libport::Symbol >());
  }
/* Line 828 of lalr1.cc  */
#line 3074 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 74:
/* Line 828 of lalr1.cc  */
#line 760 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(at, yylhs.location, yystack_[7].location, yystack_[7].value.as< ast::flavor_type >(), yystack_[6].value.as< ::ast::symbols_type >(), yystack_[4].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >(), yystack_[3].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3084 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 75:
/* Line 828 of lalr1.cc  */
#line 764 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(at_event, yylhs.location, yystack_[6].location, yystack_[6].value.as< ast::flavor_type >(), yystack_[5].value.as< ::ast::symbols_type >(), yystack_[3].value.as< ast::EventMatch >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3094 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 76:
/* Line 828 of lalr1.cc  */
#line 768 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(every, yylhs.location, yystack_[4].location, yystack_[4].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3104 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 77:
/* Line 828 of lalr1.cc  */
#line 772 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(if, yylhs.location, yystack_[3].value.as< ast::rNary >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3114 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 78:
/* Line 828 of lalr1.cc  */
#line 776 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(freezeif, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3124 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 79:
/* Line 828 of lalr1.cc  */
#line 780 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(stopif, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3134 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 80:
/* Line 828 of lalr1.cc  */
#line 784 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(switch, yystack_[5].location, yystack_[5].value.as< ast::rExp >(), yystack_[2].value.as< ::ast::Factory::cases_type >(), yystack_[1].value.as< ast::rNary >());
    }
/* Line 828 of lalr1.cc  */
#line 3144 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 81:
/* Line 828 of lalr1.cc  */
#line 788 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(timeout, yylhs.location,
                yystack_[5].value.as< ast::rExp >(), yystack_[3].value.as< ast::rExp >(), yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3155 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 82:
/* Line 828 of lalr1.cc  */
#line 793 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = new ast::Return(yylhs.location, yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3165 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 83:
/* Line 828 of lalr1.cc  */
#line 797 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = new ast::Break(yylhs.location);
    }
/* Line 828 of lalr1.cc  */
#line 3175 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 84:
/* Line 828 of lalr1.cc  */
#line 801 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = new ast::Continue(yylhs.location);
    }
/* Line 828 of lalr1.cc  */
#line 3185 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 85:
/* Line 828 of lalr1.cc  */
#line 805 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(waituntil, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3195 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 86:
/* Line 828 of lalr1.cc  */
#line 809 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(waituntil_event, yylhs.location, yystack_[1].value.as< ast::EventMatch >());
    }
/* Line 828 of lalr1.cc  */
#line 3205 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 87:
/* Line 828 of lalr1.cc  */
#line 813 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(whenever, yylhs.location, yystack_[4].value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >(), yystack_[3].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3215 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 88:
/* Line 828 of lalr1.cc  */
#line 817 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(whenever_event, yylhs.location, yystack_[3].value.as< ast::EventMatch >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3225 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 89:
/* Line 828 of lalr1.cc  */
#line 833 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rNary >() = 0;            }
/* Line 828 of lalr1.cc  */
#line 3233 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 90:
/* Line 828 of lalr1.cc  */
#line 834 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rNary >(), yystack_[0].value.as< ast::rNary >()); }
/* Line 828 of lalr1.cc  */
#line 3241 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 91:
/* Line 828 of lalr1.cc  */
#line 839 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0;            }
/* Line 828 of lalr1.cc  */
#line 3249 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 92:
/* Line 828 of lalr1.cc  */
#line 840 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3257 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 93:
/* Line 828 of lalr1.cc  */
#line 846 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0;            }
/* Line 828 of lalr1.cc  */
#line 3265 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 94:
/* Line 828 of lalr1.cc  */
#line 847 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3273 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 95:
/* Line 828 of lalr1.cc  */
#line 857 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {}
/* Line 828 of lalr1.cc  */
#line 3281 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 96:
/* Line 828 of lalr1.cc  */
#line 858 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Factory::cases_type >(), yystack_[1].value.as< ::ast::Factory::cases_type >()); yylhs.value.as< ::ast::Factory::cases_type >() << yystack_[0].value.as< ::ast::Factory::case_type >(); }
/* Line 828 of lalr1.cc  */
#line 3289 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 97:
/* Line 828 of lalr1.cc  */
#line 864 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Factory::case_type >() = ::ast::Factory::case_type(yystack_[2].value.as< ast::rMatch >(), yystack_[0].value.as< ast::rNary >()); }
/* Line 828 of lalr1.cc  */
#line 3297 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 98:
/* Line 828 of lalr1.cc  */
#line 873 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::catches_type >() = ast::catches_type(); yylhs.value.as< ast::catches_type >() << yystack_[0].value.as< ast::rCatch >(); }
/* Line 828 of lalr1.cc  */
#line 3305 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 99:
/* Line 828 of lalr1.cc  */
#line 874 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::catches_type >(), yystack_[1].value.as< ast::catches_type >());        yylhs.value.as< ast::catches_type >() << yystack_[0].value.as< ast::rCatch >(); }
/* Line 828 of lalr1.cc  */
#line 3313 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 100:
/* Line 828 of lalr1.cc  */
#line 879 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rMatch >() = new ast::Match(yylhs.location, yystack_[0].value.as< ast::rExp >(), 0);  }
/* Line 828 of lalr1.cc  */
#line 3321 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 101:
/* Line 828 of lalr1.cc  */
#line 880 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rMatch >() = new ast::Match(yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3329 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 102:
/* Line 828 of lalr1.cc  */
#line 883 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rMatch >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3337 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 103:
/* Line 828 of lalr1.cc  */
#line 884 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rMatch >(), yystack_[1].value.as< ast::rMatch >()); }
/* Line 828 of lalr1.cc  */
#line 3345 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 104:
/* Line 828 of lalr1.cc  */
#line 888 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rCatch >() = MAKE(catch, yylhs.location, yystack_[1].value.as< ast::rMatch >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3353 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 105:
/* Line 828 of lalr1.cc  */
#line 895 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3361 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 106:
/* Line 828 of lalr1.cc  */
#line 896 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 3369 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 107:
/* Line 828 of lalr1.cc  */
#line 902 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0;  }
/* Line 828 of lalr1.cc  */
#line 3377 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 108:
/* Line 828 of lalr1.cc  */
#line 903 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 3385 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 109:
/* Line 828 of lalr1.cc  */
#line 908 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(try, yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[2].value.as< ast::catches_type >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3395 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 110:
/* Line 828 of lalr1.cc  */
#line 912 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(finally, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3405 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 111:
/* Line 828 of lalr1.cc  */
#line 916 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(throw, yylhs.location, yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3415 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 112:
/* Line 828 of lalr1.cc  */
#line 945 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(loop, yylhs.location, yystack_[1].location, yystack_[1].value.as< ast::flavor_type >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3425 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 113:
/* Line 828 of lalr1.cc  */
#line 949 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(for, yylhs.location, yystack_[4].location, yystack_[4].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3435 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 114:
/* Line 828 of lalr1.cc  */
#line 953 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(for, yylhs.location, yystack_[8].location, yystack_[8].value.as< ast::flavor_type >(), yystack_[6].value.as< ast::rExp >(), yystack_[4].value.as< ast::rExp >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3445 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 115:
/* Line 828 of lalr1.cc  */
#line 957 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(for, yylhs.location, yystack_[7].location, yystack_[7].value.as< ast::flavor_type >(), yystack_[4].location, yystack_[4].value.as< libport::Symbol >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3455 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 116:
/* Line 828 of lalr1.cc  */
#line 961 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
      yylhs.value.as< ast::rExp >() = MAKE(while, yylhs.location, yystack_[4].location, yystack_[4].value.as< ast::flavor_type >(), yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
    }
/* Line 828 of lalr1.cc  */
#line 3465 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 119:
/* Line 828 of lalr1.cc  */
#line 976 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(scope, yylhs.location, 0, yystack_[0].value.as< ast::rExp >());  }
/* Line 828 of lalr1.cc  */
#line 3473 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 120:
/* Line 828 of lalr1.cc  */
#line 977 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(scope, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3481 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 121:
/* Line 828 of lalr1.cc  */
#line 989 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = true; }
/* Line 828 of lalr1.cc  */
#line 3489 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 122:
/* Line 828 of lalr1.cc  */
#line 990 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< bool >() = false; }
/* Line 828 of lalr1.cc  */
#line 3497 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 123:
/* Line 828 of lalr1.cc  */
#line 994 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(assert, yylhs.location, yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3505 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 124:
/* Line 828 of lalr1.cc  */
#line 995 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(assert, yylhs.location, yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3513 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 125:
/* Line 828 of lalr1.cc  */
#line 996 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(detach, yylhs.location, yystack_[3].value.as< bool >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3521 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 126:
/* Line 828 of lalr1.cc  */
#line 997 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(detach, yylhs.location, yystack_[1].value.as< bool >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3529 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 127:
/* Line 828 of lalr1.cc  */
#line 998 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(isdef, yylhs.location, yystack_[1].value.as< ast::rCall >()); }
/* Line 828 of lalr1.cc  */
#line 3537 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 128:
/* Line 828 of lalr1.cc  */
#line 999 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(watch, yylhs.location, yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3545 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 129:
/* Line 828 of lalr1.cc  */
#line 1009 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rLValue >() = MAKE(call, yylhs.location, yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3553 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 130:
/* Line 828 of lalr1.cc  */
#line 1010 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rLValue >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3561 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 131:
/* Line 828 of lalr1.cc  */
#line 1011 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rLValue >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3569 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 132:
/* Line 828 of lalr1.cc  */
#line 1015 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(get_slot, yylhs.location, yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3577 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 133:
/* Line 828 of lalr1.cc  */
#line 1016 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(get_slot, yylhs.location, yystack_[3].value.as< ast::rExp >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3585 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 134:
/* Line 828 of lalr1.cc  */
#line 1020 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3593 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 135:
/* Line 828 of lalr1.cc  */
#line 1025 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(binding, yylhs.location, false, yystack_[0].location, yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3603 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 136:
/* Line 828 of lalr1.cc  */
#line 1029 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(binding, yylhs.location, true, yystack_[0].location, yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3613 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 137:
/* Line 828 of lalr1.cc  */
#line 1036 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rLValue >();
  }
/* Line 828 of lalr1.cc  */
#line 3623 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 138:
/* Line 828 of lalr1.cc  */
#line 1040 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
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
#line 1053 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
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
#line 1064 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3658 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 141:
/* Line 828 of lalr1.cc  */
#line 1069 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 3666 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 142:
/* Line 828 of lalr1.cc  */
#line 1080 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = MAKE(routine, yylhs.location, yystack_[2].value.as< bool >(), yystack_[1].location, yystack_[1].value.as< ::ast::Formals* >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3676 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 143:
/* Line 828 of lalr1.cc  */
#line 1096 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(float, yylhs.location, yystack_[0].value.as< libport::ufloat >()); }
/* Line 828 of lalr1.cc  */
#line 3684 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 144:
/* Line 828 of lalr1.cc  */
#line 1106 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< libport::ufloat >() = yystack_[0].value.as< libport::ufloat >();      }
/* Line 828 of lalr1.cc  */
#line 3692 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 145:
/* Line 828 of lalr1.cc  */
#line 1107 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< libport::ufloat >() = yystack_[1].value.as< libport::ufloat >() + yystack_[0].value.as< libport::ufloat >(); }
/* Line 828 of lalr1.cc  */
#line 3700 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 146:
/* Line 828 of lalr1.cc  */
#line 1121 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::dictionary_elt_type >().first = yystack_[2].value.as< ast::rExp >();
    yylhs.value.as< ast::dictionary_elt_type >().second = yystack_[0].value.as< ast::rExp >();
  }
/* Line 828 of lalr1.cc  */
#line 3711 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 147:
/* Line 828 of lalr1.cc  */
#line 1129 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    assocs_add(up, yystack_[0].location, yylhs.value.as< ast::dictionary_elts_type >(), yystack_[0].value.as< ast::dictionary_elt_type >());
  }
/* Line 828 of lalr1.cc  */
#line 3721 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 148:
/* Line 828 of lalr1.cc  */
#line 1133 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    std::swap(yylhs.value.as< ast::dictionary_elts_type >(), yystack_[2].value.as< ast::dictionary_elts_type >());
    assocs_add(up, yystack_[0].location, yylhs.value.as< ast::dictionary_elts_type >(), yystack_[0].value.as< ast::dictionary_elt_type >());
  }
/* Line 828 of lalr1.cc  */
#line 3732 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 149:
/* Line 828 of lalr1.cc  */
#line 1140 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { /* nothing */ }
/* Line 828 of lalr1.cc  */
#line 3740 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 150:
/* Line 828 of lalr1.cc  */
#line 1141 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::dictionary_elts_type >(), yystack_[1].value.as< ast::dictionary_elts_type >()); }
/* Line 828 of lalr1.cc  */
#line 3748 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 151:
/* Line 828 of lalr1.cc  */
#line 1146 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rDictionary >() = new ast::Dictionary(yylhs.location, yystack_[1].value.as< ast::dictionary_elts_type >()); }
/* Line 828 of lalr1.cc  */
#line 3756 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 152:
/* Line 828 of lalr1.cc  */
#line 1157 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 3764 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 153:
/* Line 828 of lalr1.cc  */
#line 1158 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3772 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 154:
/* Line 828 of lalr1.cc  */
#line 1159 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3780 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 155:
/* Line 828 of lalr1.cc  */
#line 1163 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = yystack_[1].value.as< ast::exps_type* >(); }
/* Line 828 of lalr1.cc  */
#line 3788 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 156:
/* Line 828 of lalr1.cc  */
#line 1177 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 3796 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 157:
/* Line 828 of lalr1.cc  */
#line 1178 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3804 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 158:
/* Line 828 of lalr1.cc  */
#line 1182 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3812 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 159:
/* Line 828 of lalr1.cc  */
#line 1183 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >();}
/* Line 828 of lalr1.cc  */
#line 3820 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 160:
/* Line 828 of lalr1.cc  */
#line 1191 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());  }
/* Line 828 of lalr1.cc  */
#line 3828 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 161:
/* Line 828 of lalr1.cc  */
#line 1192 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(float, yylhs.location, yystack_[0].value.as< libport::ufloat >());  }
/* Line 828 of lalr1.cc  */
#line 3836 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 162:
/* Line 828 of lalr1.cc  */
#line 1193 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(float, yylhs.location, yystack_[0].value.as< libport::ufloat >());  }
/* Line 828 of lalr1.cc  */
#line 3844 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 163:
/* Line 828 of lalr1.cc  */
#line 1194 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(string, yylhs.location, yystack_[0].value.as< std::string >()); }
/* Line 828 of lalr1.cc  */
#line 3852 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 164:
/* Line 828 of lalr1.cc  */
#line 1195 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(list, yylhs.location, yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3860 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 165:
/* Line 828 of lalr1.cc  */
#line 1196 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(vector, yylhs.location, yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3868 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 166:
/* Line 828 of lalr1.cc  */
#line 1197 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = yystack_[0].value.as< ast::rDictionary >(); }
/* Line 828 of lalr1.cc  */
#line 3876 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 167:
/* Line 828 of lalr1.cc  */
#line 1198 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(tuple, yylhs.location, yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 3884 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 168:
/* Line 828 of lalr1.cc  */
#line 1204 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< std::string >(), yystack_[0].value.as< std::string >());  }
/* Line 828 of lalr1.cc  */
#line 3892 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 169:
/* Line 828 of lalr1.cc  */
#line 1205 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< std::string >(), yystack_[1].value.as< std::string >()); yylhs.value.as< std::string >() += yystack_[0].value.as< std::string >(); }
/* Line 828 of lalr1.cc  */
#line 3900 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 170:
/* Line 828 of lalr1.cc  */
#line 1213 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(position, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3908 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 171:
/* Line 828 of lalr1.cc  */
#line 1224 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::EventMatch >() = MAKE(event_match, yylhs.location, yystack_[4].value.as< ast::rExp >(), yystack_[2].value.as< ast::exps_type* >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3918 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 172:
/* Line 828 of lalr1.cc  */
#line 1231 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3926 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 173:
/* Line 828 of lalr1.cc  */
#line 1232 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3934 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 174:
/* Line 828 of lalr1.cc  */
#line 1237 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 3942 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 175:
/* Line 828 of lalr1.cc  */
#line 1238 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3950 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 176:
/* Line 828 of lalr1.cc  */
#line 1248 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::Subscript(yylhs.location, yystack_[1].value.as< ast::exps_type* >(), yystack_[3].value.as< ast::rExp >());
  }
/* Line 828 of lalr1.cc  */
#line 3960 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 177:
/* Line 828 of lalr1.cc  */
#line 1263 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::This(yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3968 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 178:
/* Line 828 of lalr1.cc  */
#line 1264 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::CallMsg(yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 3976 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 179:
/* Line 828 of lalr1.cc  */
#line 1268 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3984 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 180:
/* Line 828 of lalr1.cc  */
#line 1269 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[1].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 3992 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 181:
/* Line 828 of lalr1.cc  */
#line 1270 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(noop, yylhs.location); }
/* Line 828 of lalr1.cc  */
#line 4000 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 182:
/* Line 828 of lalr1.cc  */
#line 1271 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4008 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 183:
/* Line 828 of lalr1.cc  */
#line 1276 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4016 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 184:
/* Line 828 of lalr1.cc  */
#line 1277 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Decrementation(yylhs.location, yystack_[0].value.as< ast::rLValue >(), false); }
/* Line 828 of lalr1.cc  */
#line 4024 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 185:
/* Line 828 of lalr1.cc  */
#line 1278 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = new ast::Incrementation(yylhs.location, yystack_[0].value.as< ast::rLValue >(), false); }
/* Line 828 of lalr1.cc  */
#line 4032 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 186:
/* Line 828 of lalr1.cc  */
#line 1279 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 4040 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 187:
/* Line 828 of lalr1.cc  */
#line 1280 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 4048 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 188:
/* Line 828 of lalr1.cc  */
#line 1281 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 4056 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 189:
/* Line 828 of lalr1.cc  */
#line 1282 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), new ast::exps_type()); }
/* Line 828 of lalr1.cc  */
#line 4064 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 190:
/* Line 828 of lalr1.cc  */
#line 1307 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4072 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 191:
/* Line 828 of lalr1.cc  */
#line 1308 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4080 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 192:
/* Line 828 of lalr1.cc  */
#line 1309 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4088 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 193:
/* Line 828 of lalr1.cc  */
#line 1310 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4096 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 194:
/* Line 828 of lalr1.cc  */
#line 1311 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4104 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 195:
/* Line 828 of lalr1.cc  */
#line 1312 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4112 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 196:
/* Line 828 of lalr1.cc  */
#line 1313 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4120 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 197:
/* Line 828 of lalr1.cc  */
#line 1314 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4128 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 198:
/* Line 828 of lalr1.cc  */
#line 1315 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4136 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 199:
/* Line 828 of lalr1.cc  */
#line 1316 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4144 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 200:
/* Line 828 of lalr1.cc  */
#line 1317 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4152 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 201:
/* Line 828 of lalr1.cc  */
#line 1343 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4160 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 202:
/* Line 828 of lalr1.cc  */
#line 1344 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4168 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 203:
/* Line 828 of lalr1.cc  */
#line 1345 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4176 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 204:
/* Line 828 of lalr1.cc  */
#line 1346 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4184 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 205:
/* Line 828 of lalr1.cc  */
#line 1347 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4192 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 206:
/* Line 828 of lalr1.cc  */
#line 1348 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4200 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 207:
/* Line 828 of lalr1.cc  */
#line 1349 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4208 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 208:
/* Line 828 of lalr1.cc  */
#line 1350 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4216 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 209:
/* Line 828 of lalr1.cc  */
#line 1351 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4224 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 210:
/* Line 828 of lalr1.cc  */
#line 1352 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< libport::Symbol >(), yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4232 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 211:
/* Line 828 of lalr1.cc  */
#line 1356 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(relation, yylhs.location, yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ::ast::Factory::relations_type >()); }
/* Line 828 of lalr1.cc  */
#line 4240 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 212:
/* Line 828 of lalr1.cc  */
#line 1361 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { /* empty */ }
/* Line 828 of lalr1.cc  */
#line 4248 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 213:
/* Line 828 of lalr1.cc  */
#line 1362 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Factory::relations_type >(), MAKE(relation, yystack_[2].value.as< ::ast::Factory::relations_type >(), yystack_[1].value.as< libport::Symbol >(), yystack_[0].value.as< ast::rExp >())); }
/* Line 828 of lalr1.cc  */
#line 4256 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 214:
/* Line 828 of lalr1.cc  */
#line 1376 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4264 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 215:
/* Line 828 of lalr1.cc  */
#line 1377 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(and, yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4272 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 216:
/* Line 828 of lalr1.cc  */
#line 1378 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(or,  yylhs.location, yystack_[2].value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4280 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 217:
/* Line 828 of lalr1.cc  */
#line 1383 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), SYMBOL(has),    yystack_[2].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4288 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 218:
/* Line 828 of lalr1.cc  */
#line 1384 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = MAKE(call, yylhs.location, yystack_[0].value.as< ast::rExp >(), SYMBOL(hasNot), yystack_[3].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4296 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 219:
/* Line 828 of lalr1.cc  */
#line 1388 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::rExp >() = 0; }
/* Line 828 of lalr1.cc  */
#line 4304 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 220:
/* Line 828 of lalr1.cc  */
#line 1389 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::rExp >(), yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4312 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 221:
/* Line 828 of lalr1.cc  */
#line 1400 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< unsigned >() = static_cast<unsigned int>(yystack_[0].value.as< libport::ufloat >()); }
/* Line 828 of lalr1.cc  */
#line 4320 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 222:
/* Line 828 of lalr1.cc  */
#line 1409 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::Unscope(yylhs.location, yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4330 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 223:
/* Line 828 of lalr1.cc  */
#line 1421 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rExp >() = new ast::MetaExp(yylhs.location, yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4340 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 224:
/* Line 828 of lalr1.cc  */
#line 1429 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::MetaLValue(yylhs.location, new ast::exps_type(), yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4350 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 225:
/* Line 828 of lalr1.cc  */
#line 1437 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::MetaId(yylhs.location, 0, yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4360 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 226:
/* Line 828 of lalr1.cc  */
#line 1441 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    yylhs.value.as< ast::rLValue >() = new ast::MetaCall(yylhs.location, 0, yystack_[3].value.as< ast::rExp >(), yystack_[0].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4370 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 227:
/* Line 828 of lalr1.cc  */
#line 1449 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    {
    assert(yystack_[4].value.as< ast::rLValue >().unsafe_cast<ast::LValueArgs>());
    assert(!yystack_[4].value.as< ast::rLValue >().unsafe_cast<ast::LValueArgs>()->arguments_get());
    yylhs.value.as< ast::rExp >() = new ast::MetaArgs(yylhs.location, yystack_[4].value.as< ast::rLValue >(), yystack_[1].value.as< unsigned >());
  }
/* Line 828 of lalr1.cc  */
#line 4382 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 228:
/* Line 828 of lalr1.cc  */
#line 1465 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 4390 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 229:
/* Line 828 of lalr1.cc  */
#line 1466 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4398 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 230:
/* Line 828 of lalr1.cc  */
#line 1470 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4406 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 231:
/* Line 828 of lalr1.cc  */
#line 1471 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 4414 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 232:
/* Line 828 of lalr1.cc  */
#line 1477 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type; }
/* Line 828 of lalr1.cc  */
#line 4422 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 233:
/* Line 828 of lalr1.cc  */
#line 1478 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4430 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 234:
/* Line 828 of lalr1.cc  */
#line 1482 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = new ast::exps_type(1, yystack_[0].value.as< ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4438 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 235:
/* Line 828 of lalr1.cc  */
#line 1483 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 4446 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 236:
/* Line 828 of lalr1.cc  */
#line 1487 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[2].value.as< ast::exps_type* >()); *yylhs.value.as< ast::exps_type* >() << yystack_[0].value.as< ast::rExp >(); }
/* Line 828 of lalr1.cc  */
#line 4454 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 237:
/* Line 828 of lalr1.cc  */
#line 1492 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[1].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4462 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 238:
/* Line 828 of lalr1.cc  */
#line 1496 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ast::exps_type* >() = 0; }
/* Line 828 of lalr1.cc  */
#line 4470 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 239:
/* Line 828 of lalr1.cc  */
#line 1497 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ast::exps_type* >(), yystack_[0].value.as< ast::exps_type* >()); }
/* Line 828 of lalr1.cc  */
#line 4478 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 240:
/* Line 828 of lalr1.cc  */
#line 1507 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { /* empty */ }
/* Line 828 of lalr1.cc  */
#line 4486 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 241:
/* Line 828 of lalr1.cc  */
#line 1508 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::symbols_type >(), yystack_[1].value.as< ::ast::symbols_type >()); yylhs.value.as< ::ast::symbols_type >().push_back(yystack_[0].value.as< libport::Symbol >()); }
/* Line 828 of lalr1.cc  */
#line 4494 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 242:
/* Line 828 of lalr1.cc  */
#line 1513 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::rExp >() = yystack_[0].value.as< ast::rExp >();}
/* Line 828 of lalr1.cc  */
#line 4502 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 243:
/* Line 828 of lalr1.cc  */
#line 1518 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::rExp >()=0;}
/* Line 828 of lalr1.cc  */
#line 4510 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 244:
/* Line 828 of lalr1.cc  */
#line 1519 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::rExp >(), yystack_[0].value.as< ::ast::rExp >());}
/* Line 828 of lalr1.cc  */
#line 4518 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 245:
/* Line 828 of lalr1.cc  */
#line 1524 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formal >() = ::ast::Formal(yystack_[1].value.as< libport::Symbol >(), 0, yystack_[0].value.as< ::ast::rExp >());  }
/* Line 828 of lalr1.cc  */
#line 4526 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 246:
/* Line 828 of lalr1.cc  */
#line 1525 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formal >() = ::ast::Formal(yystack_[3].value.as< libport::Symbol >(), yystack_[1].value.as< ast::rExp >(), yystack_[0].value.as< ::ast::rExp >()); }
/* Line 828 of lalr1.cc  */
#line 4534 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 247:
/* Line 828 of lalr1.cc  */
#line 1526 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formal >() = ::ast::Formal(yystack_[2].value.as< libport::Symbol >(), true); }
/* Line 828 of lalr1.cc  */
#line 4542 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 248:
/* Line 828 of lalr1.cc  */
#line 1532 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formals* >() = new ::ast::Formals(1, yystack_[0].value.as< ::ast::Formal >()); }
/* Line 828 of lalr1.cc  */
#line 4550 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 249:
/* Line 828 of lalr1.cc  */
#line 1533 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Formals* >(), yystack_[2].value.as< ::ast::Formals* >()); *yylhs.value.as< ::ast::Formals* >() << yystack_[0].value.as< ::ast::Formal >(); }
/* Line 828 of lalr1.cc  */
#line 4558 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 250:
/* Line 828 of lalr1.cc  */
#line 1538 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formals* >() = new ::ast::Formals; }
/* Line 828 of lalr1.cc  */
#line 4566 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 251:
/* Line 828 of lalr1.cc  */
#line 1539 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Formals* >(), yystack_[1].value.as< ::ast::Formals* >()); }
/* Line 828 of lalr1.cc  */
#line 4574 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 252:
/* Line 828 of lalr1.cc  */
#line 1544 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { yylhs.value.as< ::ast::Formals* >() = 0; }
/* Line 828 of lalr1.cc  */
#line 4582 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;

  case 253:
/* Line 828 of lalr1.cc  */
#line 1545 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"
    { std::swap(yylhs.value.as< ::ast::Formals* >(), yystack_[1].value.as< ::ast::Formals* >()); }
/* Line 828 of lalr1.cc  */
#line 4590 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
    break;


/* Line 828 of lalr1.cc  */
#line 4595 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
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


  const short int parser::yypact_ninf_ = -257;

  const short int parser::yytable_ninf_ = -259;

  const short int
  parser::yypact_[] =
  {
     271,  -257,   673,  1136,    53,  -257,    55,  -257,  -257,  -257,
      28,  -257,    20,  -257,    58,    69,   867,  1320,   951,  1472,
      76,   117,  1472,   122,   146,  1548,   149,   156,   159,   -27,
     168,   188,  1136,   197,  -257,  -257,   535,   535,   -27,   248,
     535,   535,   535,   222,   194,  -257,  -257,   218,  -257,  -257,
    -257,  -257,  -257,  -257,  1700,  1700,  1700,  1700,  1548,   193,
     193,   193,   193,  -257,    90,   145,  -257,  -257,    74,    -9,
       5,   286,   557,   -27,   563,  -257,  -257,   202,  -257,  -257,
    -257,   204,  -257,  -257,  -257,   155,  -257,  -257,  -257,  -257,
    -257,  1548,  1396,  1136,   -21,   262,   135,    80,  -257,    44,
     266,     9,  -257,  -257,   258,   275,   279,   259,   274,    37,
     283,   273,  -257,    86,  -257,  1396,  1396,  -257,  1396,    17,
      42,   563,  1396,  1396,  1396,  -257,  -257,  1396,  1244,  -257,
    1396,    13,     9,   500,   500,   298,  -257,   278,   288,   305,
     257,   257,   257,  1396,  1396,  1396,   311,  -257,  -257,  -257,
    -257,   563,   240,   323,  -257,  -257,  -257,  -257,  -257,  -257,
    -257,  -257,  1136,  1136,  1396,  1396,   123,  1396,  1396,   113,
    -257,   326,   352,   146,  1136,  1396,   -26,   535,  1396,  -257,
    1027,  1396,  1396,  1396,  1396,  1396,  1396,  -257,  -257,   -27,
    -257,   266,  1548,  1548,  1548,  1548,  1548,  1548,  1548,  1548,
    1548,  1548,   700,  -257,  -257,  1136,  1136,   563,   107,   276,
     139,  -257,  -257,   -27,  1396,   345,  1396,  -257,  -257,  -257,
    1396,  -257,  -257,  -257,  -257,  1396,   136,   189,   247,   347,
     146,     7,  -257,     0,   355,   272,     0,   369,   285,  1624,
     360,  -257,   289,   317,  1396,  -257,   179,   146,   146,   -27,
     379,  -257,   193,   383,    86,   387,   364,   392,  1396,  -257,
    -257,  -257,  1548,  -257,  -257,   372,   366,    -1,  1396,   386,
       1,    50,  -257,  -257,   382,   396,   377,   380,   385,   146,
    -257,  -257,    86,   411,   -27,  -257,   193,  -257,     9,   398,
     193,   410,    86,    86,    86,    86,    86,    86,  -257,   146,
     718,   575,   701,   537,   537,   339,  -257,   339,  -257,  -257,
    -257,  -257,  -257,  -257,  -257,  -257,  -257,  -257,  -257,  -257,
    1548,  -257,  -257,  1136,  1136,  -257,   415,    86,    44,  -257,
      86,   482,  1136,   421,  1136,  1396,   146,  -257,  1136,   428,
    -257,  1396,   311,   416,  -257,  -257,   423,  1136,  1136,    38,
    1396,  1136,  1136,     0,   424,  -257,  -257,  -257,  1396,  -257,
     408,  -257,  -257,   429,   417,  -257,   401,   427,   146,  -257,
    1396,  -257,  -257,   563,   448,  -257,   413,    -1,  -257,   113,
    -257,  -257,   263,  -257,  -257,  -257,  -257,  -257,  -257,  -257,
     434,  -257,  -257,   563,  -257,   450,  -257,  -257,  -257,   461,
      41,   441,  -257,  -257,   146,  -257,    86,   386,  -257,  1136,
     450,  -257,  -257,  -257,  1396,   180,  -257,  -257,   447,  1136,
      86,   179,  -257,   -27,  -257,   430,   439,  -257,    86,  1396,
    -257,  -257,  1396,  1396,   451,  -257,  -257,  -257,  -257,   101,
     146,   450,  1396,  -257,  -257,   477,   450,  -257,   488,  1136,
    1136,   472,  -257,  -257,  -257,   453,   475,    86,   232,    86,
    -257,  1396,   485,   492,  -257,  -257,   428,    86,  1396,  -257,
    -257,  1136,   479,   472,  1136,  -257,  -257,   465,  -257,   507,
    1136,  -257,  -257,    86,  -257,  1136,  -257,  -257,   430,  1136,
     155,  -257,   476,   155,  -257
  };

  const unsigned short int
  parser::yydefact_[] =
  {
       0,     3,     0,    16,     0,     2,     0,   170,    83,    53,
       0,    84,     0,    54,     0,     0,     0,   232,     0,   219,
       0,     0,   219,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   240,   134,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   121,   122,   141,   161,   144,
     143,   168,   178,   177,     0,     0,     0,     0,   156,     0,
       0,     0,     0,     4,     0,    17,    19,   119,    25,   252,
     183,     0,   137,   129,   212,   140,   160,   162,   166,   167,
     179,   163,   190,   214,     5,    12,    13,     1,    11,    10,
       9,     0,     0,    16,     0,     0,     0,   129,   149,   234,
     252,   183,   129,   147,   254,     0,     0,   254,     0,   234,
       0,     0,   154,   220,    82,     0,     0,   111,     0,     0,
     137,   135,     0,     0,     0,   141,   132,     0,    22,   112,
       0,     0,     0,   137,   137,     0,    46,     0,    47,     0,
      51,   184,   185,     0,   228,     0,   238,   188,   189,   187,
     186,   158,     0,   254,   221,   222,   223,   224,   225,     8,
       7,     6,     0,    18,     0,     0,   238,     0,     0,   250,
      57,     0,   252,     0,     0,   232,     0,     0,     0,   126,
     232,     0,     0,     0,     0,     0,     0,    71,    72,     0,
     138,   252,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   211,   145,   169,    16,    16,   136,     0,     0,
       0,    27,    26,     0,     0,     0,   255,   150,   151,   164,
     255,   233,   181,   180,   155,   153,     0,     0,     0,   102,
       0,    91,    98,   174,     0,     0,   174,     0,     0,     0,
       0,    23,    25,     0,     0,   241,    29,     0,     0,    40,
       0,    48,     0,     0,   230,     0,   256,     0,   232,   239,
     139,   165,   255,   157,    21,    20,    63,   217,     0,   174,
     215,   216,   259,   248,   254,     0,     0,     0,     0,     0,
     142,    24,   234,     0,     0,   131,     0,   130,   182,     0,
       0,     0,    69,    66,    70,    65,    68,    67,    73,     0,
     198,   199,   196,   200,   197,   192,   195,   191,   194,   193,
     207,   205,   206,   209,   208,   204,   203,   201,   202,   210,
       0,    15,    14,     0,     0,   127,     0,   146,     0,   148,
     235,   235,     0,     0,     0,     0,     0,   110,     0,   107,
      99,     0,   238,     0,    86,   128,     0,     0,     0,   134,
       0,     0,     0,   174,     0,    30,    31,    32,     0,    34,
      37,    38,    39,     0,   254,    42,     0,     0,     0,   124,
     257,   229,   123,   159,     0,    61,    64,   218,    52,   255,
     251,   253,   243,    58,    59,    55,   176,   133,   226,   125,
       0,   237,    56,   213,    78,    91,    28,    79,    95,   105,
     100,     0,   104,    92,     0,   109,   175,   174,    85,     0,
      91,    76,   118,   117,     0,     0,   113,   116,     0,     0,
      33,    29,    44,   255,    41,     0,     0,   120,   231,     0,
      62,   249,     0,     0,     0,   244,   245,   227,    77,    89,
       0,    91,     0,   103,   108,   172,    91,    88,     0,    22,
       0,    93,    35,    43,    45,     0,     0,    60,   243,   242,
     247,     0,     0,     0,    96,   106,   107,   101,     0,   171,
      87,     0,     0,    93,     0,    75,    49,     0,   246,     0,
      16,    80,    81,   173,   115,     0,    74,    94,     0,    16,
      90,   114,     0,    97,    50
  };

  const short int
  parser::yypgoto_[] =
  {
    -257,  -257,  -257,  -257,  -257,   -14,     2,    88,    78,    10,
      64,  -257,   108,  -257,   397,   419,  -257,  -257,    45,  -257,
      43,   436,   160,  -257,    36,  -257,  -256,    65,  -257,  -257,
    -257,    83,  -257,   308,  -257,    82,  -257,  -257,    62,    -2,
      31,  -257,  -257,  -257,   334,  -257,  -257,  -257,  -257,  -257,
    -257,  -257,  -257,  -257,  -111,  -257,  -229,   354,  -257,  -257,
    -257,   529,   -50,  -257,  -257,    -8,   534,  -257,  -141,  -163,
    -257,  -257,    96,   176,  -257,  -257,   -32,   -99,  -257,  -257
  };

  const short int
  parser::yydefgoto_[] =
  {
      -1,     4,     5,    63,    84,    85,    86,    65,   240,    66,
      67,   358,   359,   360,   247,    68,   363,   364,   455,   139,
     100,   172,   375,   376,   101,   463,   339,   475,   439,   464,
     231,   401,   336,   232,   441,   405,   414,    71,    72,   102,
      74,    75,    76,    77,   103,   104,   105,    78,   110,    79,
     152,   153,    80,    81,   234,   469,   343,    82,   320,    83,
     202,   114,   155,   255,   256,   291,   107,   112,   190,   260,
     131,   435,   436,   273,   274,   275,   173,   217,   371,   276
  };

  const short int
  parser::yytable_[] =
  {
      73,    73,    96,   269,    64,   259,   164,   346,   221,   106,
     156,   157,   158,   237,    97,   169,   229,   284,   174,  -259,
     165,   165,   338,    35,    35,   259,   229,   126,   175,   170,
      73,   176,   175,   230,   341,   176,   135,   244,    70,    70,
     378,   170,   129,   164,    92,    69,    69,   164,   125,   125,
     164,   412,    70,    87,   263,    88,   121,   165,   413,    69,
     442,   165,   171,   245,   165,    91,   180,   223,    70,   285,
     165,   191,   132,   132,   171,    69,   132,   132,   132,   209,
     164,   342,    93,   215,   215,   215,   286,   120,   119,   151,
     159,    73,   164,    94,   165,   191,    89,    90,   133,   134,
     115,   177,   140,   141,   142,   177,   165,   167,   168,   461,
     187,   188,   189,   164,   213,   462,   120,   120,   120,   120,
     120,   215,   207,   214,   418,   215,    73,   165,   215,    70,
      35,   160,   161,   354,   215,   179,    69,   323,   241,   438,
     279,   116,   164,   268,   167,   168,   118,   258,   167,   168,
     272,   167,   168,   120,   447,   125,   165,   167,   166,   299,
      73,    73,   212,  -258,    70,   278,   332,   283,    16,   325,
     215,    69,    73,   122,   287,   380,   205,   206,   445,   407,
     123,   167,   168,   124,   281,   466,   164,   298,   162,   163,
     470,   215,   127,   167,   168,   164,   205,   206,    70,    70,
     165,   259,   367,    73,    73,    69,    69,   321,   322,   165,
      70,   326,   128,   288,   167,   168,   144,    69,   145,   333,
     215,   130,   449,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   355,   356,   357,   388,   280,   164,   120,
     390,    70,    70,   167,   168,   433,   143,   365,    69,    69,
     264,   265,   165,   164,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   215,   424,   136,   165,   146,   432,
     121,   154,     1,   215,     2,     3,   433,   334,   164,   203,
    -137,   180,   387,  -137,   204,   137,   434,   167,   168,   211,
     169,   164,   165,   373,   337,   164,   167,   168,   138,   216,
     220,   120,   345,   218,   222,   165,   324,   219,    16,   165,
     178,   361,   362,   224,   225,   348,   215,   205,   206,   351,
     249,    73,    73,   164,   120,   187,   188,   189,   250,   252,
      73,   215,    73,   394,   395,   258,    73,   165,   251,   167,
     168,   261,   397,   385,   399,    73,    73,   352,   403,    73,
      73,   393,   277,  -137,   167,   168,   215,   410,   411,    70,
      70,   416,   417,   392,   262,   268,    69,    69,    70,   215,
      70,   335,   164,   166,    70,    69,   169,    69,   278,   167,
     168,    69,   120,    70,    70,   344,   165,    70,    70,   164,
      69,    69,   167,   168,    69,    69,   167,   168,   164,   347,
     402,   215,   350,   165,   164,   366,   370,    73,   147,   148,
     149,   150,   165,   368,   369,   162,   374,    73,   165,   446,
     341,   453,   372,   379,   167,   168,   381,   382,   389,   451,
     383,   198,   427,   200,   201,   384,    99,   109,   113,   386,
     391,   113,   396,   398,   404,    70,   408,    73,    73,   421,
     215,   425,    69,   409,   419,    70,   422,   426,   423,   241,
     473,   429,    69,   374,   437,   338,   490,   215,   444,    73,
     440,   443,    73,   167,   168,   493,   215,   450,    73,   460,
     454,   484,   215,    73,   487,    70,    70,    73,   164,   456,
     167,   168,    69,    69,   164,   491,   468,   474,   480,   167,
     168,   477,   165,   476,   465,   167,   168,    70,   165,   485,
      70,   208,  -236,   246,    69,   488,    70,    69,   471,   481,
     489,    70,   -36,    69,   180,    70,   494,   472,    69,   452,
     210,   248,    69,   492,   226,   227,   430,   228,   486,   340,
       7,   233,   235,   236,   479,     9,   238,   242,   482,   243,
     329,   117,   111,    13,   478,   431,    15,    16,    17,    18,
       0,     0,   253,   254,   257,     0,   215,     0,   187,   188,
     189,     0,   215,     0,    27,     0,     0,     0,    29,     0,
       0,   180,     0,   266,   267,    35,   270,   271,     0,   167,
     168,     0,     0,     0,   282,   167,   168,   289,     0,   282,
     292,   293,   294,   295,   296,   297,    43,    44,    45,    46,
     125,    48,    49,    50,     0,    51,     0,    52,    53,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   197,   198,
     199,   200,   201,   327,     0,   328,     0,     0,    58,   330,
       0,     0,     0,     0,   331,    60,    61,    62,   192,   193,
     194,     0,   195,   196,   197,   198,   199,   200,   201,     0,
     192,     0,   194,   353,   195,   196,   197,   198,   199,   200,
     201,     0,     0,   -16,     6,     0,     0,   282,     7,     0,
       8,     0,     0,     9,    10,    11,     0,   377,     0,     0,
      12,    13,    14,     0,    15,    16,    17,    18,     0,     0,
       0,     0,    19,     0,    20,    21,    22,     0,    23,    24,
      25,    26,    27,    28,   -16,   -16,    29,     0,    30,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,     0,
      36,    37,    38,    39,    40,     0,     0,     0,     0,     0,
       0,    41,    42,     0,    43,    44,    45,    46,    47,    48,
      49,    50,     0,    51,   400,    52,    53,    54,     0,     0,
     406,    55,     0,     0,    56,     0,    57,     0,     0,   415,
       0,     0,     0,     0,     0,     0,    58,   420,     0,     0,
       0,     0,    59,    60,    61,    62,   192,     0,     0,   428,
     195,   196,   197,   198,   199,   200,   201,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   195,   196,   197,
     198,   199,   200,   201,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   448,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   457,     0,
       0,   458,   459,     0,     0,     0,     0,     0,     0,     0,
       0,   467,     0,     0,     0,     0,     0,     0,    95,     0,
       0,     0,     7,     0,     8,     0,     0,     9,    10,    11,
     400,     0,     0,     0,    12,    13,    14,   483,    15,    16,
      17,    18,     0,     0,   -16,     0,    19,     0,    20,    21,
      22,     0,    23,    24,    25,    26,    27,    28,   -16,   -16,
      29,     0,    30,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,     0,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    41,    42,     0,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,     0,    52,
      53,    54,   108,     0,     0,    55,     7,     0,    56,     0,
      57,     9,    10,     0,     0,     0,     0,     0,     0,    13,
      58,     0,    15,    16,    17,    18,    59,    60,    61,    62,
       0,  -152,     0,     0,     0,     0,     0,     0,    25,     0,
      27,     0,     0,     0,    29,     0,     0,     0,     0,     0,
       0,    35,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    41,
      42,     0,    43,    44,    45,    46,    47,    48,    49,    50,
       0,    51,     7,    52,    53,    54,     0,     9,    10,    55,
       0,     0,    56,     0,    57,    13,     0,     0,    15,    16,
      17,    18,     0,     0,    58,     0,     0,     0,     0,     0,
      59,    60,    61,    62,    25,     0,    27,     0,     0,     0,
      29,     0,     0,     0,     0,     0,     0,    35,     0,     0,
       0,     0,     0,     0,    36,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    41,    42,     0,    43,    44,
      45,    46,    47,    48,    49,    50,     0,    51,     0,    52,
      53,    54,     0,     0,     0,    55,     0,     0,    56,     0,
      57,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      58,     0,     0,     0,     0,     0,    59,    60,    61,    62,
     290,     7,     0,     8,     0,     0,     9,    10,    11,     0,
       0,     0,     0,    12,    13,    14,     0,    15,    16,    17,
      18,     0,     0,     0,     0,    19,     0,    20,    21,    22,
       0,    23,    24,    25,    26,    27,    28,     0,     0,    29,
       0,    30,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,     0,    36,    37,    38,    39,    40,     0,     0,
       0,     0,     0,     0,    41,    42,     0,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,     0,    52,    53,
      54,     0,     0,     0,    55,     0,     0,    56,     0,    57,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    58,
       0,     0,     0,     0,     0,    59,    60,    61,    62,     7,
       0,     8,     0,     0,     9,    10,    11,     0,     0,     0,
       0,    12,    13,    14,     0,    15,    16,    17,    18,     0,
       0,     0,     0,    19,     0,    20,    21,    22,     0,    23,
      24,   239,    26,    27,    28,     0,     0,    29,     0,    30,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
       0,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,    41,    42,     0,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,     7,    52,    53,    54,     0,
       9,    10,    55,     0,     0,    56,     0,    57,    13,     0,
       0,    15,    16,    17,    18,     0,     0,    58,     0,     0,
       0,     0,     0,    59,    60,    61,    62,    25,     0,    27,
       0,     0,     0,    29,     0,     0,     0,     0,     0,     0,
      35,     0,     0,     0,     0,     0,     0,    36,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    41,    42,
       0,    43,    44,    45,    46,    47,    48,    49,    50,    98,
      51,     7,    52,    53,    54,     0,     9,    10,    55,     0,
       0,    56,     0,    57,    13,     0,     0,    15,    16,    17,
      18,     0,     0,    58,     0,     0,     0,     0,     0,    59,
      60,    61,    62,    25,     0,    27,     0,     0,     0,    29,
       0,     0,     0,     0,     0,     0,    35,     0,     0,     0,
       0,     0,     0,    36,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    41,    42,     0,    43,    44,    45,
      46,    47,    48,    49,    50,     0,    51,     7,    52,    53,
      54,     0,     9,    10,    55,     0,     0,    56,     0,    57,
      13,     0,     0,    15,    16,    17,    18,     0,     0,    58,
       0,     0,     0,     0,     0,    59,    60,    61,    62,    25,
       0,    27,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    35,     0,     0,     0,     0,     0,     0,    36,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      41,    42,     0,    43,    44,    45,    46,    47,    48,    49,
      50,     0,    51,     7,    52,    53,    54,     0,     9,    10,
      55,     0,     0,    56,     0,    57,    13,     0,     0,    15,
      16,    17,    18,     0,     0,    58,     0,     0,     0,     0,
       0,    59,    60,    61,    62,    25,     0,    27,     0,     0,
       0,    29,     0,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    41,    42,     0,    43,
      44,    45,    46,    47,    48,    49,    50,     0,    51,     7,
      52,    53,    54,     0,     9,    10,    55,     0,     0,    56,
       0,    57,    13,     0,     0,    15,    16,    17,    18,     0,
       0,    58,     0,     0,     0,     0,     0,     0,    60,    61,
      62,    25,     0,    27,     0,     0,     0,    29,     0,     0,
       0,     0,     0,     0,   349,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    41,    42,     0,    43,    44,    45,    46,    47,
      48,    49,    50,     0,    51,     7,    52,    53,    54,     0,
       9,     0,    55,     0,     0,    56,     0,    57,    13,     0,
       0,    15,    16,    17,    18,     0,     0,    58,     0,     0,
       0,     0,     0,     0,    60,    61,    62,     0,     0,    27,
       0,     0,     0,    29,     0,     0,     0,     0,     0,     0,
      35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    41,    42,
       0,    43,    44,    45,    46,   125,    48,    49,    50,     0,
      51,     0,    52,    53,    54,     0,     0,     0,    55,     0,
       0,    56,     0,    57,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    58,     0,     0,     0,     0,     0,     0,
      60,    61,    62
  };

  const short int
  parser::yycheck_[] =
  {
       2,     3,    16,   166,     2,   146,     6,   236,   107,    17,
      60,    61,    62,   124,    16,    24,     9,    43,    13,    20,
      20,    20,    15,    50,    50,   166,     9,    29,    23,    50,
      32,    26,    23,    16,    34,    26,    38,    24,     2,     3,
     269,    50,    32,     6,    24,     2,     3,     6,    75,    75,
       6,    13,    16,     0,   153,     0,    25,    20,    20,    16,
      19,    20,    83,    50,    20,    37,    24,    30,    32,    95,
      20,    73,    36,    37,    83,    32,    40,    41,    42,    93,
       6,    81,    24,    84,    84,    84,   112,    25,    24,    58,
       0,    93,     6,    24,    20,    97,    41,    42,    36,    37,
      24,    96,    40,    41,    42,    96,    20,   107,   108,     8,
      68,    69,    70,     6,    34,    14,    54,    55,    56,    57,
      58,    84,    91,    79,   353,    84,   128,    20,    84,    93,
      50,    41,    42,   244,    84,    71,    93,    30,   128,   395,
     172,    24,     6,    20,   107,   108,    24,    24,   107,   108,
      37,   107,   108,    91,   410,    75,    20,   107,    84,   191,
     162,   163,    27,    50,   128,    26,    30,   175,    22,    30,
      84,   128,   174,    24,   176,   274,    41,    42,   407,   342,
      24,   107,   108,    24,   174,   441,     6,   189,    43,    44,
     446,    84,    24,   107,   108,     6,    41,    42,   162,   163,
      20,   342,   252,   205,   206,   162,   163,   205,   206,    20,
     174,   213,    24,   177,   107,   108,    22,   174,    24,    30,
      84,    24,    42,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,    54,    55,    56,   286,   173,     6,   177,
     290,   205,   206,   107,   108,    13,    24,   249,   205,   206,
     162,   163,    20,     6,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,    84,   364,    18,    20,    50,     6,
     239,    78,     1,    84,     3,     4,    13,    30,     6,    77,
      23,    24,   284,    26,    80,    37,    23,   107,   108,    27,
      24,     6,    20,   262,   230,     6,   107,   108,    50,    41,
      41,   239,    30,    28,    30,    20,    30,    28,    22,    20,
      24,   247,   248,    30,    41,    30,    84,    41,    42,    30,
      22,   323,   324,     6,   262,    68,    69,    70,    50,    24,
     332,    84,   334,   323,   324,    24,   338,    20,    50,   107,
     108,   101,   332,   279,   334,   347,   348,    30,   338,   351,
     352,   320,    26,    96,   107,   108,    84,   347,   348,   323,
     324,   351,   352,   299,    41,    20,   323,   324,   332,    84,
     334,    24,     6,    84,   338,   332,    24,   334,    26,   107,
     108,   338,   320,   347,   348,    30,    20,   351,   352,     6,
     347,   348,   107,   108,   351,   352,   107,   108,     6,    30,
     336,    84,    42,    20,     6,    26,    42,   409,    54,    55,
      56,    57,    20,    30,    27,    43,    50,   419,    20,   409,
      34,   423,    30,    41,   107,   108,    30,    50,    30,   419,
      50,    92,   368,    94,    95,    50,    17,    18,    19,    28,
      30,    22,    27,    22,    16,   409,    30,   449,   450,    41,
      84,    50,   409,    30,    30,   419,    27,    30,    41,   449,
     450,    13,   419,    50,    30,    15,   480,    84,   404,   471,
       9,    30,   474,   107,   108,   489,    84,    30,   480,    28,
      50,   471,    84,   485,   474,   449,   450,   489,     6,    50,
     107,   108,   449,   450,     6,   485,    19,    25,    13,   107,
     108,    26,    20,    50,   440,   107,   108,   471,    20,    30,
     474,    92,    30,    13,   471,    50,   480,   474,    30,    27,
      13,   485,    22,   480,    24,   489,    50,   449,   485,   421,
      94,   134,   489,   488,   115,   116,   376,   118,   473,   231,
       5,   122,   123,   124,   461,    10,   127,   128,   466,   130,
     216,    22,    18,    18,   458,   379,    21,    22,    23,    24,
      -1,    -1,   143,   144,   145,    -1,    84,    -1,    68,    69,
      70,    -1,    84,    -1,    39,    -1,    -1,    -1,    43,    -1,
      -1,    24,    -1,   164,   165,    50,   167,   168,    -1,   107,
     108,    -1,    -1,    -1,   175,   107,   108,   178,    -1,   180,
     181,   182,   183,   184,   185,   186,    71,    72,    73,    74,
      75,    76,    77,    78,    -1,    80,    -1,    82,    83,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    91,    92,
      93,    94,    95,   214,    -1,   216,    -1,    -1,   103,   220,
      -1,    -1,    -1,    -1,   225,   110,   111,   112,    85,    86,
      87,    -1,    89,    90,    91,    92,    93,    94,    95,    -1,
      85,    -1,    87,   244,    89,    90,    91,    92,    93,    94,
      95,    -1,    -1,     0,     1,    -1,    -1,   258,     5,    -1,
       7,    -1,    -1,    10,    11,    12,    -1,   268,    -1,    -1,
      17,    18,    19,    -1,    21,    22,    23,    24,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      57,    58,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    69,    -1,    71,    72,    73,    74,    75,    76,
      77,    78,    -1,    80,   335,    82,    83,    84,    -1,    -1,
     341,    88,    -1,    -1,    91,    -1,    93,    -1,    -1,   350,
      -1,    -1,    -1,    -1,    -1,    -1,   103,   358,    -1,    -1,
      -1,    -1,   109,   110,   111,   112,    85,    -1,    -1,   370,
      89,    90,    91,    92,    93,    94,    95,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    89,    90,    91,
      92,    93,    94,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   414,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   429,    -1,
      -1,   432,   433,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   442,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
      -1,    -1,     5,    -1,     7,    -1,    -1,    10,    11,    12,
     461,    -1,    -1,    -1,    17,    18,    19,   468,    21,    22,
      23,    24,    -1,    -1,    27,    -1,    29,    -1,    31,    32,
      33,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    58,    59,    60,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    -1,    80,    -1,    82,
      83,    84,     1,    -1,    -1,    88,     5,    -1,    91,    -1,
      93,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    18,
     103,    -1,    21,    22,    23,    24,   109,   110,   111,   112,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,
      39,    -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    57,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      69,    -1,    71,    72,    73,    74,    75,    76,    77,    78,
      -1,    80,     5,    82,    83,    84,    -1,    10,    11,    88,
      -1,    -1,    91,    -1,    93,    18,    -1,    -1,    21,    22,
      23,    24,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,
     109,   110,   111,   112,    37,    -1,    39,    -1,    -1,    -1,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    -1,    -1,    88,    -1,    -1,    91,    -1,
      93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,    -1,    -1,    -1,    -1,    -1,   109,   110,   111,   112,
     113,     5,    -1,     7,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    17,    18,    19,    -1,    21,    22,    23,
      24,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      -1,    35,    36,    37,    38,    39,    40,    -1,    -1,    43,
      -1,    45,    46,    47,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    58,    59,    60,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    -1,    -1,    88,    -1,    -1,    91,    -1,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,   109,   110,   111,   112,     5,
      -1,     7,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    17,    18,    19,    -1,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    -1,    35,
      36,    37,    38,    39,    40,    -1,    -1,    43,    -1,    45,
      46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    58,    59,    60,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    -1,    80,     5,    82,    83,    84,    -1,
      10,    11,    88,    -1,    -1,    91,    -1,    93,    18,    -1,
      -1,    21,    22,    23,    24,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,   109,   110,   111,   112,    37,    -1,    39,
      -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,     5,    82,    83,    84,    -1,    10,    11,    88,    -1,
      -1,    91,    -1,    93,    18,    -1,    -1,    21,    22,    23,
      24,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,   109,
     110,   111,   112,    37,    -1,    39,    -1,    -1,    -1,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    -1,    80,     5,    82,    83,
      84,    -1,    10,    11,    88,    -1,    -1,    91,    -1,    93,
      18,    -1,    -1,    21,    22,    23,    24,    -1,    -1,   103,
      -1,    -1,    -1,    -1,    -1,   109,   110,   111,   112,    37,
      -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      68,    69,    -1,    71,    72,    73,    74,    75,    76,    77,
      78,    -1,    80,     5,    82,    83,    84,    -1,    10,    11,
      88,    -1,    -1,    91,    -1,    93,    18,    -1,    -1,    21,
      22,    23,    24,    -1,    -1,   103,    -1,    -1,    -1,    -1,
      -1,   109,   110,   111,   112,    37,    -1,    39,    -1,    -1,
      -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    71,
      72,    73,    74,    75,    76,    77,    78,    -1,    80,     5,
      82,    83,    84,    -1,    10,    11,    88,    -1,    -1,    91,
      -1,    93,    18,    -1,    -1,    21,    22,    23,    24,    -1,
      -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,
     112,    37,    -1,    39,    -1,    -1,    -1,    43,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    68,    69,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    -1,    80,     5,    82,    83,    84,    -1,
      10,    -1,    88,    -1,    -1,    91,    -1,    93,    18,    -1,
      -1,    21,    22,    23,    24,    -1,    -1,   103,    -1,    -1,
      -1,    -1,    -1,    -1,   110,   111,   112,    -1,    -1,    39,
      -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    69,
      -1,    71,    72,    73,    74,    75,    76,    77,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    -1,    -1,    88,    -1,
      -1,    91,    -1,    93,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,    -1,    -1,    -1,    -1,    -1,    -1,
     110,   111,   112
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,     1,     3,     4,   115,   116,     1,     5,     7,    10,
      11,    12,    17,    18,    19,    21,    22,    23,    24,    29,
      31,    32,    33,    35,    36,    37,    38,    39,    40,    43,
      45,    46,    47,    48,    49,    50,    57,    58,    59,    60,
      61,    68,    69,    71,    72,    73,    74,    75,    76,    77,
      78,    80,    82,    83,    84,    88,    91,    93,   103,   109,
     110,   111,   112,   117,   120,   121,   123,   124,   129,   134,
     138,   151,   152,   153,   154,   155,   156,   157,   161,   163,
     166,   167,   171,   173,   118,   119,   120,     0,     0,    41,
      42,    37,    24,    24,    24,     1,   119,   153,    79,   129,
     134,   138,   153,   158,   159,   160,   179,   180,     1,   129,
     162,   180,   181,   129,   175,    24,    24,   175,    24,   124,
     152,   154,    24,    24,    24,    75,   153,    24,    24,   123,
      24,   184,   138,   152,   152,   153,    18,    37,    50,   133,
     152,   152,   152,    24,    22,    24,    50,   171,   171,   171,
     171,   154,   164,   165,    78,   176,   176,   176,   176,     0,
      41,    42,    43,    44,     6,    20,    84,   107,   108,    24,
      50,    83,   135,   190,    13,    23,    26,    96,    24,   124,
      24,    62,    63,    64,    65,    66,    67,    68,    69,    70,
     182,   153,    85,    86,    87,    89,    90,    91,    92,    93,
      94,    95,   174,    77,    80,    41,    42,   154,   129,   119,
     135,    27,    27,    34,    79,    84,    41,   191,    28,    28,
      41,   191,    30,    30,    30,    41,   129,   129,   129,     9,
      16,   144,   147,   129,   168,   129,   129,   168,   129,    37,
     122,   123,   129,   129,    24,    50,    13,   128,   128,    22,
      50,    50,    24,   129,   129,   177,   178,   129,    24,   182,
     183,   101,    41,   191,   121,   121,   129,   129,    20,   183,
     129,   129,    37,   187,   188,   189,   193,    26,    26,   190,
     124,   123,   129,   179,    43,    95,   112,   153,   138,   129,
     113,   179,   129,   129,   129,   129,   129,   129,   153,   190,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     172,   120,   120,    30,    30,    30,   153,   129,   129,   158,
     129,   129,    30,    30,    30,    24,   146,   124,    15,   140,
     147,    34,    81,   170,    30,    30,   170,    30,    30,    50,
      42,    30,    30,   129,   168,    54,    55,    56,   125,   126,
     127,   124,   124,   130,   131,   153,    26,   176,    30,    27,
      42,   192,    30,   154,    50,   136,   137,   129,   170,    41,
     191,    30,    50,    50,    50,   124,    28,   153,   176,    30,
     176,    30,   124,   154,   123,   123,    27,   123,    22,   123,
     129,   145,   124,   123,    16,   149,   129,   183,    30,    30,
     123,   123,    13,    20,   150,   129,   123,   123,   170,    30,
     129,    41,    27,    41,   191,    50,    30,   124,   129,    13,
     136,   187,     6,    13,    23,   185,   186,    30,   140,   142,
       9,   148,    19,    30,   124,   170,   123,   140,   129,    42,
      30,   123,   126,   153,    50,   132,    50,   129,   129,   129,
      28,     8,    14,   139,   143,   124,   140,   129,    19,   169,
     140,    30,   122,   123,    25,   141,    50,    26,   186,   145,
      13,    27,   149,   129,   123,    30,   141,   123,    50,    13,
     119,   123,   132,   119,    50
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
     160,   161,   162,   162,   162,   163,   164,   164,   165,   165,
     166,   166,   166,   166,   166,   166,   166,   166,   167,   167,
     166,   168,   169,   169,   170,   170,   152,   166,   166,   138,
     138,   138,   138,   171,   171,   171,   171,   171,   171,   171,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   172,   172,   172,   172,   172,   172,   172,   172,   172,
     172,   173,   174,   174,   129,   129,   129,   129,   129,   175,
     175,   176,   129,   138,   152,   152,   152,   138,   177,   177,
     178,   178,   179,   179,   180,   180,   181,   182,   183,   183,
     184,   184,   185,   186,   186,   187,   187,   187,   188,   188,
     189,   189,   190,   190,   191,   191,   192,   192,   193,   193
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
       1,     1,     1,     1,     3,     3,     1,     1,     1,     2,
       1,     5,     0,     2,     0,     2,     4,     1,     1,     1,
       3,     3,     3,     1,     2,     2,     2,     2,     2,     2,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     0,     3,     1,     3,     3,     3,     4,     0,
       1,     1,     2,     2,     2,     2,     4,     5,     0,     2,
       1,     3,     0,     2,     1,     3,     3,     3,     0,     1,
       0,     2,     2,     0,     1,     3,     5,     4,     1,     3,
       0,     2,     0,     3,     0,     1,     0,     1,     0,     1
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
  "assocs", "dictionary", "tuple.exps", "tuple", "bitor-exps",
  "bitor-exps.1", "literal-exp", "string", "event_match", "guard.opt",
  "tilda.opt", "unary-exp", "rel-op", "rel-exp", "rel-ops", "exp.opt",
  "unsigned", "claims", "claims.1", "exps", "exps.1", "exps.2", "args",
  "args.opt", "identifiers", "typespec", "typespec.opt", "formal",
  "formals.1", "formals.0", "formals", "comma.opt", "semi.opt", "var.opt", 0
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
    1141,  1146,  1157,  1158,  1159,  1163,  1177,  1178,  1182,  1183,
    1191,  1192,  1193,  1194,  1195,  1196,  1197,  1198,  1204,  1205,
    1213,  1223,  1231,  1232,  1237,  1238,  1247,  1263,  1264,  1268,
    1269,  1270,  1271,  1276,  1277,  1278,  1279,  1280,  1281,  1282,
    1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,
    1317,  1343,  1344,  1345,  1346,  1347,  1348,  1349,  1350,  1351,
    1352,  1356,  1361,  1362,  1376,  1377,  1378,  1383,  1384,  1388,
    1389,  1400,  1408,  1420,  1428,  1436,  1440,  1448,  1465,  1466,
    1470,  1471,  1477,  1478,  1482,  1483,  1487,  1492,  1496,  1497,
    1507,  1508,  1513,  1518,  1519,  1524,  1525,  1526,  1532,  1533,
    1538,  1539,  1544,  1545,  1548,  1548,  1549,  1549,  1550,  1550
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
#line 5583 "/home/bearclaw/qi-2/urbi/urbi/build-sys-linux-i686/src/parser/ugrammar.cc"
/* Line 1132 of lalr1.cc  */
#line 1552 "/home/bearclaw/qi-2/urbi/urbi/src/parser/ugrammar.y"


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
