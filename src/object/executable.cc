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
    proto_add(object_class);
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
