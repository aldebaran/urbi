/**
 ** \file object/UVar.cc
 ** \brief Creation of the URBI object UVar.
 */

# include <kernel/userver.hh>

# include <object/uvar.hh>
# include <object/global.hh>
# include <object/list.hh>

# include <runner/call.hh>

# include <runner/runner.hh>


namespace object
{
  static inline void callNotify(runner::Runner& r, rObject self,
                                libport::Symbol notifyList)
  {
    rList l = self->slot_get(notifyList).value().unsafe_cast<List>();
    objects_type args;
    foreach(rObject& co, l->value_get())
      r.apply(self, co, SYMBOL(NOTIFY), args);
  }

  static rObject
  update_bounce(objects_type args)
  {
    //called with self slotname slotval
    check_arg_count(args.size() - 1, 2);
    libport::intrusive_ptr<UVar> rvar =
      args.front()
      ->slot_get(libport::Symbol(args[1]->as<String>()->value_get())).value()
      .unsafe_cast<UVar>();
    rvar->update_(args[2]);
    return void_class;
  }

  UVar::UVar()
    : Primitive( boost::bind(&UVar::accessor, this))
    , looping_(false)
    , inChange_(false)
    , inAccess_(false)
  {
    protos_set(new List);
    proto_add(proto ? proto : Primitive::proto);
  }

  UVar::UVar(libport::intrusive_ptr<UVar>)
    : Primitive( boost::bind(&UVar::accessor, this))
    , looping_(false)
    , inChange_(false)
    , inAccess_(false)
  {
    protos_set(new List);
    proto_add(proto ? proto : Primitive::proto);
  }

  void
  UVar::loopCheck()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    if (!looping_
        && !slot_get(SYMBOL(change))->as<List>()->value_get().empty()
        && !slot_get(SYMBOL(access))->as<List>()->value_get().empty())
    {
      looping_ = true;
      while (true)
      {
        accessor();
        objects_type args;
        rObject period = urbi_call(global_class, SYMBOL(getPeriod), args);
        r.yield_until(libport::utime() +
          static_cast<libport::utime_t>(period->as<Float>()->value_get()
                                       * 1000000.0));
      }
    }
  }

  rObject
  UVar::update_(rObject val)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    slot_update(SYMBOL(val), val);
    if (is_true(slot_get(SYMBOL(owned))))
      callNotify(r, rObject(this), SYMBOL(changeOwned));
     else if (!inChange_)
     {
       inChange_ = true;
       callNotify(r, rObject(this), SYMBOL(change));
       inChange_ = false;
     }
   return val;
  }

  rObject
  UVar::accessor()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    if (this == proto.get())
      return this;
    if (!inAccess_)
    {
      inAccess_ = true;
      callNotify(r, rObject(this), SYMBOL(access));
      inAccess_ = false;
    }
    if (!is_true(slot_get(SYMBOL(owned))))
      return slot_get(SYMBOL(val));
    else
      return slot_get(SYMBOL(valsensor));
  }

  rObject
  UVar::writeOwned(rObject newval)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    slot_update(SYMBOL(valsensor), newval);
    callNotify(r, rObject(this), SYMBOL(change));
    return newval;
  }

  void
  UVar::initialize(CxxObject::Binder<UVar>& bind)
  {
    bind(SYMBOL(writeOwned), &UVar::writeOwned);
    bind(SYMBOL(update_), &UVar::update_);
    bind(SYMBOL(loopCheck), &UVar::loopCheck);
    bind(SYMBOL(accessor), &UVar::accessor);
    proto->slot_set(SYMBOL(update_bounce), new Primitive(&update_bounce));
  }

  rObject
  UVar::proto_make()
  {
    return new UVar();
  }

  URBI_CXX_OBJECT_REGISTER(UVar);
}
