/* A Bison parser, made by GNU Bison 2.3b.655-27c2.  */

/* Skeleton interface for Bison LALR(1) parsers in C++

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

/* C++ LALR(1) parser skeleton written by Akim Demaille.  */

#ifndef PARSER_HEADER_H
# define PARSER_HEADER_H

/* "%code requires" blocks.  */
/* Line 146 of lalr1.cc  */
#line 29 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"

#include <kernel/config.h> // YYDEBUG.

#include <ast/call.hh>
#include <ast/catches-type.hh>
#include <ast/event-match.hh>
#include <ast/exps-type.hh>
#include <ast/factory.hh>
#include <ast/formal.hh>
#include <ast/fwd.hh>
#include <ast/symbols-type.hh>
#include <urbi/kernel/fwd.hh>
#include <libport/hash.hh>
#include <libport/ufloat.hh>
#include <list>
#include <urbi/object/symbols.hh>
#include <parser/fwd.hh>

/* Line 146 of lalr1.cc  */
#line 198 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/src/parser/ugrammar.y"

#include <ast/flavor.hh>


/* Line 146 of lalr1.cc  */
#line 67 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.hh"



#include <stdexcept>
#include <string>
#include <iostream>
#include "stack.hh"
#include "location.hh"


namespace yy {
/* Line 156 of lalr1.cc  */
#line 80 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.hh"

  /// A char[S] buffer to store and retrieve objects.
  ///
  /// Sort of a variant, but does not keep track of the nature
  /// of the stored data, since that knowledge is available
  /// via the current state.
  template <size_t S>
  struct variant
  {
    /// Empty construction.
    inline
    variant ()
    {}

    /// Instantiate a \a T in here.
    template <typename T>
    inline T&
    build ()
    {
      return *new (buffer.raw) T;
    }

    /// Instantiate a \a T in here from \a t.
    template <typename T>
    inline T&
    build (const T& t)
    {
      return *new (buffer.raw) T(t);
    }

    /// Construct and fill.
    template <typename T>
    inline
    variant (const T& t)
    {
      new (buffer.raw) T(t);
    }

    /// Accessor to a built \a T.
    template <typename T>
    inline T&
    as ()
    {
      return reinterpret_cast<T&>(buffer.raw);
    }

    /// Const accessor to a built \a T (for %printer).
    template <typename T>
    inline const T&
    as () const
    {
      return reinterpret_cast<const T&>(buffer.raw);
    }

    /// Swap the content with \a other.
    template <typename T>
    inline void
    swap (variant<S>& other)
    {
      std::swap (as<T>(), other.as<T>());
    }

    /// Assign the content of \a other to this.
    /// Destroys \a other.
    template <typename T>
    inline void
    build (variant<S>& other)
    {
      build<T>();
      swap<T>(other);
      other.destroy<T>();
    }

    /// Destroy the stored \a T.
    template <typename T>
    inline void
    destroy ()
    {
      as<T>().~T();
    }

    /// A buffer large enough to store any of the semantic values.
    /// Long double is chosen as it has the strongest alignment
    /// constraints.
    union
    {
      long double align_me;
      char raw[S];
    } buffer;
  };


} // yy
/* Line 156 of lalr1.cc  */
#line 175 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.hh"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


namespace yy {
/* Line 178 of lalr1.cc  */
#line 198 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.hh"

  /// A Bison parser.
  class parser
  {
  public:
#ifndef YYSTYPE
    /// An auxiliary type to compute the largest semantic type.
    union union_type
    {
      // ","
      // ";"
      // "&"
      // "|"
      // "every"
      // "for"
      // "loop"
      // "while"
      // "at"
      char dummy1[sizeof(ast::flavor_type)];

      // root
      // root_exp
      // root_exps
      // cstmt.opt
      // cstmt
      // stmt.opt
      // stmt
      // block
      // proto
      // exp
      // primary-exp
      // else.opt
      // onleave.opt
      // catch.opt
      // finally.opt
      // bitor-exp
      // new
      // float-exp
      // literal-exp
      // guard.opt
      // tilda.opt
      // unary-exp
      // rel-exp
      // exp.opt
      char dummy2[sizeof(ast::rExp)];

      // "identifier"
      // "^="
      // "-="
      // "%="
      // "+="
      // "/="
      // "*="
      // "new"
      // "!"
      // "bitand"
      // "bitor"
      // "^"
      // "compl"
      // ">>"
      // "<<"
      // "-"
      // "%"
      // "+"
      // "/"
      // "*"
      // "**"
      // "=~="
      // "=="
      // "==="
      // ">="
      // ">"
      // "<="
      // "<"
      // "!="
      // "!=="
      // "~="
      // "&&"
      // "||"
      // event_or_function
      // id
      // rel-op
      char dummy3[sizeof(libport::Symbol)];

      // stmts
      // default.opt
      char dummy4[sizeof(ast::rNary)];

      // protos.1
      // protos
      // tuple.exps
      // tuple
      // claims
      // claims.1
      // exps
      // exps.1
      // exps.2
      // args
      // args.opt
      char dummy5[sizeof(ast::exps_type*)];

      // id.0
      // id.1
      char dummy6[sizeof(ast::symbols_type)];

      // unsigned
      char dummy7[sizeof(unsigned)];

      // routine
      // detach
      char dummy8[sizeof(bool)];

      // k1_id
      char dummy9[sizeof(ast::rCall)];

      // modifier
      char dummy10[sizeof(::ast::Factory::modifier_type)];

