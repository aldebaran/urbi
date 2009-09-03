/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <libport/assert.hh>

#include <object/executable.hh>

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

  rObject Executable::proto_make()
  {
    return new Executable();
  }

  void Executable::initialize(CxxObject::Binder<Executable>&)
  {}

  URBI_CXX_OBJECT_REGISTER(Executable);
}
