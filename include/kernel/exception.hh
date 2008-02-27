#ifndef KERNEL_EXCEPTION_HH
# define KERNEL_EXCEPTION_HH

# include <string>

# include <boost/optional.hpp>

# include <libport/shared-ptr.hh>

namespace kernel
{
  class exception
  {
  public:
    virtual std::string what () const throw ();
    virtual exception* clone () const = 0;
    virtual void rethrow () const = 0;
  };

  typedef libport::shared_ptr<exception> exception_ptr;

  void rethrow (const exception_ptr&);

#define ADD_FIELD(Type, Name)						\
  public:								\
  bool Name ## _is_set () const { return Name ## _; };			\
  const Type& Name ## _get () const { return Name ## _ .get (); };	\
  void Name ## _set (const Type& data) { Name ## _ = data; };		\
private:								\
 boost::optional<Type> Name ## _;

#define COMPLETE_EXCEPTION(Name)		\
  virtual exception* clone () const		\
  {						\
    return new Name (*this);			\
  };						\
  virtual void rethrow () const			\
  {						\
    throw *this;				\
  };

} // namespace kernel

# include "kernel/exception.hxx"

#endif // KERNEL_EXCEPTION_HH
