/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
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

# include <urbi/kernel/uconnection.hh>
# include <kernel/uobject.hh>
# include <urbi/kernel/userver.hh>
# include <kernel/uvalue-cast.hh>

# include <urbi/object/symbols.hh>
# include <object/uconnection.hh>
# include <urbi/object/urbi-exception.hh>
# include <object/uvalue.hh>
# include <object/uvar.hh>

# include <urbi/object/event.hh>
# include <urbi/object/global.hh>
# include <urbi/object/list.hh>
# include <urbi/object/lobby.hh>

# include <runner/runner.hh>
# include <runner/interpreter.hh>


GD_CATEGORY(Urbi.UVar);
namespace urbi
{
  namespace object
  {
    using libport::Symbol;
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
               UVar::Callbacks& callbacks_, rObject sourceUVar = 0)
    {
      if (callbacks_.empty())
        return;
      // List might be modified while iterating, so make a copy.
      UVar::Callbacks callbacks(callbacks_);
      objects_type args;
      args.push_back(self);
      if (sourceUVar)
        args.push_back(sourceUVar);
      for (UVar::Callbacks::iterator i = callbacks.begin();
           i != callbacks.end(); ++i)
      {
        bool failed = true;
        GD_FINFO_DUMP("Calling notify on %s with %s args  ", i->second,
                      args.size());
        try
        {
          r.apply(i->second, SYMBOL(NOTIFY), args);
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
          self->removeCallback(callbacks_, i->first);
        }
      }
    }

