/**
 ** \file object/semaphore-class.hh
 ** \brief Definition of the URBI object semaphore.
 */

#ifndef OBJECT_SEMAPHORE_CLASS_HH
# define OBJECT_SEMAPHORE_CLASS_HH

# include <object/fwd.hh>
# include <scheduler/fwd.hh>

namespace object
{
  class Semaphore: public CxxObject
  {
  public:
    typedef std::pair< int, std::deque<scheduler::rJob> > value_type;

    Semaphore();
    Semaphore(rSemaphore model);
    Semaphore(const value_type& value);

    value_type& value_get();

    static rSemaphore _new(rObject sem, rFloat count);
    void p(runner::Runner& r);
    void v();
    rObject criticalSection(runner::Runner&, rCode);

  private:
    value_type value_;

  URBI_CXX_OBJECT(Semaphore);
  };

}; // namespace object

#endif // OBJECT_SEMAPHORE_CLASS_HH
