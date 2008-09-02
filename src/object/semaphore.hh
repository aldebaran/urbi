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
  extern rObject semaphore_class;

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

    static const std::string type_name;
    virtual std::string type_name_get() const;

  private:
    value_type value_;

  public:
    static void initialize(CxxObject::Binder<Semaphore>& binder);
    static bool semaphore_added;
  };

}; // namespace object

#endif // OBJECT_SEMAPHORE_CLASS_HH
