/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/kernel/userver.hh>

#include <urbi/object/cxx-object.hh>
#include <urbi/object/global.hh>
#include <urbi/object/symbols.hh>

#include <urbi/runner/raise.hh>

GD_CATEGORY(Urbi.CxxObject);

namespace urbi
{
  namespace object
  {
    CxxObject::CxxObject()
    {}

    void
    type_check(const rObject& o, const rObject& exp)
    {
      aver(o);
      aver(exp);
      if (!is_a(o, exp))
        runner::raise_type_error(o, exp);
    }

    rObject
    resolve_namespace(std::string& name)
    {
      rObject res = Object::package_lang_get();
      size_t dot;
      while ((dot = name.find(".")) != std::string::npos)
      {
        std::string ns = name.substr(0, dot);
        name = name.substr(dot + 1);
        if (res->hasLocalSlot(ns))
          res = res->getSlotValue(ns);
        else
        {
          ::urbi::object::rObject o = new ::urbi::object::Object;
          o->proto_add(::urbi::object::Object::proto);
          o->setSlotValue(SYMBOL(asString), to_urbi(ns));
          res = res->setSlot(ns, o);
        }
      }
      return res;
    }
  }
}
