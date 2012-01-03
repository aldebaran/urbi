/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
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

# include <urbi/object/cxx-object.hh>
# include <urbi/object/fwd.hh>
# include <runner/runner.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Semaphore: public CxxObject
    {
    public:
      typedef std::pair< int, std::list<runner::rRunner> > value_type;

      Semaphore();
      Semaphore(rSemaphore model);
      Semaphore(const value_type& value);

      value_type& value_get();

      static rSemaphore _new(rObject sem, rFloat count);
      void acquire();
      void release();
      void release_and_forward(bool forward);
      rObject criticalSection(rCode);

    private:
      value_type value_;

      URBI_CXX_OBJECT(Semaphore, CxxObject);
    };

  }; // namespace object
}

#endif // OBJECT_SEMAPHORE_HH
