/// \file parser/parse-result.hh

#ifndef PARSER_PARSE_RESULT_HH
# define PARSER_PARSE_RESULT_HH

# include <libport/shared-ptr.hh>
# include <list>
# include <string>

namespace ast
{
  class Nary;
}

namespace parser
{

  /// A structure to record the result of a parsing (AST and errors).
  ///
  /// This structure is used to decouple the parser and all its
  /// internal data from the user's space.  There are two main
  /// advantages.
  ///
  /// Firstly, the user needs little headers to include.
  ///
  /// Secondly, it allows a very functional style: the parsing returns
  /// a single object containing all the aspects of the result, and
  /// keeps no relation to it.  This is much clearer than a previous
  /// interfave where the users had to keep the UParser to ask it
  /// first its ast, then its errors etc.  It was then quite unclear
  /// when such a UParser could be used again for another parsing
  /// session.  Now it's easy: when parsing is done, it's ready for
  /// another one.
  class ParseResult
  {
  public:
    ParseResult();
    ParseResult(ParseResult& rhs);

    /// Errors must have been dumped or cleared.
    /// This is to avoid that parse errors get lost.
    ~ParseResult();

    /// The status as reported by Bison's parser.
    int status;
    /// Whether everything is fine.
    /// The status is 0, the ast is defined, there are no warnings/errors.
    bool good() const;

    /// \name The resulting AST.
    /// \{
    /// The type of AST node returned by the parser.
    typedef ast::Nary ast_type;
    /// The latest AST read by parse().
    ast_type* ast_get();
    /// Return the AST and reset \a ast_.
    libport::shared_ptr<ast_type> ast_take();
    /// Same as \a ast_take, but assert the result.
    libport::shared_ptr<ast_type> ast_xtake();
    /// Set \a ast_.
    void ast_set(libport::shared_ptr<ast_type> ast);
    /// Give \a ast.
    void ast_reset(libport::shared_ptr<ast_type> ast = 0);

  private:
    /// The resulting AST.
      libport::shared_ptr<ast_type> ast_;
    /// \}

  public:
    /// Declare an error about \a msg.
    void error(const std::string& msg);

    /// Warn about \a msg.
    void warn(const std::string& msg);

    /// Dump all the errors on std::cerr.
    /// For developpers.
    void dump_errors() const;

    /// Push all warning and error messages in \b target.
    /// If errors were pushed, the ast is deleted and set to 0.
    void process_errors(ast_type& target);

  private:
    /// Errors and warnings.
    typedef std::list<std::string> messages_type;

    /// List of parse error messages.
    messages_type errors_;

    /// List of warnings.
    messages_type warnings_;

    /// Whether the errors and warnings were output or given.
    mutable bool reported_;

  public:
    /// Dump for debugging.
    std::ostream& dump(std::ostream& o) const;
  };

  /// Dump \a p on \a o for debugging.
  std::ostream& operator<<(std::ostream& o, const ParseResult& p);
}


# include <parser/parse-result.hxx>

#endif // !PARSER_PARSE_RESULT_HH
