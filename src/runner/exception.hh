/*
 * Copyright (C) 2010, 2012, Gostai S.A.S.
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
# include <runner/runner.hh>

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
    /// Print all fatal errors.
    void print(runner::Runner& c) const;
    /// Raise an adequate Urbi exception.
    /// precondition: !empty().
    ATTRIBUTE_NORETURN
    void raise(const std::string& input) const;

  private:
    struct Message
    {
      Message(const ast::loc& loc, const std::string& kind,
              const std::string& msg, const std::string& prefix);
      void print(runner::Runner& r) const;
      ast::loc loc;
      std::string kind, msg, prefix;
    };
    typedef std::vector<Message> messages_type;
    messages_type messages_;
  };
}

#endif
