/**
 ** \file ast/flow-exception.hh
 ** \brief Contains flow exceptions.
 */

#ifndef AST_FLOW_EXCEPTION_HH
# define AST_FLOW_EXCEPTION_HH

# include <boost/exception.hpp>
# include <ostream>

# include "ast/loc.hh"
# include "object/fwd.hh"

namespace ast
{
  /// The \c FlowException class. Base class for exceptions used to control the
  /// flow of execution, like \c BreakException.
  class FlowException : public boost::exception
  {
  public:
    explicit FlowException(const loc&);
    loc location_get () const;
  };

  /// \c BreakException, thrown to manage break keyword.
  class BreakException : public FlowException
  {
  public:
    explicit BreakException(const loc&);
  };

  /// \c ReturnException, thrown to manage return keyword.
  class ReturnException : public FlowException
  {
  public:
    explicit ReturnException(const loc&, object::rObject);
    object::rObject result_get() const;
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
