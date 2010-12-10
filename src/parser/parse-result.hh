/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file parser/parse-result.hh

#ifndef PARSER_PARSE_RESULT_HH
# define PARSER_PARSE_RESULT_HH

# include <libport/intrusive-ptr.hh>
# include <list>
# include <string>

# include <ast/nary.hh>
# include <urbi/export.hh>

namespace parser
{

  /// A structure to record the result of a parsing (AST).
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
  /// interface where the users had to keep the UParser to ask it
  /// first its AST, then its errors etc.  It was then quite unclear
  /// when such a UParser could be used again for another parsing
  /// session.  Now it's easy: when parsing is done, it's ready for
  /// another one.
  class URBI_SDK_API ParseResult
  {
  public:
    ParseResult();
    ParseResult(ParseResult& rhs);

    /// Errors must have been dumped or cleared.
    /// This is to avoid that parse errors get lost.
    ~ParseResult();

    /// Whether everything is fine.
    /// The status is 0, the ast is defined, there are no errors.
    /// Warnings are allowed though.
    bool good() const;

    /// Whether everything is perfect.
    /// Must be good(), and no warnings.
    bool perfect() const;

    /// \name The resulting AST.
    /// \{
    /// The type of AST node returned by the parser.
    typedef ast::rExp ast_type;
    /// The latest AST read by parse().
    ast_type ast_get();
    /// Same as \a ast_get, but assert the result.
    ast_type ast_xget();
    /// Set \a ast_.
    void ast_set(ast_type ast);

  private:
    /// The resulting AST.
    ast_type ast_;
    /// \}

  public:
    /// Dump for debugging.
    std::ostream& dump(std::ostream& o) const;
  };

  /// Dump \a p on \a o for debugging.
  std::ostream& operator<<(std::ostream& o, const ParseResult& p);
}


# include <parser/parse-result.hxx>

#endif // !PARSER_PARSE_RESULT_HH
