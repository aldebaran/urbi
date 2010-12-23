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

# include <libport/debug.hh>

# include <kernel/uconnection.hh>
# include <kernel/uobject.hh>
# include <kernel/userver.hh>
# include <kernel/uvalue-cast.hh>

# include <object/symbols.hh>
# include <object/uconnection.hh>
# include <object/urbi-exception.hh>
# include <object/uvalue.hh>
# include <object/uvar.hh>

# include <urbi/object/event.hh>
# include <urbi/object/global.hh>
# include <urbi/object/list.hh>
# include <urbi/object/lobby.hh>

# include <runner/runner.hh>
# include <runner/interpreter.hh>


GD_CATEGORY(UVar);
namespace urbi
{
  namespace object
  {
    static inline void
    show_exception_message(runner::Runner& r, rUVar self,
                           const char* m1, const char* m2 = "")
    {
      std::string msg =
        libport::format("!!! %s %s.%s%s",
                        m1,
                        (self->slot_get(SYMBOL(ownerName))
                         ->as<String>()->value_get()),
                        self->initialName,
                        m2);

      r.lobby_get()->call(SYMBOL(send),
                          new String(msg),
                          new String("error"));
    }

    inline void
    callNotify(runner::Runner& r, rUVar self,
               libport::Symbol notifyList, rObject sourceUVar = 0)
    {
      rList l =
        self->slot_get(notifyList)->slot_get(SYMBOL(values))->as<List>();
      objects_type args;
      args.push_back(self);
      if (sourceUVar)
        args.push_back(sourceUVar);
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

    static inline void
    callConnections(runner::Runner& r, rObject self, libport::Symbol notifyList)
    {
      rList l = self->slot_get(notifyList)->as<List>();
      // We must copy the list as callbacks might remove themselve
      List::value_type callbacks = l->value_get();
      for (List::value_type::iterator i = callbacks.begin();
           i != callbacks.end(); )
      {
        rUConnection c = (*i)->as<UConnection>();
        if (c->call(r, self))
          ++i;
        else
          i = callbacks.erase(i);
      }
    }

    rObject UVar::fromName(const std::string& name)
    {
      try
      {
        ::urbi::uobjects::StringPair p = ::urbi::uobjects::split_name(name);
        rObject o = uobjects::get_base(p.first);
        if (!o)
          return o;
        else
          return o->slot_get(libport::Symbol(p.second));
      }
      catch(UrbiException& e)
      {
        return 0;
      }
      catch(sched::exception& e)
      {
        throw;
      }
      return 0;
    }

    UVar::UVar()
      : Primitive(boost::function1<rObject, objects_type>(boost::bind(&UVar::accessor, this, _1)))
      , looping_(false)
      , inAccess_(false)
      , waiterCount_(0)
      , owned(false)
    {
      protos_set(new List);
      proto_add(proto ? rPrimitive(proto) : Primitive::proto);
      slot_set(SYMBOL(waiterTag), new Tag());
    }

    UVar::UVar(libport::intrusive_ptr<UVar>)
      : Primitive(boost::function1<rObject, const objects_type&>(boost::bind(&UVar::accessor, this, _1)))
      , looping_(false)
      , inAccess_(false)
      , waiterCount_(0)
      , owned(false)
    {
      protos_set(new List);
      proto_add(proto ? rPrimitive(proto) : Primitive::proto);
      slot_set(SYMBOL(waiterTag), new Tag());
    }

    static rObject
    uvar_update_bounce(objects_type args)
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

    URBI_CXX_OBJECT_INIT(UVar)
      : Primitive(boost::function1<rObject, objects_type>
                    (boost::bind(&UVar::accessor, this, _1)))
      , looping_(false)
      , inAccess_(false)
    {
#define DECLARE(Name, Cxx)           \
      bind(SYMBOL(Name), &UVar::Cxx)

      DECLARE(writeOwned,    writeOwned);
      DECLARE(update_timed,  update_timed);
      DECLARE(loopCheck,     loopCheck);
      DECLARE(accessor,      accessor);
      DECLARE(update_,       update_);
      DECLARE(update_timed_, update_timed_);
      DECLARE(owned,         owned);
      DECLARE(initialName,   initialName);
      DECLARE(changed,   changed);
#undef DECLARE

      setSlot(SYMBOL(updateBounce), new Primitive(&uvar_update_bounce));
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
        callNotify(r, rUVar(this), SYMBOL(accessInLoop));
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
      bool loopChecker = !looping_
          &&
          (!slot_get(SYMBOL(change))->call(SYMBOL(empty))->as_bool()
          || (changed_ && changed_->hasSubscribers())
          ||!slot_get(SYMBOL(changeConnections))->call(SYMBOL(empty))->as_bool()
           )
          && !slot_get(SYMBOL(access))->call(SYMBOL(empty))->as_bool();
      GD_FINFO_TRACE("Loopcheck on %s: %s", initialName, loopChecker);
      if (loopChecker)
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
        val = slot_get(owned
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
      getSlot(SYMBOL(owner))->setProperty(initialName,
                                          SYMBOL(timestamp), to_urbi(double(timestamp) / 1000000));
      // Do not bother with UValue for numeric types.
      if (rUValue uval = val->as<UValue>())
      {
        if (uval->value_get().type == urbi::DATA_DOUBLE)
          val = uval->extract();
      }
      else
        val = val->call(SYMBOL(uvalueDeserialize));
      runner::Runner& r = ::kernel::runner();
      slot_update(SYMBOL(val), val);
      if (owned)
        callNotify(r, rUVar(this), SYMBOL(changeOwned));
      else
      {
        checkBypassCopy();
        std::set<void*>::iterator i = inChange_.find(&r);
        GD_FINFO_DUMP("update calling notify if %s", i == inChange_.end());
        if (i == inChange_.end())
        {
          i = inChange_.insert(&r).first;
          FINALLY(((std::set<void*>&, inChange_)) ((std::set<void*>::iterator&, i)), inChange_.erase(i));
          callNotify(r, rUVar(this), SYMBOL(change));
          callConnections(r, rObject(this), SYMBOL(changeConnections));
        }
        if (changed_)
          changed_->emit();
      }
      return val;
    }

    rObject
    UVar::changed()
    {
      GD_FINFO_DEBUG("Creating changed! for var %s", initialName);
      if (!changed_)
      {
        changed_ = new Event;
        slot_update(SYMBOL(changed), changed_);
      }
      return changed_;
    }

    rObject
    UVar::accessor(const objects_type&)
    {
      runner::Runner* r = ::kernel::urbiserver->getCurrentRunnerOpt();
      bool dl = r->dependencies_log_get();
      r->dependencies_log_set(false);
      FINALLY(((runner::Runner*, r))((bool, dl)),
              r->dependencies_log_set(dl));
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
        callNotify(r, rUVar(this), SYMBOL(access));
        inAccess_ = false;
      }
      rObject res = slot_get(owned
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
            res = slot_get(owned
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
      callNotify(r, rUVar(this), SYMBOL(change));
      callConnections(r, rObject(this), SYMBOL(changeConnections));
      if (changed_)
        changed_->emit();
      return newval;
    }
  }
}
