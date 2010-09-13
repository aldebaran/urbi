/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/uvar.cc
 ** \brief Creation of the Urbi object UVar.
 */

# include <kernel/uconnection.hh>
# include <kernel/userver.hh>
# include <kernel/uvalue-cast.hh>

# include <object/symbols.hh>
# include <object/urbi-exception.hh>
# include <object/uvalue.hh>
# include <object/uvar.hh>

# include <urbi/object/global.hh>
# include <urbi/object/list.hh>
# include <urbi/object/lobby.hh>

# include <runner/runner.hh>
# include <runner/interpreter.hh>


namespace urbi
{
  namespace object
  {
    static inline void
    show_exception_message(runner::Runner& r, rObject self,
                           const char* m1, const char* m2 = "")
    {
      std::string msg =
        libport::format("!!! %s %s.%s%s",
                        m1,
                        (self->slot_get(SYMBOL(ownerName))
                         ->as<String>()->value_get()),
                        (self->slot_get(SYMBOL(initialName))
                         ->as<String>()->value_get()),
                        m2);

      r.lobby_get()->call(SYMBOL(send),
                          new String(msg),
                          new String("error"));
    }

    static inline void
    callNotify(runner::Runner& r, rObject self,
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
        bool failed = true;
        try
        {
          r.apply(*i, SYMBOL(NOTIFY), args);
          failed = false;
        }
        catch(UrbiException& e)
        {
          show_exception_message
            (r, self,
             "Exception caught while processing notify on", ":");
          runner::Interpreter& in = dynamic_cast<runner::Interpreter&>(r);
          in.show_exception(e);

        }
        catch(sched::exception& e)
        {
          throw;
        }
        catch(...)
        {
          show_exception_message
            (r, self,
             "Unknown exception called while processing notify on");
        }
        if (failed && self->slot_get(SYMBOL(eraseThrowingCallbacks))->as_bool())
        {
          // 'value' is just a cache, not the actual storage location.
          self->slot_get(notifyList)->call(SYMBOL(remove), *i);
          i = callbacks.erase(i);
        }
        else
          ++i;
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
      if (!rvar)
        RAISE("UVar updatehook called on non-uvar slot");
      rvar->update_(args[2]);
      return void_class;
    }

    UVar::UVar()
      : Primitive( boost::bind(&UVar::accessor, this))
      , looping_(false)
      , inAccess_(false)
      , waiterCount_(0)
    {
      protos_set(new List);
      proto_add(proto ? rPrimitive(proto) : Primitive::proto);
      slot_set(SYMBOL(waiterTag), new Tag());
    }

    UVar::UVar(libport::intrusive_ptr<UVar>)
      : Primitive( boost::bind(&UVar::accessor, this))
      , looping_(false)
      , inAccess_(false)
      , waiterCount_(0)
    {
      protos_set(new List);
      proto_add(proto ? rPrimitive(proto) : Primitive::proto);
      slot_set(SYMBOL(waiterTag), new Tag());
    }

    void
    UVar::changeAccessLoop()
    {
      // Prepare a call to System.period.  Keep its computation in the
      // loop, so that we can change it at run time.
      CAPTURE_GLOBAL(System);
      runner::Runner& r = ::kernel::runner();
      while (true)
      {
        callNotify(r, rObject(this), SYMBOL(accessInLoop));
        rObject period = System->call(SYMBOL(period));
        r.yield_for(libport::utime_t(period->as<Float>()->value_get()
                                     * 1000000.0));
      }
    }

    void
    UVar::loopCheck()
    {
      runner::Runner& r = ::kernel::runner();
      bool prevState = r.non_interruptible_get();
      FINALLY(((runner::Runner&, r))((bool, prevState)),
        r.non_interruptible_set(prevState));
      r.non_interruptible_set(true);
      // Loop if we have both notifychange and notifyaccess callbacs.
      // Listeners on the changed event counts as notifychange
      if (!looping_
          &&
          (!slot_get(SYMBOL(change))->call(SYMBOL(empty))->as_bool()
           || slot_get(SYMBOL(changed))->call(SYMBOL(hasSubscribers))->as_bool()
           )
          && !slot_get(SYMBOL(access))->call(SYMBOL(empty))->as_bool())
      {
        // There is no need to keep an accessor present if we are
        // going to trigger it periodicaly.
        slot_update(SYMBOL(accessInLoop),
                    slot_get(SYMBOL(access)));
        slot_update(SYMBOL(access),
                    slot_get(SYMBOL(WeakDictionary))->call(SYMBOL(new)));
        looping_ = true;
	runner::Interpreter* nr =
        new runner::Interpreter(
                   ::kernel::urbiserver->ghost_connection_get().lobby_get(),
                   r.scheduler_get(),
                   boost::bind(&UVar::changeAccessLoop, this),
                   this, SYMBOL(changeAccessLoop));
	nr->tag_stack_clear();
	nr->start_job();
       }
    }

