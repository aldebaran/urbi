#include <object/global.hh>
#include <object/cxx-object.hh>
#include <runner/call.hh>

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
             runner::Runner& r, const libport::Symbol,
             boost::optional<unsigned> idx)
  {
    assert(o);
    assert(exp);
    if (!is_a(o, exp))
    {
      rObject exn =
        idx
        ? urbi_call(r, global_class->slot_get(SYMBOL(ArgumentTypeError)),
                    SYMBOL(new), new Float(idx.get()), exp, o)
        : urbi_call(r, global_class->slot_get(SYMBOL(TypeError)),
                    SYMBOL(new), exp, o);
      r.raise(exn);
    }
  }

  void check_arg_count (runner::Runner& r,
                        unsigned effective, unsigned formal)
  {
    if (formal != effective)
    {
      rObject exn =
        urbi_call(r, global_class->slot_get(SYMBOL(ArityError)), SYMBOL(new),
                  new Float(effective), new Float(formal));
      r.raise(exn);
    }
  }

  void check_arg_count (runner::Runner& r,
                        unsigned effective, unsigned min, unsigned max)
  {
    if (effective < min || effective > max)
    {
      rObject exn =
        urbi_call(r, global_class->slot_get(SYMBOL(ArityError)), SYMBOL(new),
                  new Float(effective), new Float(min), new Float(max));
      r.raise(exn);
    }
  }

}
