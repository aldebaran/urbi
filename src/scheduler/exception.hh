#ifndef SCHEDULER_EXCEPTION_HH
# define SCHEDULER_EXCEPTION_HH

# include <string>

# include <boost/optional.hpp>

# include <libport/compiler.hh>
# include <libport/shared-ptr.hh>
# include <urbi/export.hh>

namespace scheduler
{
  class exception
  {
  public:
    virtual ~exception ();
    virtual std::string what () const throw ();
    virtual exception* clone () const = 0;
    ATTRIBUTE_NORETURN virtual void rethrow () const = 0;
  };

  typedef libport::shared_ptr<exception, false> exception_ptr;

  ATTRIBUTE_NORETURN void rethrow (const exception_ptr&);

/// \def ADD_FIELD(Type, Name)
/// Define an optional field Name, and accessors.
#define ADD_FIELD(Type, Name)						\
 public:                                                                \
   bool Name ## _is_set () const { return Name ## _; };			\
   const Type& Name ## _get () const { return Name ## _ .get (); };	\
   void Name ## _set (const Type& data) { Name ## _ = data; };		\
 private:								\
   boost::optional<Type> Name ## _;

#define COMPLETE_EXCEPTION(Name)				\
  virtual Name* clone () const					\
  {								\
    return new Name (*this);					\
  };								\
  ATTRIBUTE_NORETURN virtual void rethrow () const		\
  {								\
    throw *this;						\
  };

} // namespace scheduler

# include <scheduler/exception.hxx>

#endif // SCHEDULER_EXCEPTION_HH
