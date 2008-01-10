/**
 ** \file ast/flow-exception.hh
 ** \brief Contains flow exceptions.
 */

#ifndef AST_FLOW_EXCEPTION_HH
# define AST_FLOW_EXCEPTION_HH

# include <exception>
# include <ostream>

# include "ast/loc.hh"
# include "object/fwd.hh"

namespace ast
{
  /// The \c FlowException class. Base class for exceptions used to control the
  /// flow of execution, like \c BreakException.
  class FlowException : std::exception
  {
  public:
    explicit FlowException(const loc&);

    const loc& location_get () const;

  protected:
    loc location_;
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
    ReturnException(const loc&, object::rObject);

    ~ReturnException() throw ();
    
    object::rObject	result_get();

  protected:
    object::rObject	result_;
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
