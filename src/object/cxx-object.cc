#include <object/cxx-object.hh>

namespace object
{


  CxxObject::CxxObject()
  {

  }

  // Initialization

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
