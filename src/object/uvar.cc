/**
 ** \file object/UVar.cc
 ** \brief Creation of the URBI object UVar.
 */

# include <kernel/userver.hh>
# include <kernel/uvalue-cast.hh>

# include <object/global.hh>
# include <object/list.hh>
# include <object/symbols.hh>
# include <object/urbi-exception.hh>
# include <object/uvar.hh>

# include <runner/runner.hh>


namespace object
{
  static inline void callNotify(runner::Runner& r, rObject self,
                                libport::Symbol notifyList)
  {
    rList l =
      self->slot_get(notifyList)->slot_get(SYMBOL(values))->as<List>();
    objects_type args;
    args.push_back(self);
    List::value_type& callbacks = l->value_get();
    for (List::value_type::iterator i = callbacks.begin();
         i != callbacks.end(); )
    {
      try
      {
         r.apply(*i, SYMBOL(NOTIFY), args);
         ++i;
      }
      catch(UrbiException& e)
      {
        std::cerr << "Urbi Exception caught while processing notify: "
          << *e.value_get() << std::endl;
        std::cerr << "backtrace: " << std::endl;
        rforeach (call_type c, e.backtrace_get())
        {
          std::ostringstream o;
          std::cerr << "    called from: ";
          if (c.second)
            std::cerr << *c.second << ": ";
          std::cerr << c.first << std::endl;
        }
        i = callbacks.erase(i);
      }
      catch(sched::exception& e)
      {
        throw;
      }
      catch(...)
      {
        std::cerr << "Unknown exception caught while processing notify."
          << std::endl;
        i = callbacks.erase(i);
      }
    }
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
    , inAccess_(false)
  {
    protos_set(new List);
    proto_add(proto ? proto : Primitive::proto);
  }

  UVar::UVar(libport::intrusive_ptr<UVar>)
    : Primitive( boost::bind(&UVar::accessor, this))
    , looping_(false)
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
        && !slot_get(SYMBOL(change))->call(SYMBOL(empty))->as_bool()
        && !slot_get(SYMBOL(access))->call(SYMBOL(empty))->as_bool())
    {
      looping_ = true;
      while (true)
      {
        accessor();
        objects_type args;
        args.push_back(global_class);
        rObject period = args[0]->call(SYMBOL(getPeriod), args);
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
    if (slot_get(SYMBOL(owned))->as_bool())
      callNotify(r, rObject(this), SYMBOL(changeOwned));
    else
    {
      std::set<void*>::iterator i = inChange_.find(&r);
      if (i == inChange_.end())
      {
	// callNotify does not throw, this is safe.
	i = inChange_.insert(&r).first;
	callNotify(r, rObject(this), SYMBOL(change));
	inChange_.erase(i);
      }
    }
    return val;
  }

  rObject
  UVar::accessor()
  {
    return getter(false);
  }

  rObject
  UVar::getter(bool fromCXX)
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
    rObject res = slot_get(slot_get(SYMBOL(owned))->as_bool()
                           ? SYMBOL(valsensor)
                           : SYMBOL(val));
    if (!fromCXX)
      if (object::rUValue bv = res->as<object::UValue>())
        return bv->extract();
    return res;
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

  UValue::UValue()
  {
    protos_set(new List);
    proto_add(proto ? proto : CxxObject::proto);
  }
  UValue::UValue(libport::intrusive_ptr<UValue>)
  {
    protos_set(new List);
    proto_add(proto ? proto : CxxObject::proto);
  }
  UValue::UValue(const urbi::UValue& v, bool bypass)
  {
    protos_set(new List);
    proto_add(proto ? proto : CxxObject::proto);
    put(v, bypass);
  }
  UValue::~UValue()
  {
  }
  rObject
  UValue::extract()
  {
    if (cache_)
      return cache_;
    if (value_.type == urbi::DATA_VOID)
      return nil_class;
    if (!cache_)
      cache_ = object_cast(value_);
    return cache_;
  }
  const urbi::UValue&
  UValue::value_get()
  {
    static urbi::UValue dummy;
    if (value_.type != urbi::DATA_VOID)
      return value_;
    if (!cache_ || cache_ == nil_class)
      return dummy;
    value_ = uvalue_cast(cache_);
    alocated_ = true;
    return value_;
  }
  void
  UValue::invalidate()
  {
    if (!alocated_)
      value_ = urbi::UValue();
  }
  void
  UValue::put(const urbi::UValue& v,  bool bypass)
  {
    alocated_ = !bypass;
    value_.set(v, !bypass);
    cache_ = 0;
  }
  void
  UValue::put(rObject r)
  {
    value_ = urbi::UValue();
    cache_ = r;
  }
  rObject
  UValue::proto_make()
  {
    return new UValue();
  }
  void
  UValue::initialize(CxxObject::Binder<UValue>& bind)
  {
    bind(SYMBOL(extract), &UValue::extract);
    bind(SYMBOL(invalidate), &UValue::invalidate);
    bind(SYMBOL(put), (void (UValue::*)(rObject))&UValue::put);
  }
  URBI_CXX_OBJECT_REGISTER(UValue);
}
