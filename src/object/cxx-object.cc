#include <kernel/userver.hh>

#include <object/global.hh>
#include <object/cxx-object.hh>

#include <runner/call.hh>
#include <runner/runner.hh>

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

  void
  CxxObject::create()
  {
    foreach (Initializer* init, initializers_get())
      init->create();
  }

  void
  CxxObject::initialize(rObject global)
  {
    foreach (Initializer* init, initializers_get())
      global->slot_set(init->name(), init->make_class());
  }

  CxxObject::initializers_type&
  CxxObject::initializers_get()
  {
    static initializers_type res;
    return res;
  }

  void
  CxxObject::cleanup()
  {
    foreach (Initializer* init, initializers_get())
      delete init;
  }

  void
  type_check(const rObject& o, const rObject& exp,
             boost::optional<unsigned> idx)
  {
    runner::Runner& r = kernel::urbiserver->getCurrentRunner();

    assert(o);
    assert(exp);
    if (!is_a(o, exp))
    {
      if (idx)
        runner::raise_argument_type_error(idx.get(), o, exp);
      else
      {
	CAPTURE_GLOBAL(TypeError);
        r.raise(urbi_call(TypeError, SYMBOL(new), exp, o));
      }
    }
  }

}
