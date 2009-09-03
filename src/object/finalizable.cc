/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <libport/lexical-cast.hh>
#include <libport/symbol.hh>

#include <kernel/userver.hh>

#include <object/finalizable.hh>
#include <object/symbols.hh>

#include <runner/runner.hh>

namespace object
{

  Finalizable::Finalizable()
  : CxxObject()
  {
    proto_add(proto ? proto : Object::proto);
  }

  Finalizable::Finalizable(rFinalizable model)
  {
    proto_add(model);
  }

  Finalizable::~Finalizable()
  {
    // Prevent further destruction of this object.
    counter_inc();
    if (rSlot finalize = slot_locate(SYMBOL(finalize), false).second)
    {
      objects_type args;
      args.push_back(this);
      ::kernel::urbiserver->getCurrentRunner().apply
           (finalize->value(), SYMBOL(finalize), args);
    }
    // We are allready in the destructor, we cannot allow a reference to be
    // kept. This check must be outside the above block.
    if (counter_get() != 1)
    {
      pabort("Object finalizer "+ string_cast((void*)this) + " kept "
             + string_cast(counter_get()-1)
             + " reference(s) to this");
    }
  }
  URBI_CXX_OBJECT_REGISTER(Finalizable);

  void Finalizable::__dec()
  {
    counter_dec();
  }
  void Finalizable::__inc()
  {
    counter_inc();
  }
  int Finalizable::__get()
  {
    return counter_get();
  }
  void Finalizable::initialize(CxxObject::Binder<Finalizable>& bind)
  {
    bind(SYMBOL(__dec),         &Finalizable::__dec);
    bind(SYMBOL(__inc),         &Finalizable::__inc);
    bind(SYMBOL(__get),         &Finalizable::__get);
  }

  rObject
  Finalizable::proto_make()
  {
    return new Finalizable();
  }
}
