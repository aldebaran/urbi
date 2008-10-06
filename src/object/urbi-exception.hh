/**
 ** \file object/urbi-exception.hh
 ** \brief Definition of Exception
 */

#ifndef OBJECT_URBI_EXCEPTION_HH
# define OBJECT_URBI_EXCEPTION_HH

# include <string>

# include <libport/ufloat.hh>
# include <libport/symbol.hh>

# include <ast/fwd.hh>
# include <ast/call.hh>
# include <ast/loc.hh>
# include <kernel/exception.hh>
# include <object/object.hh>

namespace object
{
  /// The call stack
  typedef std::pair<libport::Symbol,
                    boost::optional<ast::loc> > call_type;
  typedef std::vector<call_type> call_stack_type;

  /// Urbi-visible exceptions
  class UrbiException: public kernel::exception
  {
  public:
    UrbiException(rObject value, const call_stack_type& bt);
    rObject value_get();
    const call_stack_type& backtrace_get();

  private:
    rObject value_;
    call_stack_type bt_;
    COMPLETE_EXCEPTION(UrbiException);
  };

} // namespace object

# include <object/urbi-exception.hxx>

#endif //! OBJECT_URBI_EXCEPTION_HH
