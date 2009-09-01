/**
 ** \file object/urbi-exception.hh
 ** \brief Definition of Exception
 */

#ifndef OBJECT_URBI_EXCEPTION_HH
# define OBJECT_URBI_EXCEPTION_HH

# include <string>

# include <libport/ufloat.hh>
# include <libport/symbol.hh>

# include <ast/loc.hh>
# include <object/fwd.hh>
# include <sched/exception.hh>

namespace object
{
  /// One call.
  typedef std::pair<libport::Symbol,
                    boost::optional<ast::loc> > call_type;

  /// Call stack.
  typedef std::vector<call_type> call_stack_type;

  /// Urbi-visible exceptions.
  class UrbiException: public sched::exception
  {
  public:
    UrbiException(rObject value, const call_stack_type& bt);

    /// Dump this on \a o, for debugging.
    virtual std::ostream& dump(std::ostream& o) const;

    ADD_FIELD(rObject, value)
    ADD_FIELD(call_stack_type, backtrace)
    COMPLETE_EXCEPTION(UrbiException)
  };

  std::ostream& operator<<(std::ostream& o, const UrbiException& e);

} // namespace object

// These guys are actually instances of std::, so their operator<<
// need to be there too.

namespace std
{
  ostream& operator<<(ostream& o, const object::call_type& c);
  ostream& operator<<(ostream& o, const object::call_stack_type& c);
}

# include <object/urbi-exception.hxx>

#endif //! OBJECT_URBI_EXCEPTION_HH
