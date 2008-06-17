#include <object/cxx-object.hh>

namespace object
{


  CxxObject::CxxObject()
  {

  }

  CxxObject::Initializer::Initializer(rObject& tgt)
    : res_(tgt)
  {}

  CxxObject::Initializer::~Initializer()
  {}

  void CxxObject::create()
  {
    foreach (Initializer* init, initializers_get())
      init->create();
  }

  void CxxObject::initialize(rObject global)
  {
    foreach (Initializer* init, initializers_get())
      global->slot_set(init->name(), init->make_class());
  }

  CxxObject::initializers_type& CxxObject::initializers_get()
  {
    static initializers_type res;
    return res;
  }

}
