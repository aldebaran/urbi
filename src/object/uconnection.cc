/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

# include <urbi/object/event.hh>
# include <urbi/object/symbols.hh>
# include <object/uconnection.hh>
# include <urbi/object/urbi-exception.hh>
# include <object/uvalue.hh>
# include <urbi/object/slot.hh>

# include <runner/job.hh>

# include <urbi/object/lobby.hh>

GD_CATEGORY(Urbi.UConnection);
namespace urbi
{
  namespace object
  {
    UConnection::UConnection()
    {
      proto_add(new UConnection);
      init_();
    }

    UConnection::UConnection(libport::intrusive_ptr<UConnection> model)
    {
      proto_add(model);
      init_();
    }

    URBI_CXX_OBJECT_INIT(UConnection)
    {
      BIND(source);
      BIND(target);
      BIND(instances, instances_get);
      init_();
    }

    void UConnection::init_()
    {
      maxParallelEvents_set(1);
      asynchronous_set(false);
      cb_ = new callback_type(boost::bind(&UConnection::call, this, _1));
    }

    void UConnection::call(const objects_type&)
    {
      // FIXME: stay connected in the hope of reconnecting one day?
      rSlot t = target->as<Slot>();
      if (!t || t == Slot::proto)
        disconnected_set(true);
      if (t->call(SYMBOL(dead)) == true_class)
      {
        //FIXME: try to keep informations for possible future reconnection
        GD_INFO_TRACE("Target is dead");
        target = Slot::proto;
        disconnected_set(true);
      }
      GD_FINFO_DUMP("%s -> %s call, enabled: %s",
                    source, target, enabled_get());
      rSlot s = source->as<Slot>();
      if (!s)
        return;
      rObject val = s->value(nil_class, true);
      target->as<Slot>()->set(val, nil_class, libport::utime());
    }
  }
}