      // modifiers
      char dummy11[sizeof(ast::modifiers_type)];

      // cases
      char dummy12[sizeof(::ast::Factory::cases_type)];

      // case
      char dummy13[sizeof(::ast::Factory::case_type)];

      // catches.1
      char dummy14[sizeof(ast::catches_type)];

      // match
      // match.opt
      char dummy15[sizeof(ast::rMatch)];

      // catch
      char dummy16[sizeof(ast::rCatch)];

      // lvalue
      char dummy17[sizeof(ast::rLValue)];

      // "angle"
      // "duration"
      // "float"
      // duration
      char dummy18[sizeof(libport::ufloat)];

      // assoc
      char dummy19[sizeof(ast::dictionary_elt_type)];

      // assocs.1
      // assocs
      char dummy20[sizeof(ast::dictionary_elts_type)];

      // dictionary
      char dummy21[sizeof(ast::rDictionary)];

      // "string"
      // string
      char dummy22[sizeof(std::string)];

      // event_match
      char dummy23[sizeof(ast::EventMatch)];

      // rel-ops
      char dummy24[sizeof(::ast::Factory::relations_type)];

      // identifiers
      char dummy25[sizeof(::ast::symbols_type)];

      // typespec
      // typespec.opt
      char dummy26[sizeof(::ast::rExp)];

      // formal
      char dummy27[sizeof(::ast::Formal)];

      // formals.1
      // formals.0
      // formals
      char dummy28[sizeof(::ast::Formals*)];
};

    /// Symbol semantic values.
    typedef variant<sizeof(union_type)> semantic_type;
#else
    typedef YYSTYPE semantic_type;
#endif
    /// Symbol locations.
    typedef location location_type;

    /// Syntax errors thrown from user actions.
    struct syntax_error : std::runtime_error
    {
      syntax_error (const location_type& l, const std::string& m);
      location_type location;
    };

    /// Tokens.
    struct token
    {
      /* Tokens.  */
   enum yytokentype {
     TOK_EOF = 0,
     TOK_MODE_EXP = 258,
     TOK_MODE_EXPS = 259,
     TOK___HERE__ = 260,
     TOK_EQ = 261,
     TOK_BREAK = 262,
     TOK_CASE = 263,
     TOK_CATCH = 264,
     TOK_CLOSURE = 265,
     TOK_CONST = 266,
     TOK_CONTINUE = 267,
     TOK_COLON = 268,
     TOK_DEFAULT = 269,
     TOK_ELSE = 270,
     TOK_FINALLY = 271,
     TOK_FREEZEIF = 272,
     TOK_FUNCTION = 273,
     TOK_IF = 274,
     TOK_IN = 275,
     TOK_ISDEF = 276,
     TOK_LBRACE = 277,
     TOK_LBRACKET = 278,
     TOK_LPAREN = 279,
     TOK_ONLEAVE = 280,
     TOK_POINT = 281,
     TOK_RBRACE = 282,
     TOK_RBRACKET = 283,
     TOK_RETURN = 284,
     TOK_RPAREN = 285,
     TOK_STOPIF = 286,
     TOK_SWITCH = 287,
     TOK_THROW = 288,
     TOK_TILDA = 289,
     TOK_TIMEOUT = 290,
     TOK_TRY = 291,
     TOK_VAR = 292,
     TOK_WAITUNTIL = 293,
     TOK_WATCH = 294,
     TOK_WHENEVER = 295,
     TOK_COMMA = 296,
     TOK_SEMICOLON = 297,
     TOK_AMPERSAND = 298,
     TOK_PIPE = 299,
     TOK_EVERY = 300,
     TOK_FOR = 301,
     TOK_LOOP = 302,
     TOK_WHILE = 303,
     TOK_AT = 304,
     TOK_IDENTIFIER = 305,
     TOK_ASSIGN = 306,
     TOK_EMPTY = 307,
     TOK_UNARY = 308,
     TOK_PRIVATE = 309,
     TOK_PROTECTED = 310,
     TOK_PUBLIC = 311,
     TOK_CLASS = 312,
     TOK_PACKAGE = 313,
     TOK_ENUM = 314,
     TOK_EXTERNAL = 315,
     TOK_IMPORT = 316,
     TOK_CARET_EQ = 317,
     TOK_MINUS_EQ = 318,
     TOK_PERCENT_EQ = 319,
     TOK_PLUS_EQ = 320,
     TOK_SLASH_EQ = 321,
     TOK_STAR_EQ = 322,
     TOK_MINUS_MINUS = 323,
     TOK_PLUS_PLUS = 324,
     TOK_MINUS_GT = 325,
     TOK_DO = 326,
     TOK_ASSERT = 327,
     TOK_DETACH = 328,
     TOK_DISOWN = 329,
     TOK_NEW = 330,
     TOK_ANGLE = 331,
     TOK_DURATION = 332,
     TOK_FLOAT = 333,
     TOK_EQ_GT = 334,
     TOK_STRING = 335,
     TOK_QUEST_MARK = 336,
     TOK_CALL = 337,
     TOK_THIS = 338,
     TOK_BANG = 339,
     TOK_BITAND = 340,
     TOK_BITOR = 341,
     TOK_CARET = 342,
     TOK_COMPL = 343,
     TOK_GT_GT = 344,
     TOK_LT_LT = 345,
     TOK_MINUS = 346,
     TOK_PERCENT = 347,
     TOK_PLUS = 348,
     TOK_SLASH = 349,
     TOK_STAR = 350,
     TOK_STAR_STAR = 351,
     TOK_EQ_TILDA_EQ = 352,
     TOK_EQ_EQ = 353,
     TOK_EQ_EQ_EQ = 354,
     TOK_GT_EQ = 355,
     TOK_GT = 356,
     TOK_LT_EQ = 357,
     TOK_LT = 358,
     TOK_BANG_EQ = 359,
     TOK_BANG_EQ_EQ = 360,
     TOK_TILDA_EQ = 361,
     TOK_AMPERSAND_AMPERSAND = 362,
     TOK_PIPE_PIPE = 363,
     TOK_PERCENT_UNSCOPE_COLON = 364,
     TOK_PERCENT_EXP_COLON = 365,
     TOK_PERCENT_LVALUE_COLON = 366,
     TOK_PERCENT_ID_COLON = 367,
     TOK_PERCENT_EXPS_COLON = 368
   };

    };

