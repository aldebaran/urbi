#ifndef SCHEDULER_EXCEPTION_HH
# define SCHEDULER_EXCEPTION_HH

# include <string>

# include <boost/optional.hpp>

# include <libport/compiler.hh>
# include <urbi/export.hh>

namespace scheduler
{
  class exception;
  typedef std::auto_ptr<exception> exception_ptr;

  class exception
  {
  public:
    virtual ~exception();
    virtual std::string what() const throw ();
    virtual exception_ptr clone() const = 0;
    ATTRIBUTE_NORETURN void rethrow() const;
  protected:
    ATTRIBUTE_NORETURN virtual void rethrow_() const = 0;
  };

/// \def ADD_FIELD(Type, Name)
/// Define an optional field Name, and accessors.
#define ADD_FIELD(Type, Name)						\
 public:                                                                \
   bool Name ## _is_set() const { return Name ## _; };			\
   const Type& Name ## _get() const { return Name ## _ .get(); };	\
   void Name ## _set(const Type& data) { Name ## _ = data; };		\
 private:								\
   boost::optional<Type> Name ## _;

#define COMPLETE_EXCEPTION(Name)				\
 public:							\
 virtual ::scheduler::exception_ptr clone() const		\
 {								\
   return ::scheduler::exception_ptr(new Name(*this));		\
 };								\
 virtual std::string what() const throw()			\
 {								\
   return #Name;						\
 }								\
private:							\
  ATTRIBUTE_NORETURN virtual void rethrow_() const		\
  {								\
    throw *this;						\
  };

} // namespace scheduler

# include <scheduler/exception.hxx>

#endif // SCHEDULER_EXCEPTION_HH
