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

# include <memory>
# include <string>

# include "parser/fwd.hh"

namespace ast
{
  class Nary;
}

namespace parser
{

  class UParser
  {
  public:
    UParser();
    UParser(const UParser&);
    ~UParser();

    /// Parse the command from a buffer.
    /// \return yyparse's result (0 on success).
    int parse(const std::string& code);

    /// Parse the command from a TWEAST.
    /// \param t  the Tweast to process.  Emptied as it is used.
    /// \return yyparse's result (0 on success).
    /// \note   Recursive calls are forbidden.
    int parse(parser::Tweast& t);

    /// Parse a file.
    /// \return yyparse's result (0 on success).
    int parse_file(const std::string& fn);

    /// \{
    /// The type of AST node returned by the parser.
    typedef ast::Nary ast_type;
    /// Return the AST and reset \a ast_.
    std::auto_ptr<ast_type> ast_take();
    /// Same as \a ast_take, but assert the result.
    std::auto_ptr<ast_type> ast_xtake();
    /// \}

    /// Push all warning and error messages in \b target.
    /// If errors were pushed, the ast is deleted and set to 0.
    void process_errors(ast_type* target);

    /// Dump all the errors on std::cerr.
    /// For developpers.
    void dump_errors() const;

  private:
    std::auto_ptr<ParserImpl> pimpl_;
  };


/*--------------------------.
| Free-standing functions.  |
`--------------------------*/

  /// Parse \a cmd, and return a UParser that holds the result and the
  /// errors.  Admittedly, we could avoid using UParser at all and
  /// return a tuple, maybe in another step.
  UParser parse(const std::string& cmd);

  /// Parse a Tweast and return the (desugared) AST.
  UParser::ast_type* parse(Tweast& t);

  /// Parse a file.
  /// \see parse().
  UParser parse_file(const std::string& file);
}

#endif // !PARSER_UPARSER_HH