    /// Token type.
    typedef token::yytokentype token_type;

    /// A complete symbol, with its type.
    template <typename Exact>
    struct symbol_base_type
    {
      /// Default constructor.
      inline symbol_base_type ();

      /// Constructor.
      inline symbol_base_type (const location_type& l);
      inline symbol_base_type (const semantic_type& v, const location_type& l);

      /// Return this with its exact type.
      const Exact& self () const;
      Exact& self ();

      /// Return the type of this symbol.
      int type_get () const;

      /// The semantic value.
      semantic_type value;

      /// The location.
      location_type location;
    };

    /// External form of a symbol: its type and attributes.
    struct symbol_type : symbol_base_type<symbol_type>
    {
      /// The parent class.
      typedef symbol_base_type<symbol_type> super_type;

      /// Default constructor.
      inline symbol_type ();

      /// Constructor for tokens with semantic value.
      inline symbol_type (token_type t, const semantic_type& v, const location_type& l);

      /// Constructor for valueless tokens.
      inline symbol_type (token_type t, const location_type& l);

      /// The symbol type.
      int type;

      /// The symbol type.
      inline int type_get_ () const;

      /// The token.
      inline token_type token () const;
    };
    // Symbol constructors declarations.
    static inline
    symbol_type
    make_EOF (const location_type& l);

    static inline
    symbol_type
    make_MODE_EXP (const location_type& l);

    static inline
    symbol_type
    make_MODE_EXPS (const location_type& l);

    static inline
    symbol_type
    make___HERE__ (const location_type& l);

    static inline
    symbol_type
    make_EQ (const location_type& l);

    static inline
    symbol_type
    make_BREAK (const location_type& l);

    static inline
    symbol_type
    make_CASE (const location_type& l);

    static inline
    symbol_type
    make_CATCH (const location_type& l);

    static inline
    symbol_type
    make_CLOSURE (const location_type& l);

    static inline
    symbol_type
    make_CONST (const location_type& l);

    static inline
    symbol_type
    make_CONTINUE (const location_type& l);

    static inline
    symbol_type
    make_COLON (const location_type& l);

    static inline
    symbol_type
    make_DEFAULT (const location_type& l);

    static inline
    symbol_type
    make_ELSE (const location_type& l);

    static inline
    symbol_type
    make_FINALLY (const location_type& l);

    static inline
    symbol_type
    make_FREEZEIF (const location_type& l);

    static inline
    symbol_type
    make_FUNCTION (const location_type& l);

    static inline
    symbol_type
    make_IF (const location_type& l);

    static inline
    symbol_type
    make_IN (const location_type& l);

    static inline
    symbol_type
    make_ISDEF (const location_type& l);

    static inline
    symbol_type
    make_LBRACE (const location_type& l);

    static inline
    symbol_type
    make_LBRACKET (const location_type& l);

    static inline
    symbol_type
    make_LPAREN (const location_type& l);

    static inline
    symbol_type
    make_ONLEAVE (const location_type& l);

    static inline
    symbol_type
    make_POINT (const location_type& l);

    static inline
    symbol_type
    make_RBRACE (const location_type& l);

    static inline
    symbol_type
    make_RBRACKET (const location_type& l);

    static inline
    symbol_type
    make_RETURN (const location_type& l);

    static inline
    symbol_type
    make_RPAREN (const location_type& l);

    static inline
    symbol_type
    make_STOPIF (const location_type& l);

    static inline
    symbol_type
    make_SWITCH (const location_type& l);

    static inline
    symbol_type
    make_THROW (const location_type& l);

    static inline
    symbol_type
    make_TILDA (const location_type& l);

    static inline
    symbol_type
    make_TIMEOUT (const location_type& l);

    static inline
    symbol_type
    make_TRY (const location_type& l);

    static inline
    symbol_type
    make_VAR (const location_type& l);

    static inline
    symbol_type
    make_WAITUNTIL (const location_type& l);

    static inline
    symbol_type
    make_WATCH (const location_type& l);

    static inline
    symbol_type
    make_WHENEVER (const location_type& l);

    static inline
    symbol_type
    make_COMMA (const ast::flavor_type& v, const location_type& l);

    static inline
    symbol_type
    make_SEMICOLON (const ast::flavor_type& v, const location_type& l);

    static inline
    symbol_type
    make_AMPERSAND (const ast::flavor_type& v, const location_type& l);

    static inline
    symbol_type
    make_PIPE (const ast::flavor_type& v, const location_type& l);

    static inline
    symbol_type
    make_EVERY (const ast::flavor_type& v, const location_type& l);

