/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/barrier.hh
 ** \brief Definition of the Urbi object barrier.
 */

#ifndef OBJECT_BARRIER_HH
# define OBJECT_BARRIER_HH

# include <urbi/object/cxx-object.hh>
# include <urbi/object/fwd.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Barrier: public CxxObject
    {
    public:
      typedef std::list<sched::rJob> value_type;

      Barrier(rBarrier model);
      Barrier(const value_type& value);

      value_type& value_get();

      static rBarrier _new(rObject);
      rObject wait();
      unsigned int signal(rObject);
      unsigned int signalAll(rObject);

    private:
      value_type value_;

      URBI_CXX_OBJECT(Barrier, CxxObject);
    };

  }; // namespace object
}

#endif // OBJECT_BARRIER_HH
