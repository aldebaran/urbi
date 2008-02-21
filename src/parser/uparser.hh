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
# include <string>

# include "parser/fwd.hh"
# include "parser/utoken.hh"

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
  int process (const std::string& code);

  /// Parse the command from a TWEAST.
  /// \return yyparse's result (0 on success).
  int process (parser::Tweast& t);

  parser::Tweast* tweast_;

  /// Parse a file.
  /// \return yyparse's result (0 on success).
  int process_file (const std::string& fn);


  /// \{
  /// The latest AST read by process().
  ast::Ast* ast_get ();
  const ast::Ast* ast_get () const;
  void ast_set (ast::Ast* ast);
private:
  ast::Ast* ast_;
  /// \}

public:
  /// Declare an error at \a l about \a msg.
  void error (const location_type& l, const std::string& msg);

  /// Warn at \a l about \a msg.
  void warn (const location_type& l, const std::string& msg);

  /// Push all warning and error messages in \b target.
  /// If errors were pushed, the command tree is deleted and set to 0.
  void process_errors(ast::Nary* target);

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

#endif // !PARSER_UPARSER_HH
