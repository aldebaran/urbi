/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_FINALIZABLE_HH
# define OBJECT_FINALIZABLE_HH

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Finalizable: public CxxObject
    {
      public:
      Finalizable();
      Finalizable(rFinalizable model);
      virtual ~Finalizable();

      void __inc();
      void __dec();
      int __get() const;
      private:
      URBI_CXX_OBJECT(Finalizable, CxxObject);
    };
  }
}

#endif
