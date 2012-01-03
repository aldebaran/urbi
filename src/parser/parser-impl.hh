/*
 * Copyright (C) 2005-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file parser/parser-impl.hh

#ifndef PARSER_PARSER_IMPL_HH
# define PARSER_PARSER_IMPL_HH

# include <list>
# include <memory>
# include <string>

# include <boost/optional.hpp>

# include <ast/fwd.hh>
# include <parser/fwd.hh>
# include <parser/utoken.hh>
# include <runner/exception.hh>

namespace parser
{

  class ParserImpl
  {
  public:
    typedef yy::parser parser_type;
    typedef parser_type::location_type location_type;
    typedef parser_type::token_type token_type;
    typedef parser_type::symbol_type symbol_type;

    ParserImpl(std::istream& input);
    void initial_token_set(token_type initial_token);
    boost::optional<token_type>& initial_token_get();


    /// Parse and store the result in \c result_.
    /// \param source  what's to parse.
    /// \param loc     its location
    ///
    /// If loc is null, then the parsing proceeds with loc_.
    /// This is used to implement some "continuity": a single source
    /// is sent by chunks to this parser, and loc_ keeps track of
    /// the whole parsing sequence.
    ///
    /// If loc is non null, then loc_ is saved, the parsing proceeds
    /// with loc, and then loc_ is restored.
    ///
    /// This behavior is quite complex and should probably be redesigned.
    parse_result_type parse(const location_type* loc = 0);

    /// Whether meta-variables are enabled.
    bool meta() const;

    /// Enable/disable meta variables support.
    /// Disabled by default.
    void meta(bool b);

    /// Declare an error at \a l about \a msg.
    void error(const location_type& l, const std::string& msg);

    /// Warn at \a l about \a msg.
    void warn(const location_type& l, const std::string& msg);

    /// \param what        the obsolete construct.
    /// \param suggestion  the replacement.
    void deprecated(const location_type& loc,
                    const std::string& what,
                    const std::string& suggestion = "");
    /// The factory.
    const ast::Factory& factory() const;

  private:
    // Give access to loc_.
    friend int parser_type::parse();
    friend YY_DECL;

  public:
    /// The scanner.
    yyFlexLexer scanner_;

  private:
    /// The scanner input stream.
    std::istream& input_;

    /// The current location.
    ///
    /// The real parser (ugrammar.y) fetches its starting location
    /// here, and saves the final location when the parsing
    /// was successful.
    location_type loc_;

    /// A stack of locations to support //#push and //#pop.
    std::stack<yy::location> synclines_;

    /// The resut of the parsing.
    parse_result_type result_;

    /// Whether in debug mode.
    /// Enabled when the envvar YYDEBUG is defined (whatever the value).
    bool debug_;

    /// Whether meta-variables are enabled.
    bool meta_;

    /// Factory.
    std::auto_ptr<ast::Factory> factory_;

    /// Initial token to select mode
    boost::optional<yy::parser::token_type> initial_token_;

    /// Errors so far
    runner::Exception errors_;
  };

}

# include <parser/parser-impl.hxx>

#endif // !PARSER_PARSER_IMPL_HH
