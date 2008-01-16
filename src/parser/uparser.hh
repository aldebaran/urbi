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

#ifndef UPARSER_HH
# define UPARSER_HH

# include <list>
# include <set>
# include <string>

# include "kernel/fwd.hh"
# include "kernel/utypes.hh"

# include "ugrammar.hh"
# include "parser/flex-lexer.hh"


# undef  YY_DECL
# define YY_DECL                                                 \
  UParser::token_type						 \
  yyFlexLexer::yylex(UParser::semantic_type* valp,		 \
		     UParser::location_type* locp,		 \
		     UParser& up)

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

  UParser (UConnection& cn);

  /// Parse the command from a buffer.
  /// \return yyparse's result (0 on success).
  int process (const std::string& code);

  /// Parse a file.
  /// \return yyparse's result (0 on success).
  int process_file (const std::string& fn);

  /// \{
  /// The last AST read by process().
  ast::Ast* command_tree_get ();
  const ast::Ast* command_tree_get () const;
  void command_tree_set (ast::Ast* ast);
private:
  ast::Ast* command_tree_;
  /// \}

public:
  bool binaryCommand;

  token_type scan(semantic_type* val, location_type* loc);

  /// Declare an error at \a l about \a msg.
  void error (const location_type& l, const std::string& msg);

  /// Warn at \a l about \a msg.
  void warn (const location_type& l, const std::string& msg);

  /// The connection we belong to.
  UConnection& connection;

  /// @name Errors and warnings handling
  /// @{

  /// Whether one or several errors occured during parsing.
  bool hasError() const;
  /// Give the oldest error message.
  std::string error_get() const;
  /// Pop the oldest error message.
  void error_pop ();

  /// Whether one or several warnings occured during parsing.
  bool hasWarning() const;
  /// Give the oldest warning message.
  std::string warning_get() const;
  /// Pop the oldest warning message.
  void warning_pop ();

  /// @}

private:
  // Give access to loc_ and scanner_.
  friend int parser_type::parse ();
  friend token_type yylex (semantic_type*, location_type*, UParser&);

  /// List of parse error messages.
  std::list<std::string> errors_;

  /// List of warnings.
  std::list<std::string> warnings_;

  /// Run the parse.  Expects the scanner to be initialized.
  int parse_ ();

  /// The Flex scanner.
  yyFlexLexer scanner_;

  /// The current location.
  location_type loc_;
};

#endif