    static inline
    symbol_type
    make_FOR (const ast::flavor_type& v, const location_type& l);

    static inline
    symbol_type
    make_LOOP (const ast::flavor_type& v, const location_type& l);

    static inline
    symbol_type
    make_WHILE (const ast::flavor_type& v, const location_type& l);

    static inline
    symbol_type
    make_AT (const ast::flavor_type& v, const location_type& l);

    static inline
    symbol_type
    make_IDENTIFIER (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_ASSIGN (const location_type& l);

    static inline
    symbol_type
    make_EMPTY (const location_type& l);

    static inline
    symbol_type
    make_UNARY (const location_type& l);

    static inline
    symbol_type
    make_PRIVATE (const location_type& l);

    static inline
    symbol_type
    make_PROTECTED (const location_type& l);

    static inline
    symbol_type
    make_PUBLIC (const location_type& l);

    static inline
    symbol_type
    make_CLASS (const location_type& l);

    static inline
    symbol_type
    make_PACKAGE (const location_type& l);

    static inline
    symbol_type
    make_ENUM (const location_type& l);

    static inline
    symbol_type
    make_EXTERNAL (const location_type& l);

    static inline
    symbol_type
    make_IMPORT (const location_type& l);

    static inline
    symbol_type
    make_CARET_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_MINUS_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_PERCENT_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_PLUS_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_SLASH_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_STAR_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_MINUS_MINUS (const location_type& l);

    static inline
    symbol_type
    make_PLUS_PLUS (const location_type& l);

    static inline
    symbol_type
    make_MINUS_GT (const location_type& l);

    static inline
    symbol_type
    make_DO (const location_type& l);

    static inline
    symbol_type
    make_ASSERT (const location_type& l);

    static inline
    symbol_type
    make_DETACH (const location_type& l);

    static inline
    symbol_type
    make_DISOWN (const location_type& l);

    static inline
    symbol_type
    make_NEW (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_ANGLE (const libport::ufloat& v, const location_type& l);

    static inline
    symbol_type
    make_DURATION (const libport::ufloat& v, const location_type& l);

    static inline
    symbol_type
    make_FLOAT (const libport::ufloat& v, const location_type& l);

    static inline
    symbol_type
    make_EQ_GT (const location_type& l);

    static inline
    symbol_type
    make_STRING (const std::string& v, const location_type& l);

    static inline
    symbol_type
    make_QUEST_MARK (const location_type& l);

    static inline
    symbol_type
    make_CALL (const location_type& l);

    static inline
    symbol_type
    make_THIS (const location_type& l);

    static inline
    symbol_type
    make_BANG (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_BITAND (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_BITOR (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_CARET (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_COMPL (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_GT_GT (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_LT_LT (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_MINUS (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_PERCENT (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_PLUS (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_SLASH (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_STAR (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_STAR_STAR (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_EQ_TILDA_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_EQ_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_EQ_EQ_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_GT_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_GT (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_LT_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_LT (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_BANG_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_BANG_EQ_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_TILDA_EQ (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_AMPERSAND_AMPERSAND (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_PIPE_PIPE (const libport::Symbol& v, const location_type& l);

    static inline
    symbol_type
    make_PERCENT_UNSCOPE_COLON (const location_type& l);

    static inline
    symbol_type
    make_PERCENT_EXP_COLON (const location_type& l);

    static inline
    symbol_type
    make_PERCENT_LVALUE_COLON (const location_type& l);

    static inline
    symbol_type
    make_PERCENT_ID_COLON (const location_type& l);

    static inline
    symbol_type
    make_PERCENT_EXPS_COLON (const location_type& l);


    /// Build a parser object.
    parser (::parser::ParserImpl& up_yyarg);
    virtual ~parser ();

    /// Parse.
    /// \returns  0 iff parsing succeeded.
    virtual int parse ();

#if YYDEBUG
    /// The current debugging stream.
    std::ostream& debug_stream () const;
    /// Set the current debugging stream.
    void set_debug_stream (std::ostream &);

    /// Type for debugging levels.
    typedef int debug_level_type;
    /// The current debugging level.
    debug_level_type debug_level () const;
    /// Set the current debugging level.
    void set_debug_level (debug_level_type l);
#endif

    /// Report a syntax error.
    /// \param loc    where the syntax error is found.
    /// \param msg    a description of the syntax error.
    virtual void error (const location_type& loc, const std::string& msg);

  private:
    /// State numbers.
    typedef int state_type;

    /// Generate an error message.
    /// \param yystate   the state where the error occurred.
    /// \param yytoken   the lookahead token.
    virtual std::string yysyntax_error_ (state_type yystate, int yytoken);

    /// Compute post-reduction state.
    /// \param yystate   the current state
    /// \param yylhs     the nonterminal to push on the stack
    state_type yy_lr_goto_state_ (state_type yystate, int yylhs);

    /// Whether the given \c yypact_ value indicates a defaulted state.
    /// \param yyvalue   the value to check
    static bool yy_pact_value_is_default_ (int yyvalue);

    /// Whether the given \c yytable_ value indicates a syntax error.
    /// \param yyvalue   the value to check
    static bool yy_table_value_is_error_ (int yyvalue);

    /// Internal symbol numbers.
    typedef unsigned char token_number_type;
    static const short int yypact_ninf_;
    static const short int yytable_ninf_;

    /* Tables.  */
  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.    */
  static const short int yypact_[];

  /* YYDEFACT[S] -- default reduction number in state S.  Performed when
     YYTABLE does not specify something else to do.  Zero means the default
     is an error.    */
  static const unsigned char yydefact_[];

  /* YYPGOTO[NTERM-NUM].    */
  static const short int yypgoto_[];

  /* YYDEFGOTO[NTERM-NUM].    */
  static const short int yydefgoto_[];

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF, syntax error.    */
  static const short int yytable_[];

  static const short int yycheck_[];

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.    */
  static const unsigned char yystos_[];

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.    */
  static const unsigned char yyr1_[];

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.    */
  static const unsigned char yyr2_[];


#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
    /// For a symbol, its name in clear.
    static const char* const yytname_[];
#endif

    /// Convert the symbol name \a n to a form suitable for a diagnostic.
    static std::string yytnamerr_ (const char *n);

#if YYDEBUG
  /* YYRLINEYYN -- Source line where rule number YYN was defined.    */
  static const unsigned short int yyrline_[];
    /// Report on the debug stream that the rule \a r is going to be reduced.
    virtual void yy_reduce_print_ (int r);
    /// Print the state stack on the debug stream.
    virtual void yystack_print_ ();

    /* Debugging.  */
    int yydebug_;
    std::ostream* yycdebug_;
#endif

    /// Convert a scanner token number \a t to a symbol number.
    static inline token_number_type yytranslate_ (token_type t);

#if YYDEBUG
    /// \brief Display a symbol type, value and location.
    /// \param yyo    The output stream.
    /// \param yysym  The symbol.
    template <typename Exact>
    void yy_print_ (std::ostream& yyo,
                    const symbol_base_type<Exact>& yysym) const;
#endif

    /// \brief Reclaim the memory associated to a symbol.
    /// \param yymsg     Why this token is reclaimed.
    ///                  If null, print nothing.
    /// \param s         The symbol.
    template <typename Exact>
    inline void yy_destroy_ (const char* yymsg,
                             symbol_base_type<Exact>& yysym) const;

  private:
    /// Element of the stack: a state and its attributes.
    struct stack_symbol_type : symbol_base_type<stack_symbol_type>
    {
      /// The parent class.
      typedef symbol_base_type<stack_symbol_type> super_type;

      /// Default constructor.
      inline stack_symbol_type ();

      /// Constructor.
      inline stack_symbol_type (state_type s, const semantic_type& v, const location_type& l);

      /// The state.
      state_type state;

      /// The type (corresponding to \a state).
      inline int type_get_ () const;
    };

    /// Stack type.
    typedef stack<stack_symbol_type> stack_type;

    /// The stack.
    stack_type yystack_;

    /// Push a new state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the symbol
    /// \warning the contents of \a s.value is stolen.
    inline void yypush_ (const char* m, stack_symbol_type& s);

    /// Push a new look ahead token on the state on the stack.
    /// \param m    a debug message to display
    ///             if null, no trace is output.
    /// \param s    the state
    /// \param sym  the symbol (for its value and location).
    /// \warning the contents of \a s.value is stolen.
    inline void yypush_ (const char* m, state_type s, symbol_type& sym);

    /// Pop \a n symbols the three stacks.
    inline void yypop_ (unsigned int n = 1);

    /* Constants.  */
    enum
    {
      yyeof_ = 0,
      yylast_ = 1790,           //< Last index in yytable_.
      yynnts_ = 78,  //< Number of nonterminal symbols.
      yyempty_ = -2,
      yyfinal_ = 86, //< Termination state number.
      yyterror_ = 1,
      yyerrcode_ = 256,
      yyntokens_ = 114,   //< Number of tokens.
    };


    /* User arguments.  */
    ::parser::ParserImpl& up;
  };

  // Symbol number corresponding to token number t.
  parser::token_number_type
  parser::yytranslate_ (token_type t)
  {
    static
    const token_number_type
    translate_table[] =
    {
     0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113
    };
    const unsigned int user_token_number_max_ = 368;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }

  inline
  parser::syntax_error::syntax_error (const location_type& l, const std::string& m)
    : std::runtime_error (m)
    , location (l)
  {}

  // symbol_base_type.
  template <typename Exact>
  inline
  parser::symbol_base_type<Exact>::symbol_base_type ()
    : value()
    , location()
  {
  }

  template <typename Exact>
  inline
  parser::symbol_base_type<Exact>::symbol_base_type (const location_type& l)
    : value()
    , location(l)
  {
  }

  template <typename Exact>
  inline
  parser::symbol_base_type<Exact>::symbol_base_type (const semantic_type& v, const location_type& l)
    : value(v)
    , location(l)
  {
  }

  template <typename Exact>
  inline
  const Exact&
  parser::symbol_base_type<Exact>::self () const
  {
    return static_cast<const Exact&>(*this);
  }

  template <typename Exact>
  inline
  Exact&
  parser::symbol_base_type<Exact>::self ()
  {
    return static_cast<Exact&>(*this);
  }

  template <typename Exact>
  inline
  int
  parser::symbol_base_type<Exact>::type_get () const
  {
    return self ().type_get_ ();
  }

  // symbol_type.
  inline
  parser::symbol_type::symbol_type ()
    : super_type ()
    , type ()
  {
  }

  inline
  parser::symbol_type::symbol_type (token_type t, const location_type& l)
    : super_type (l)
    , type (yytranslate_ (t))
  {
  }

  inline
  parser::symbol_type::symbol_type (token_type t, const semantic_type& v, const location_type& l)
    : super_type (v, l)
    , type (yytranslate_ (t))
  {
  }

  inline
  int
  parser::symbol_type::type_get_ () const
  {
    return type;
  }

  inline
  parser::token_type
  parser::symbol_type::token () const
  {
    // YYTOKNUM[NUM] -- (External) token number corresponding to the
    // (internal) symbol number NUM (which must be that of a token).  */
    static
    const unsigned short int
    yytoken_number_[] =
    {
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368
    };
    return static_cast<token_type> (yytoken_number_[type]);
  }
  // Implementation of make_symbol for each symbol type.
  parser::symbol_type
  parser::make_EOF (const location_type& l)
  {
    return symbol_type (token::TOK_EOF, l);
  }

  parser::symbol_type
  parser::make_MODE_EXP (const location_type& l)
  {
    return symbol_type (token::TOK_MODE_EXP, l);
  }

  parser::symbol_type
  parser::make_MODE_EXPS (const location_type& l)
  {
    return symbol_type (token::TOK_MODE_EXPS, l);
  }

  parser::symbol_type
  parser::make___HERE__ (const location_type& l)
  {
    return symbol_type (token::TOK___HERE__, l);
  }

  parser::symbol_type
  parser::make_EQ (const location_type& l)
  {
    return symbol_type (token::TOK_EQ, l);
  }

  parser::symbol_type
  parser::make_BREAK (const location_type& l)
  {
    return symbol_type (token::TOK_BREAK, l);
  }

  parser::symbol_type
  parser::make_CASE (const location_type& l)
  {
    return symbol_type (token::TOK_CASE, l);
  }

  parser::symbol_type
  parser::make_CATCH (const location_type& l)
  {
    return symbol_type (token::TOK_CATCH, l);
  }

  parser::symbol_type
  parser::make_CLOSURE (const location_type& l)
  {
    return symbol_type (token::TOK_CLOSURE, l);
  }

  parser::symbol_type
  parser::make_CONST (const location_type& l)
  {
    return symbol_type (token::TOK_CONST, l);
  }

  parser::symbol_type
  parser::make_CONTINUE (const location_type& l)
  {
    return symbol_type (token::TOK_CONTINUE, l);
  }

  parser::symbol_type
  parser::make_COLON (const location_type& l)
  {
    return symbol_type (token::TOK_COLON, l);
  }

  parser::symbol_type
  parser::make_DEFAULT (const location_type& l)
  {
    return symbol_type (token::TOK_DEFAULT, l);
  }

  parser::symbol_type
  parser::make_ELSE (const location_type& l)
  {
    return symbol_type (token::TOK_ELSE, l);
  }

  parser::symbol_type
  parser::make_FINALLY (const location_type& l)
  {
    return symbol_type (token::TOK_FINALLY, l);
  }

  parser::symbol_type
  parser::make_FREEZEIF (const location_type& l)
  {
    return symbol_type (token::TOK_FREEZEIF, l);
  }

  parser::symbol_type
  parser::make_FUNCTION (const location_type& l)
  {
    return symbol_type (token::TOK_FUNCTION, l);
  }

  parser::symbol_type
  parser::make_IF (const location_type& l)
  {
    return symbol_type (token::TOK_IF, l);
  }

  parser::symbol_type
  parser::make_IN (const location_type& l)
  {
    return symbol_type (token::TOK_IN, l);
  }

  parser::symbol_type
  parser::make_ISDEF (const location_type& l)
  {
    return symbol_type (token::TOK_ISDEF, l);
  }

  parser::symbol_type
  parser::make_LBRACE (const location_type& l)
  {
    return symbol_type (token::TOK_LBRACE, l);
  }

  parser::symbol_type
  parser::make_LBRACKET (const location_type& l)
  {
    return symbol_type (token::TOK_LBRACKET, l);
  }

  parser::symbol_type
  parser::make_LPAREN (const location_type& l)
  {
    return symbol_type (token::TOK_LPAREN, l);
  }

  parser::symbol_type
  parser::make_ONLEAVE (const location_type& l)
  {
    return symbol_type (token::TOK_ONLEAVE, l);
  }

  parser::symbol_type
  parser::make_POINT (const location_type& l)
  {
    return symbol_type (token::TOK_POINT, l);
  }

  parser::symbol_type
  parser::make_RBRACE (const location_type& l)
  {
    return symbol_type (token::TOK_RBRACE, l);
  }

  parser::symbol_type
  parser::make_RBRACKET (const location_type& l)
  {
    return symbol_type (token::TOK_RBRACKET, l);
  }

  parser::symbol_type
  parser::make_RETURN (const location_type& l)
  {
    return symbol_type (token::TOK_RETURN, l);
  }

  parser::symbol_type
  parser::make_RPAREN (const location_type& l)
  {
    return symbol_type (token::TOK_RPAREN, l);
  }

  parser::symbol_type
  parser::make_STOPIF (const location_type& l)
  {
    return symbol_type (token::TOK_STOPIF, l);
  }

  parser::symbol_type
  parser::make_SWITCH (const location_type& l)
  {
    return symbol_type (token::TOK_SWITCH, l);
  }

  parser::symbol_type
  parser::make_THROW (const location_type& l)
  {
    return symbol_type (token::TOK_THROW, l);
  }

  parser::symbol_type
  parser::make_TILDA (const location_type& l)
  {
    return symbol_type (token::TOK_TILDA, l);
  }

  parser::symbol_type
  parser::make_TIMEOUT (const location_type& l)
  {
    return symbol_type (token::TOK_TIMEOUT, l);
  }

  parser::symbol_type
  parser::make_TRY (const location_type& l)
  {
    return symbol_type (token::TOK_TRY, l);
  }

  parser::symbol_type
  parser::make_VAR (const location_type& l)
  {
    return symbol_type (token::TOK_VAR, l);
  }

  parser::symbol_type
  parser::make_WAITUNTIL (const location_type& l)
  {
    return symbol_type (token::TOK_WAITUNTIL, l);
  }

  parser::symbol_type
  parser::make_WATCH (const location_type& l)
  {
    return symbol_type (token::TOK_WATCH, l);
  }

  parser::symbol_type
  parser::make_WHENEVER (const location_type& l)
  {
    return symbol_type (token::TOK_WHENEVER, l);
  }

  parser::symbol_type
  parser::make_COMMA (const ast::flavor_type& v, const location_type& l)
  {
    return symbol_type (token::TOK_COMMA, v, l);
  }

  parser::symbol_type
  parser::make_SEMICOLON (const ast::flavor_type& v, const location_type& l)
  {
    return symbol_type (token::TOK_SEMICOLON, v, l);
  }

  parser::symbol_type
  parser::make_AMPERSAND (const ast::flavor_type& v, const location_type& l)
  {
    return symbol_type (token::TOK_AMPERSAND, v, l);
  }

  parser::symbol_type
  parser::make_PIPE (const ast::flavor_type& v, const location_type& l)
  {
    return symbol_type (token::TOK_PIPE, v, l);
  }

  parser::symbol_type
  parser::make_EVERY (const ast::flavor_type& v, const location_type& l)
  {
    return symbol_type (token::TOK_EVERY, v, l);
  }

  parser::symbol_type
  parser::make_FOR (const ast::flavor_type& v, const location_type& l)
  {
    return symbol_type (token::TOK_FOR, v, l);
  }

  parser::symbol_type
  parser::make_LOOP (const ast::flavor_type& v, const location_type& l)
  {
    return symbol_type (token::TOK_LOOP, v, l);
  }

  parser::symbol_type
  parser::make_WHILE (const ast::flavor_type& v, const location_type& l)
  {
    return symbol_type (token::TOK_WHILE, v, l);
  }

  parser::symbol_type
  parser::make_AT (const ast::flavor_type& v, const location_type& l)
  {
    return symbol_type (token::TOK_AT, v, l);
  }

  parser::symbol_type
  parser::make_IDENTIFIER (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_IDENTIFIER, v, l);
  }

  parser::symbol_type
  parser::make_ASSIGN (const location_type& l)
  {
    return symbol_type (token::TOK_ASSIGN, l);
  }

  parser::symbol_type
  parser::make_EMPTY (const location_type& l)
  {
    return symbol_type (token::TOK_EMPTY, l);
  }

  parser::symbol_type
  parser::make_UNARY (const location_type& l)
  {
    return symbol_type (token::TOK_UNARY, l);
  }

  parser::symbol_type
  parser::make_PRIVATE (const location_type& l)
  {
    return symbol_type (token::TOK_PRIVATE, l);
  }

  parser::symbol_type
  parser::make_PROTECTED (const location_type& l)
  {
    return symbol_type (token::TOK_PROTECTED, l);
  }

  parser::symbol_type
  parser::make_PUBLIC (const location_type& l)
  {
    return symbol_type (token::TOK_PUBLIC, l);
  }

  parser::symbol_type
  parser::make_CLASS (const location_type& l)
  {
    return symbol_type (token::TOK_CLASS, l);
  }

  parser::symbol_type
  parser::make_PACKAGE (const location_type& l)
  {
    return symbol_type (token::TOK_PACKAGE, l);
  }

  parser::symbol_type
  parser::make_ENUM (const location_type& l)
  {
    return symbol_type (token::TOK_ENUM, l);
  }

  parser::symbol_type
  parser::make_EXTERNAL (const location_type& l)
  {
    return symbol_type (token::TOK_EXTERNAL, l);
  }

  parser::symbol_type
  parser::make_IMPORT (const location_type& l)
  {
    return symbol_type (token::TOK_IMPORT, l);
  }

  parser::symbol_type
  parser::make_CARET_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_CARET_EQ, v, l);
  }

  parser::symbol_type
  parser::make_MINUS_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_MINUS_EQ, v, l);
  }

  parser::symbol_type
  parser::make_PERCENT_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_PERCENT_EQ, v, l);
  }

  parser::symbol_type
  parser::make_PLUS_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_PLUS_EQ, v, l);
  }

  parser::symbol_type
  parser::make_SLASH_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_SLASH_EQ, v, l);
  }

  parser::symbol_type
  parser::make_STAR_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_STAR_EQ, v, l);
  }

  parser::symbol_type
  parser::make_MINUS_MINUS (const location_type& l)
  {
    return symbol_type (token::TOK_MINUS_MINUS, l);
  }

  parser::symbol_type
  parser::make_PLUS_PLUS (const location_type& l)
  {
    return symbol_type (token::TOK_PLUS_PLUS, l);
  }

  parser::symbol_type
  parser::make_MINUS_GT (const location_type& l)
  {
    return symbol_type (token::TOK_MINUS_GT, l);
  }

  parser::symbol_type
  parser::make_DO (const location_type& l)
  {
    return symbol_type (token::TOK_DO, l);
  }

  parser::symbol_type
  parser::make_ASSERT (const location_type& l)
  {
    return symbol_type (token::TOK_ASSERT, l);
  }

  parser::symbol_type
  parser::make_DETACH (const location_type& l)
  {
    return symbol_type (token::TOK_DETACH, l);
  }

  parser::symbol_type
  parser::make_DISOWN (const location_type& l)
  {
    return symbol_type (token::TOK_DISOWN, l);
  }

  parser::symbol_type
  parser::make_NEW (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_NEW, v, l);
  }

  parser::symbol_type
  parser::make_ANGLE (const libport::ufloat& v, const location_type& l)
  {
    return symbol_type (token::TOK_ANGLE, v, l);
  }

  parser::symbol_type
  parser::make_DURATION (const libport::ufloat& v, const location_type& l)
  {
    return symbol_type (token::TOK_DURATION, v, l);
  }

  parser::symbol_type
  parser::make_FLOAT (const libport::ufloat& v, const location_type& l)
  {
    return symbol_type (token::TOK_FLOAT, v, l);
  }

  parser::symbol_type
  parser::make_EQ_GT (const location_type& l)
  {
    return symbol_type (token::TOK_EQ_GT, l);
  }

  parser::symbol_type
  parser::make_STRING (const std::string& v, const location_type& l)
  {
    return symbol_type (token::TOK_STRING, v, l);
  }

  parser::symbol_type
  parser::make_QUEST_MARK (const location_type& l)
  {
    return symbol_type (token::TOK_QUEST_MARK, l);
  }

  parser::symbol_type
  parser::make_CALL (const location_type& l)
  {
    return symbol_type (token::TOK_CALL, l);
  }

  parser::symbol_type
  parser::make_THIS (const location_type& l)
  {
    return symbol_type (token::TOK_THIS, l);
  }

  parser::symbol_type
  parser::make_BANG (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_BANG, v, l);
  }

  parser::symbol_type
  parser::make_BITAND (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_BITAND, v, l);
  }

  parser::symbol_type
  parser::make_BITOR (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_BITOR, v, l);
  }

  parser::symbol_type
  parser::make_CARET (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_CARET, v, l);
  }

