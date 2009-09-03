/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/**
 ** \file ast/error.hh
 ** \brief Declaration of ast::Error.
 */

#ifndef AST_ERROR_HH
# define AST_ERROR_HH

# include <iosfwd>
# include <list>
# include <string>

# include <ast/loc.hh>

namespace ast
{

  class Nary;

  /// Storing (static) errors and warnings about some AST.
  class Error
  {
  public:
    Error();
    virtual ~Error();

    /// Whether there are no errors.
    bool good() const;

    /// Whether there are no warnings/errors.
    bool empty() const;

  public:
    /// Declare an error about \a msg.
    void error(const loc& l, const std::string& msg);

    /// Warn about \a msg.
    void warn(const loc& l, const std::string& msg);

    /// Dump all the errors on std::cerr.
    /// For developpers.
    void dump_errors() const;

    /// Push all warning and error messages in \b target.
    /// If errors were pushed, the ast is deleted and set to 0.
    void process_errors(Nary& target);

  private:
    /// Errors and warnings.
    typedef std::list<std::string> messages_type;

    /// Record a new error/warning.
    void message_(messages_type& ms, const loc& l, const std::string& msg);

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
  std::ostream& operator<<(std::ostream& o, const Error& p);

} // namespace ast

# include <ast/error.hxx>

#endif // !AST_ERROR_HH
