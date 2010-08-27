/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
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
# include <ast/message.hh>
# include <ast/nary-fwd.hh>

namespace ast
{

  /// Storing (static) errors and warnings about some AST.
  class Error : public libport::RefCounted
  {
  public:
    Error();
    virtual ~Error();

    /// Whether there are no errors.
    bool good() const;

    /// Whether there are no warnings/errors.
    bool empty() const;

  public:
    /// Errors and warnings.
    typedef std::list<rMessage> messages_type;

    /// Declare an error about \a msg.
    void error(const loc& l, const std::string& msg);

    /// Warn about \a msg.
    void warn(const loc& l, const std::string& msg);

    /// The list of messages.
    const messages_type& messages_get() const;

    /// The list of messages.
    messages_type& messages_get();

    /// Dump all the errors on std::cerr.
    /// For developpers.
    void dump_errors() const;

    /// Push all warning and error messages in \b target.
    /// If errors were pushed, the ast is deleted and set to 0.
    void process_errors(Nary& target);

  private:
    /// Record a new error/warning.
    /// \param kind   "error" or "warning".
    void message_(const loc& l,
                  const std::string& kind, const std::string& msg);

    /// List of parse error/warnings messages.
    messages_type messages_;

    /// Whether the errors and warnings were output or given.
    mutable bool reported_;

  public:
    /// Dump for debugging.
    std::ostream& dump(std::ostream& o) const;
  };

  /// Dump \a p on \a o for debugging.
  std::ostream& operator<<(std::ostream& o, const Error& p);

  typedef libport::intrusive_ptr<Error> rError;
  typedef libport::intrusive_ptr<const Error> rConstError;

} // namespace ast

# include <ast/error.hxx>

#endif // !AST_ERROR_HH
