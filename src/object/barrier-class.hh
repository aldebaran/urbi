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
  extern rObject barrier_class;

  class Barrier: public CxxObject
  {
  public:
    typedef std::deque<scheduler::rJob> value_type;

    Barrier();
    Barrier(rBarrier model);
    Barrier(const value_type& value);

    value_type& value_get();

    static rBarrier _new(rObject);
    rObject wait(runner::Runner&);
    rFloat signal(rObject);
    rFloat signalAll(rObject);

    static const std::string type_name;
    virtual std::string type_name_get() const;

  private:
    value_type value_;

  public:
    static void initialize(CxxObject::Binder<Barrier>& binder);
    static bool barrier_added;
  };

}; // namespace object

#endif // OBJECT_BARRIER_CLASS_HH