    void
    callConnections(runner::Runner& r, rObject self, rList l)
    {

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

    unsigned int UVar::uid_ = 0;

    static inline bool matchCachedProps(Symbol s)
    {
      return
         s == SYMBOL(rangemin)
      || s == SYMBOL(rangemax)
      || s == SYMBOL(timestamp)
      || s == SYMBOL(changed);
    }

    static rObject parentGetProperty(const objects_type& o)
    {
      if (o.size() != 3)
        runner::raise_arity_error(o.size()-1, 2);
      Object* self = o[0];
      Symbol vname = Symbol(o[1]->as_checked<String>()->value_get());
      Symbol pname = Symbol(o[2]->as_checked<String>()->value_get());
      rUVar v = self->slot_get(vname)->as<UVar>();
      if (v && matchCachedProps(pname))
      {
        return v->call(pname);
      }
      else
        return self->property_get(vname, pname);
    };
    static rObject parentHasProperty(const objects_type& o)
    {
      if (o.size() != 3)
        runner::raise_arity_error(o.size()-1, 2);
      Object* self = o[0];
      Symbol vname = Symbol(o[1]->as_checked<String>()->value_get());
      Symbol pname = Symbol(o[2]->as_checked<String>()->value_get());
      rUVar v = self->slot_get(vname)->as<UVar>();
      if (v && matchCachedProps(pname))
      {
        return true_class;
      }
      else
        return self->property_has(vname, pname)?true_class:false_class;
    };
    static rObject parentSetProperty(const objects_type& o)
    {
      if (o.size() != 4)
        runner::raise_arity_error(o.size()-1, 3);
      Object* self = o[0];
      Symbol vname = Symbol(o[1]->as_checked<String>()->value_get());
      Symbol pname = Symbol(o[2]->as_checked<String>()->value_get());
      rUVar v = self->slot_get(vname)->as<UVar>();
      if (v && matchCachedProps(pname))
      {
        return v->slot_update(pname, o[3]);
      }
      else
        return self->property_set(vname, pname, o[3]);
    };

    unsigned int
    UVar::notifyChange_(rObject function)
    {
      unsigned int id = ++uid_;
      bool empty = change_.empty();
      change_.push_back(std::make_pair(id, function));
      if (empty)
        loopCheck();
      GD_FINFO_DUMP("Registered change with id %s on %s", id, this);
      return id;
    }

    unsigned int
    UVar::notifyChangeOwned_(rObject function)
    {
      unsigned int id = ++uid_;
      changeOwned_.push_back(std::make_pair(id, function));
      GD_FINFO_DUMP("Registered changeOwned with id %s on %s ", id, this);
      return id;
    }

    unsigned int
    UVar::notifyAccess_(rObject function)
    {
      unsigned int id = ++uid_;
      bool empty = access_.empty();
      access_.push_back(std::make_pair(id, function));
      if (empty)
      {
        if (!loopCheck())
          call(SYMBOL(hookChangedEvent)); // monitor changed.onSubscribe
      }
      GD_FINFO_DUMP("Registered access with id %s on %s", id, this);
      return id;
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
        {
          location_type l = o->slot_locate(libport::Symbol(p.second), true);
          return l.first?l.second->value():0;
        }
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
      : Primitive(boost::function1<rObject, objects_type>
                  (boost::bind(&UVar::accessor, this, _1)))
      , changeConnections(new List)
      , rangemin(-std::numeric_limits<libport::ufloat>::infinity())
      , rangemax(std::numeric_limits<libport::ufloat>::infinity())
      , val(void_class)
      , valsensor(void_class)
      , looping_(false)
      , inAccess_(false)
      , waiterCount_(0)
      , owned(false)
      , changed_(0)
    {
      proto_add(proto ? rPrimitive(proto) : Primitive::proto);
      slot_set(SYMBOL(waiterTag), new Tag());
    }

    UVar::UVar(libport::intrusive_ptr<UVar>)
      : Primitive(boost::function1<rObject, const objects_type&>
                  (boost::bind(&UVar::accessor, this, _1)))
      , changeConnections(new List)
      , rangemin(-std::numeric_limits<libport::ufloat>::infinity())
      , rangemax(std::numeric_limits<libport::ufloat>::infinity())
      , val(void_class)
      , valsensor(void_class)
      , looping_(false)
      , inAccess_(false)
      , waiterCount_(0)
      , owned(false)
      , changed_(0)
    {
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
#define DECLARE(Name)           \
      bind(SYMBOL_(Name), &UVar::Name)

      DECLARE(writeOwned);
      DECLARE(update_timed);
      DECLARE(loopCheck);
      DECLARE(accessor);
      DECLARE(update_);
      DECLARE(update_timed_);
      DECLARE(owned);
      DECLARE(initialName);
      DECLARE(changeConnections);
      DECLARE(removeNotifyChange);
      DECLARE(removeNotifyChangeOwned);
      DECLARE(removeNotifyAccess);
      DECLARE(notifyChange_);
      DECLARE(notifyChangeOwned_);
      DECLARE(notifyAccess_);
      DECLARE(rangemax);
      DECLARE(rangemin);
      DECLARE(timestamp);
      DECLARE(val);
      DECLARE(valsensor);
#undef DECLARE
      bind(SYMBOL(changed), &UVar::changed_get);
      slot_set(SYMBOL(parentGetProperty), new Primitive(&parentGetProperty));
      slot_set(SYMBOL(parentSetProperty), new Primitive(&parentSetProperty));
      slot_set(SYMBOL(parentHasProperty), new Primitive(&parentHasProperty));
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
        callNotify(r, rUVar(this), accessInLoop_);
        rObject period = System->call(SYMBOL(period));
        r.yield_for(libport::utime_t(period->as<Float>()->value_get()
                                     * 1000000.0));
      }
    }

    bool
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
          (!change_.empty()
           || (changed_ && changed_get()->hasSubscribers())
           ||!changeConnections->empty()
           )
          && !access_.empty();
      GD_FINFO_TRACE("Loopcheck on %s: %s", initialName, loopChecker);
      if (loopChecker)
      {
        // There is no need to keep an accessor present if we are
        // going to trigger it periodicaly.
        std::swap(access_, accessInLoop_);
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
       return loopChecker;
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
        val = (owned?valsensor:this->val)->as<UValue>();
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
      // Prevent loopback notification on the remote who called us.
      if (!r->slot_has(SYMBOL(DOLLAR_uobjectInUpdate)))
        r->slot_set(SYMBOL(DOLLAR_uobjectInUpdate), slot_get(SYMBOL(fullName)));
      update_timed_(val, timestamp);
      r->slot_remove(SYMBOL(DOLLAR_uobjectInUpdate));
      return void_class;
    }

    rObject
    UVar::update_timed_(rObject val, libport::utime_t timestamp)
    {
      this->timestamp = timestamp / 1000000.0;
      // Apply rangemax/rangemin for float and encapsulated float
      // Do not bother with UValue for numeric types.
      if (rUValue uval = val->as<UValue>())
      {
        if (uval->value_get().type == urbi::DATA_DOUBLE)
        {
          ufloat f = uval->value_get().val;
          f = std::min(rangemax, std::max(f, rangemin));
          val = to_urbi(f);
        }
      }
      else if (rFloat vf = val->as<Float>())
      {
        ufloat f = vf->value_get();
        f = std::min(rangemax, std::max(f, rangemin));
        val = to_urbi(f);
      }
      else
        val = val->call(SYMBOL(uvalueDeserialize));
      runner::Runner& r = ::kernel::runner();
      this->val = val;
      if (owned)
        callNotify(r, rUVar(this), changeOwned_);
      else
      {
        checkBypassCopy();
        bool isIn = libport::has(inChange_, &r);
        GD_FINFO_DUMP("update calling notify if %s", isIn);
        if (!isIn)
        {
          inChange_.push_back(&r);
          FINALLY(((std::vector<void*>&, inChange_))
                  ((runner::Runner&, r)),
                   for (unsigned i=0; i<inChange_.size(); ++i)
                  if (inChange_[i] == &r)
                  {
                    if (i != inChange_.size()-1)
                      inChange_[inChange_.size()-1] = inChange_[i];
                    inChange_.pop_back();
                  }
                  );
          callNotify(r, rUVar(this), change_);
          callConnections(r, rObject(this), changeConnections);
        }
        changed();
      }
      return val;
    }

    rObject
    UVar::accessor(const objects_type&)
    {
      URBI_AT_HOOK(changed);
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
        callNotify(r, rUVar(this), access_);
        inAccess_ = false;
      }
      rObject res = owned?valsensor:val;
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
            res = owned?valsensor:val;
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
      valsensor = newval;
      checkBypassCopy();
      callNotify(r, rUVar(this), change_);
      callConnections(r, rObject(this), changeConnections);
      changed();
      return newval;
    }
    bool
    UVar::removeNotifyChange(unsigned int id)
    {
      return removeCallback(change_, id);
    }
    bool
    UVar::removeNotifyChangeOwned(unsigned int id)
    {
      return removeCallback(changeOwned_, id);
    }
    bool
    UVar::removeNotifyAccess(unsigned int id)
    {
      return removeCallback(access_, id) ||
        removeCallback(accessInLoop_, id);
    }
    bool
    UVar::removeCallback(Callbacks& cb, unsigned int id)
    {
      for (unsigned int i=0; i<cb.size(); ++i)
      {
        if (cb[i].first == id)
        {
          // Remove by swapping with last since order is not important.
          if (i != cb.size() -1)
            cb[i] = cb[cb.size()-1];
          cb.pop_back();
          return true;
        }
      }
      return false;
    }

    /*
      SYMBOL(changed)
    */
    URBI_ATTRIBUTE_ON_DEMAND_IMPL(UVar, Event, changed);
  }
}
