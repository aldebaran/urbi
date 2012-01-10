/*
 * Copyright (C) 2007-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file urbi/object/urbi-exception.hh
 ** \brief Definition of UrbiException.
 */

// FIXME: Not an Urbi object, so the presence in here is dubious.

#ifndef URBI_OBJECT_EXCEPTION_HH
# define URBI_OBJECT_EXCEPTION_HH

# include <string>

# include <libport/ufloat.hh>
# include <libport/symbol.hh>

# include <urbi/object/fwd.hh>
# include <urbi/parser/location.hh>
# include <sched/exception.hh>

namespace urbi
{
  namespace object
  {
    /// One call.
    typedef std::pair<libport::Symbol,
                      boost::optional< ::yy::location> > call_type;

    /// Call stack.
    typedef std::vector<call_type> call_stack_type;

    /// Urbi-visible exceptions.
    class UrbiException: public sched::exception
    {
    public:
      UrbiException(rObject value, const call_stack_type& bt);
      ~UrbiException() throw() {}
      /// Dump this on \a o, for debugging.
      virtual std::ostream& dump(std::ostream& o) const;
      virtual const char* what() const throw();
      const rObject& value() const;
      ADD_FIELD(rObject, value)
      ADD_FIELD(call_stack_type, backtrace)
      PARTIAL_COMPLETE_EXCEPTION(UrbiException)
    };

    std::ostream& operator<<(std::ostream& o, const UrbiException& e);

  } // namespace object
}

// These guys are actually instances of std::, so their operator<<
// need to be there too.

namespace std
{
  ostream& operator<<(ostream& o, const urbi::object::call_type& c);
  ostream& operator<<(ostream& o, const urbi::object::call_stack_type& c);
}

# include <urbi/object/urbi-exception.hxx>

#endif //! URBI_OBJECT_EXCEPTION_HH
