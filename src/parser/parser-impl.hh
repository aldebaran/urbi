/// \file parser/parser-impl.hh

#ifndef PARSER_PARSER_IMPL_HH
# define PARSER_PARSER_IMPL_HH

# include <list>
# include <memory>
# include <string>

# include "parser/fwd.hh"
# include "parser/utoken.hh"

namespace parser
{

  /// This is a private auxiliary structure, so yes, I want to
  /// access its fields easily.
  struct ParserImpl
  {
  public:
    typedef yy::parser parser_type;
    typedef parser_type::token_type token_type;
    typedef parser_type::semantic_type semantic_type;
    typedef parser_type::location_type location_type;

    ParserImpl ();

    /// Parse the command from a buffer.
    /// \return yyparse's result (0 on success).
    int parse (const std::string& code);

    /// Parse the command from a TWEAST.
    /// \param t  the Tweast to process.  Emptied as it is used.
    /// \return yyparse's result (0 on success).
    /// \note   Recursive calls are forbidden.
    int parse (parser::Tweast& t);

    /// The TWEAST currently analyzed (if there is one).
    parser::Tweast* tweast_;

    /// Parse a file.
    /// \return yyparse's result (0 on success).
    int parse_file (const std::string& fn);


    /// \{
    /// The type of AST node returned by the parser.
    typedef ast::Nary ast_type;
    /// The latest AST read by parse().
    inline ast_type* ast_get ();
    /// Return the AST and reset \a ast_.
    std::auto_ptr<ast_type> ast_take();
    /// Same as \a ast_take, but assert the result.
    std::auto_ptr<ast_type> ast_xtake();
    /// Set \a ast_.
    inline void ast_set (ast_type* ast);

  private:
    /// The resulting AST.
    ast_type* ast_;
    /// \}

  public:
    /// Declare an error at \a l about \a msg.
    void error (const location_type& l, const std::string& msg);

    /// Warn at \a l about \a msg.
    void warn (const location_type& l, const std::string& msg);

    /// Push all warning and error messages in \b target.
    /// If errors were pushed, the ast is deleted and set to 0.
    void process_errors(ast_type* target);

    /// Dump all the errors on std::cerr.
    /// For developpers.
    void dump_errors() const;

  private:
    // Give access to loc_.
    friend int parser_type::parse ();
    friend YY_DECL;

    /// Errors and warnings.
    typedef std::list<std::string> messages_type;

    /// List of parse error messages.
    messages_type errors_;

    /// List of warnings.
    messages_type warnings_;

    /// Push a warning or an error.
    void message_push(messages_type& msgs,
		      const location_type& l,
		      const std::string& msg);

    /// Run the parse.
    int parse_ (std::istream& source);

    /// The current location.
    location_type loc_;

    /// A stack of locations to support #push/#pop.
    std::stack<yy::location> synclines_;
  };

}

# include "parser/parser-impl.hxx"

#endif // !PARSER_PARSER_IMPL_HH
