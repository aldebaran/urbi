/**
 ** \file object/barrier-class.hh
 ** \brief Definition of the URBI object barrier.
 */

#ifndef OBJECT_BARRIER_CLASS_HH
# define OBJECT_BARRIER_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <runner/runner.hh>

namespace object
{
  class Barrier: public CxxObject
  {
  public:
    typedef std::deque<scheduler::rJob> value_type;

    Barrier(rBarrier model);
    Barrier(const value_type& value);

    value_type& value_get();

    static rBarrier _new(rObject);
    rObject wait(runner::Runner&);
    unsigned int signal(rObject);
    unsigned int signalAll(rObject);

  private:
    value_type value_;

  URBI_CXX_OBJECT(Barrier);
  };

}; // namespace object

#endif // OBJECT_BARRIER_CLASS_HH
