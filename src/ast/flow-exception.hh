/**
 ** \file ast/flow-exception.hh
 ** \brief Contains flow exceptions.
 */

#ifndef AST_FLOW_EXCEPTION
# define AST_FLOW_EXCEPTION

# include <exception>
# include <ostream>

# include "ast/loc.hh"
# include "object/fwd.hh"

namespace ast
{
  /// The FlowException class. Base class for exceptions used to control the
  /// flow of execution, like BreakException.
  class FlowException : std::exception
  {
  public:
    FlowException(const loc& location);

    const loc& location_get () const;

  protected:
    loc location_;
  };

  /// BreakException, thrown to manage break keyword.
  class BreakException : public FlowException
  {
  public:
    BreakException(const loc& location);
  };

  /// kind of a flow exception
  enum flow_exception_kind
  {
    break_exception,
  };

  /// Report \a e on \a o.
  std::ostream& operator<<(std::ostream& o, flow_exception_kind k);

} // namespace ast

#endif // !AST_FLOW_EXCEPTION
