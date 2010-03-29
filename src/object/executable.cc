/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cassert>

#include <urbi/object/executable.hh>

namespace urbi
{
  namespace object
  {
    rObject Executable::operator()(object::objects_type)
    {
      pabort("Should never be here");
    }

    Executable::Executable()
    {
      proto_add(Object::proto);
    }

    Executable::Executable(rExecutable model)
    {
      proto_add(model);
    }

    void Executable::initialize(CxxObject::Binder<Executable>&)
    {}

    URBI_CXX_OBJECT_REGISTER(Executable)
    {

    }
  }
}
