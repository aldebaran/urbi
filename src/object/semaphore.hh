/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/semaphore.hh
 ** \brief Definition of the Urbi object semaphore.
 */

#ifndef OBJECT_SEMAPHORE_HH
# define OBJECT_SEMAPHORE_HH

# include <urbi/object/fwd.hh>
# include <sched/fwd.hh>

namespace urbi
{
  namespace object
  {
    class Semaphore: public CxxObject
    {
    public:
      typedef std::pair< int, std::list<sched::rJob> > value_type;

      Semaphore();
      Semaphore(rSemaphore model);
      Semaphore(const value_type& value);

      value_type& value_get();

      static rSemaphore _new(rObject sem, rFloat count);
      void acquire();
      void release();
      rObject criticalSection(rCode);

    private:
      value_type value_;

      URBI_CXX_OBJECT_(Semaphore);
    };

  }; // namespace object
}

#endif // OBJECT_SEMAPHORE_HH
