/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/lexical-cast.hh>
#include <libport/symbol.hh>

#include <urbi/kernel/userver.hh>

#include <object/finalizable.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/urbi-exception.hh>

#include <eval/call.hh>

namespace urbi
{
  namespace object
  {

    Finalizable::Finalizable()
      : CxxObject()
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Finalizable::Finalizable(rFinalizable model)
    {
      proto_add(model);
    }

    URBI_CXX_OBJECT_INIT(Finalizable)
    {
      BIND(__dec);
      BIND(__inc);
      BIND(__get);
    }

    Finalizable::~Finalizable()
    {
      try
      {
        if (rObject finalize = slot_get_value(SYMBOL(finalize), false))
        {
          objects_type args;
          args << this;
          eval::call_apply(::kernel::runner(),
                           finalize,
                           SYMBOL(finalize),
                           args);
        }
      }
      catch (const UrbiException& e)
      {
        // We cannot allow an exception, which might contain a ref to
        // this, to escape. Throw the string representation instead.
        throw UrbiException(
          e.value_get()->call(SYMBOL(asString))->as<object::String>(),
          e.backtrace_get());
      }
    }

    void Finalizable::__dec()
    {
      counter_dec();
    }
    void Finalizable::__inc()
    {
      counter_inc();
    }
    int Finalizable::__get() const
    {
      return counter_get();
    }
  }
}