  parser::symbol_type
  parser::make_COMPL (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_COMPL, v, l);
  }

  parser::symbol_type
  parser::make_GT_GT (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_GT_GT, v, l);
  }

  parser::symbol_type
  parser::make_LT_LT (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_LT_LT, v, l);
  }

  parser::symbol_type
  parser::make_MINUS (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_MINUS, v, l);
  }

  parser::symbol_type
  parser::make_PERCENT (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_PERCENT, v, l);
  }

  parser::symbol_type
  parser::make_PLUS (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_PLUS, v, l);
  }

  parser::symbol_type
  parser::make_SLASH (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_SLASH, v, l);
  }

  parser::symbol_type
  parser::make_STAR (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_STAR, v, l);
  }

  parser::symbol_type
  parser::make_STAR_STAR (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_STAR_STAR, v, l);
  }

  parser::symbol_type
  parser::make_EQ_TILDA_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_EQ_TILDA_EQ, v, l);
  }

  parser::symbol_type
  parser::make_EQ_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_EQ_EQ, v, l);
  }

  parser::symbol_type
  parser::make_EQ_EQ_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_EQ_EQ_EQ, v, l);
  }

  parser::symbol_type
  parser::make_GT_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_GT_EQ, v, l);
  }

  parser::symbol_type
  parser::make_GT (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_GT, v, l);
  }

  parser::symbol_type
  parser::make_LT_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_LT_EQ, v, l);
  }

  parser::symbol_type
  parser::make_LT (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_LT, v, l);
  }

  parser::symbol_type
  parser::make_BANG_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_BANG_EQ, v, l);
  }

  parser::symbol_type
  parser::make_BANG_EQ_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_BANG_EQ_EQ, v, l);
  }

  parser::symbol_type
  parser::make_TILDA_EQ (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_TILDA_EQ, v, l);
  }

  parser::symbol_type
  parser::make_AMPERSAND_AMPERSAND (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_AMPERSAND_AMPERSAND, v, l);
  }

  parser::symbol_type
  parser::make_PIPE_PIPE (const libport::Symbol& v, const location_type& l)
  {
    return symbol_type (token::TOK_PIPE_PIPE, v, l);
  }

  parser::symbol_type
  parser::make_PERCENT_UNSCOPE_COLON (const location_type& l)
  {
    return symbol_type (token::TOK_PERCENT_UNSCOPE_COLON, l);
  }

  parser::symbol_type
  parser::make_PERCENT_EXP_COLON (const location_type& l)
  {
    return symbol_type (token::TOK_PERCENT_EXP_COLON, l);
  }

  parser::symbol_type
  parser::make_PERCENT_LVALUE_COLON (const location_type& l)
  {
    return symbol_type (token::TOK_PERCENT_LVALUE_COLON, l);
  }

  parser::symbol_type
  parser::make_PERCENT_ID_COLON (const location_type& l)
  {
    return symbol_type (token::TOK_PERCENT_ID_COLON, l);
  }

  parser::symbol_type
  parser::make_PERCENT_EXPS_COLON (const location_type& l)
  {
    return symbol_type (token::TOK_PERCENT_EXPS_COLON, l);
  }



} // yy
/* Line 347 of lalr1.cc  */
#line 2053 "/home/bearclaw/aldebaran/qi-2/urbi/kernel/build-linux32/src/parser/ugrammar.hh"



#endif /* ! defined PARSER_HEADER_H */
