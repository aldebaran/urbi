/**
 ** \file ast/flow-exception.hh
 ** \brief Contains flow exceptions.
 */

#ifndef AST_FLOW_EXCEPTION_HH
# define AST_FLOW_EXCEPTION_HH

# include <ostream>

# include "ast/loc.hh"
# include "kernel/exception.hh"
# include "object/fwd.hh"
# include "object/object.hh"

namespace ast
{
  /// The \c FlowException class. Base class for exceptions used to control the
  /// flow of execution, like \c BreakException.
  class FlowException : public kernel::exception
  {
  public:
    explicit FlowException(const loc&);

    ADD_FIELD (ast::loc, location)
    COMPLETE_EXCEPTION (FlowException)
  };

  /// \c BreakException, thrown to manage break keyword.
  class BreakException : public FlowException
  {
  public:
    explicit BreakException(const loc&);
    COMPLETE_EXCEPTION (BreakException)
  };

  /// \c ReturnException, thrown to manage return keyword.
  class ReturnException : public FlowException
  {
  public:
    explicit ReturnException(const loc&, object::rObject);
    ADD_FIELD (object::rObject, result)
    COMPLETE_EXCEPTION (ReturnException)
  };

  /// Kind of a flow exception.
  enum flow_exception_kind
  {
    break_exception,
    return_exception,
  };

  /// Report \a e on \a o.
  std::ostream& operator<<(std::ostream& o, flow_exception_kind k);

} // namespace ast

#endif // !AST_FLOW_EXCEPTION_HH