    void
    UVar::checkBypassCopy()
    {
      // If there are blocked reads, call extract to force caching of the
      // temporary value, and unblock them.
      if (waiterCount_)
      {
        /* Split val declaration and assignment to work around g++ 4.3.3 wich warns:
         * intrusive-ptr.hxx:89: error: 'val.libport::intrusive_ptr<urbi::object::UValue>::pointee_'
         *                              may be used uninitialized in this function.
         */
        rUValue val;
        val = slot_get(slot_get(SYMBOL(owned))->as_bool()
                       ? SYMBOL(valsensor)
                       : SYMBOL(val))->as<UValue>();
        if (val)
          val->extract();
        slot_get(SYMBOL(waiterTag))->call(SYMBOL(stop));
      }
    }

    rObject
    UVar::update_(rObject val)
    {
      return update_timed_(val, libport::utime());
    }

    rObject
    UVar::update_timed(rObject val, libport::utime_t timestamp)
    {
      rObject r =  kernel::urbiserver->getCurrentRunner().as_job();
      if (!r->slot_has(SYMBOL(DOLLAR_uobjectInUpdate)))
        r->slot_set(SYMBOL(DOLLAR_uobjectInUpdate), true_class);
      update_timed_(val, timestamp);
      r->slot_remove(SYMBOL(DOLLAR_uobjectInUpdate));
      return void_class;
    }

    rObject
    UVar::update_timed_(rObject val, libport::utime_t timestamp)
    {
      getSlot(SYMBOL(owner))->setProperty(from_urbi<std::string>(getSlot(SYMBOL(initialName))),
                                          SYMBOL(timestamp), to_urbi(double(timestamp) / 1000000));
      // Do not bother with UValue for numeric types.
      if (rUValue uval = val->as<UValue>())
        if (uval->value_get().type == urbi::DATA_DOUBLE)
          val = uval->extract();
      runner::Runner& r = ::kernel::runner();
      slot_update(SYMBOL(val), val);
      if (slot_get(SYMBOL(owned))->as_bool())
        callNotify(r, rObject(this), SYMBOL(changeOwned));
      else
      {
        checkBypassCopy();
        std::set<void*>::iterator i = inChange_.find(&r);
        if (i == inChange_.end())
        {
          // callNotify does not throw, this is safe.
          i = inChange_.insert(&r).first;
          callNotify(r, rObject(this), SYMBOL(change));
          inChange_.erase(i);
        }
        slot_get(SYMBOL(changed))->call("emit");
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
      runner::Runner& r = ::kernel::runner();

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
      {
        if (rUValue bv = res->as<UValue>())
        {
          if (bv->bypassMode_ && bv->extract() == nil_class)
          {
            // This is a read on a bypass-mode UVar, from outside any
            // notifychange: the value is not available.
            // So we mark that we wait by inc-ing waiterCount, and wait
            // on waiterTag until we timeout, or someone writes to the UVar
            // and unlock us.

            // free the shared ptrs
            res.reset();
            bv.reset();
            ++waiterCount_;
            slot_get(SYMBOL(waiterTag))->call(SYMBOL(waitUntilStopped),
                                              new Float(0.5));
            --waiterCount_;
            // The val slot likely changed, fetch it again.
            res = slot_get(slot_get(SYMBOL(owned))->as_bool()
                           ? SYMBOL(valsensor)
                           : SYMBOL(val));
            if (rUValue bv = res->as<UValue>())
              return bv->extract();
            else
              return res;
          }
          return bv->extract();
        }
      }
      return res;
    }

    rObject
    UVar::writeOwned(rObject newval)
    {
      runner::Runner& r = ::kernel::runner();
      slot_update(SYMBOL(valsensor), newval);
      checkBypassCopy();
      callNotify(r, rObject(this), SYMBOL(change));
      slot_get(SYMBOL(changed))->call("emit");
      return newval;
    }

    void
    UVar::initialize(CxxObject::Binder<UVar>& bind)
    {
      bind(SYMBOL(writeOwned), &UVar::writeOwned);
      bind(SYMBOL(update_), &UVar::update_);
      bind(SYMBOL(update_timed_), &UVar::update_timed_);
      bind(SYMBOL(update_timed), &UVar::update_timed);
      bind(SYMBOL(loopCheck), &UVar::loopCheck);
      bind(SYMBOL(accessor), &UVar::accessor);
      proto->slot_set(SYMBOL(updateBounce), new Primitive(&update_bounce));
    }

    URBI_CXX_OBJECT_REGISTER(UVar)
      : Primitive( boost::bind(&UVar::accessor, this))
      , looping_(false)
      , inAccess_(false)
    {}

  }
}
