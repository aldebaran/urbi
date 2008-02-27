#ifndef KERNEL_EXCEPTION_HXX
# define KERNEL_EXCEPTION_HXX

namespace kernel
{

  inline std::string
  exception::what () const throw()
  {
    return "unknown exception";
  };

  inline
  void rethrow (const exception_ptr& e)
  {
    e->rethrow ();
  }

} // namespace kernel

#endif // KERNEL_EXCEPTION_HXX
