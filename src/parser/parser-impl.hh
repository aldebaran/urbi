/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
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

# include <ast/fwd.hh>
# include <parser/fwd.hh>
# include <parser/parse-result.hh>
# include <parser/utoken.hh>

namespace parser
{

  class ParserImpl
  {
  public:
    typedef yy::parser parser_type;
    typedef parser_type::location_type location_type;
    typedef parser_type::symbol_type symbol_type;

    ParserImpl();

    /// Parse the command from a buffer.
    /// \param code  the source to parse.
    /// \param loc   the location to use for this parsing.
    parse_result_type parse(const std::string& code,
                            const location_type* loc = 0);

    /// Parse a file.
    parse_result_type parse_file(const std::string& fn);

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
    void parse_(std::istream& source, const location_type* loc = 0);

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
  };

}

# include <parser/parser-impl.hxx>

#endif // !PARSER_PARSER_IMPL_HH
