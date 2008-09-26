#ifndef KERNEL_EXCEPTION_HXX
# define KERNEL_EXCEPTION_HXX

# include <cstdlib>

namespace kernel
{

  inline
  exception::~exception ()
  {
  }

  inline std::string
  exception::what () const throw()
  {
    return "unknown exception";
  }

  inline
  void rethrow (const exception_ptr& e)
  {
    e->rethrow ();
    // GCC cannot guarantee that an overriden virtual method
    // will not return, so help it here.
    abort();
  }

} // namespace kernel

#endif // KERNEL_EXCEPTION_HXX
