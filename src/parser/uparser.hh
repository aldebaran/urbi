/*! \file uparser.hh
 *******************************************************************************

 File: uparser.h\n
 Definition of the UParser class used to make flex/bison reentrant.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef PARSER_UPARSER_HH
# define PARSER_UPARSER_HH

# include <list>
# include <memory>
# include <string>

# include "parser/fwd.hh"
# include "parser/utoken.hh"

namespace parser
{

  //! UParser uses 'flex' and 'bison' as scanner/parser
  /*! The choice of flex/bison is detailed in the comment on the UServer class.
      The main concern is about reentrancy, which is not necessary in most
      cases but would become a problem with a multithreaded server.
  */
  class UParser
  {
  public:
    typedef yy::parser parser_type;
    typedef parser_type::token_type token_type;
    typedef parser_type::semantic_type semantic_type;
    typedef parser_type::location_type location_type;

    UParser ();

    /// Parse the command from a buffer.
    /// \return yyparse's result (0 on success).
    int parse (const std::string& code);

    /// Parse the command from a TWEAST.
    /// \return yyparse's result (0 on success).
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
    inline std::auto_ptr<ast_type> ast_take ();
    /// Same as \a ast_take, but assert the result.
    inline std::auto_ptr<ast_type> ast_xtake ();
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
		      const yy::parser::location_type& l,
		      const std::string& msg);

    /// Run the parse.
    int parse_ (std::istream& source);

    /// The current location.
    location_type loc_;

    /// A stack of locations to support #push/#pop.
    std::stack<yy::location> synclines_;
  };


/*--------------------------.
| Free-standing functions.  |
`--------------------------*/

  /// Parse \a cmd, and return a UParser that holds the result and the
  /// errors.  Admittedly, we could avoid using UParser at all and
  /// return a tuple, maybe in another step.
  UParser parse(const std::string& cmd);

  /// Parse a file.
  /// \see parse().
  UParser parse_file(const std::string& file);
}

# include "parser/uparser.hxx"

#endif // !PARSER_UPARSER_HH
