/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_RUNNER_EXCEPTION_HH
# define URBI_RUNNER_EXCEPTION_HH

# include <vector>

# include <ast/loc.hh>
# include <urbi/runner/fwd.hh>

namespace runner
{
  class Exception
  {
  public:
    Exception();
    /// Report a fatal error.
    void err(const ast::loc& loc, const std::string& msg,
	     const std::string& prefix = "");
    /// Report a warning.
    static
    void warn(const ast::loc& loc, const std::string& msg,
	      const std::string& prefix = "");
    /// Whether there's any fatal error.
    bool empty() const;
    /// Clear all stored errors.
    void clear();
    /// Print all fatal errors (via the runner).
    void print(runner::Job& c) const;

    /// For debugging only, e.g., in ast-dump.
    std::ostream& dump(std::ostream& o) const;

    /// Raise an adequate Urbi exception.
    /// precondition: !empty().
    ATTRIBUTE_NORETURN
    void raise(const std::string& input) const;

  private:
    struct Message
    {
      Message(const ast::loc& loc, const std::string& kind,
              const std::string& msg, const std::string& prefix);
      /// Format loc, msg, and prefix.
      std::string message() const;
      /// Send the message via the runner.
      void print(runner::Job& r) const;
      /// For debugging only, e.g., in ast-dump.
      std::ostream& dump(std::ostream& o) const;

      ast::loc loc;
      std::string kind, msg, prefix;
    };
    typedef std::vector<Message> messages_type;
    messages_type messages_;
  };

  /// For debugging only, e.g., in ast-dump.
  std::ostream&
  operator<<(std::ostream& o, const Exception& e);
}

#endif
