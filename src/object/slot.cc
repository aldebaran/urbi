/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/global.hh>
#include <urbi/object/slot.hh>
#include <urbi/object/slot.hxx>
#include <urbi/object/event.hh>
#include <urbi/object/job.hh>
#include <urbi/object/symbols.hh>
#include <object/uconnection.hh>
#include <object/uvalue.hh>
#include <runner/job.hh>
#include <urbi/kernel/uconnection.hh>
#include <eval/call.hh>

GD_CATEGORY(Urbi.Slot);
namespace urbi
{
  namespace object
  {
    URBI_CXX_OBJECT_INIT(Slot)
    {
      Ward w(this);
      // The BIND below will create slots that will use this, aka proto,
      // as their proto, so we must be valid right now.
      proto = this;
      proto_add(Object::proto);
      init();
      // FIXME: bind get/set mechanism in urbiscript
      bind("n", &Slot::normalized, &Slot::normalized_set);
      BIND(dead, dead_);
      BIND(split, split_);
      BIND(owned, split_); // for backward
      BIND(value, value_);
      BIND(timestamp, timestamp_);
      BIND(outputValue, output_value_);
      BIND(rangemax, rangemax_);
      BIND(rangemin, rangemin_);
      bind("set", &Slot::set_get, &Slot::set_set);
      bind("get", &Slot::get_get, &Slot::get_set);
      bind("oset", &Slot::oset_get, &Slot::oset_set);
      bind("oget", &Slot::oget_get, &Slot::oget_set);
      BIND(constant, constant_);
      bind("rtp", &Slot::rtp_get, &Slot::rtp_set);
      slot_remove(SYMBOL(type));
      BIND(type, type_);
      BIND(get_get); // debug
      BIND(set_get);
      BIND(oget_get); // debug
      BIND(oset_get);
      BIND(updateHook, updateHook_);
      BIND(copyOnWrite, copyOnWrite_);
      BIND(setOutputValue, set_output_value);
      BIND(pushPullCheck, push_pull_check);
      BIND(update_timed);
      rSlot s(new Slot);
      slot_set(SYMBOL(changed), s);
      boost::function2<rObject, Slot&, rObject>
        getter(boost::bind(&Slot::changed, _1));
      s->oget_set(primitive(getter));
      s->constant_set(true);
    }

    rObject
    Slot::property_get(libport::Symbol k)
    {
      return slot_get_value(k, false);
    }

    /// FIXME: does not work with "changed".
    bool
    Slot::property_has(libport::Symbol k) const
    {
      return slot_has(k);
    }

    bool
    Slot::property_set(libport::Symbol k, rObject value)
    {
      Object::location_type r = slot_locate(k, true);
      if (!r.first)
        slot_set_value(k, value);
      else
        slot_update(k, value);
      return !r.first;
    }

    void
    Slot::property_remove(libport::Symbol k)
    {
      slot_remove(k);
    }

    // FIXME: Does not work with changed.
    Slot::properties_type*
    Slot::properties_get()
    {
      return 0;//local_slots_get();
    }

    void
    Slot::update_timed(rObject val,  libport::utime_t timestamp)
    {
      uobject_set(val, nil_class, timestamp);
    }

    void
    Slot::uobject_set(rObject val, Object* sender, libport::utime_t timestamp)
    {
      rJob r = kernel::runner().as_job();
      // Prevent loopback notification on the remote who called us.
      ufloat f = (unsigned long)(void*)this;
      if (!r->slot_has(SYMBOL(DOLLAR_uobjectInUpdate)))
        r->slot_set_value(SYMBOL(DOLLAR_uobjectInUpdate),
                          new Float(f));
      set(val, sender, timestamp);
      r->slot_remove(SYMBOL(DOLLAR_uobjectInUpdate));
    }

    void
    Slot::set(rObject value, Object* sender)
    {
      set(value, sender, libport::utime());
    }

    void
    Slot::set(rObject value, Object* sender, libport::utime_t timestamp)
    {
      static rObject void_object = capture(SYMBOL(void), Object::package_lang_get());
      GD_FINFO_DUMP("Slot::set, slot %s, sender %s, oset %s",
        this, sender, !!oset_);
      if (type_)
      {
        if (!value->call(SYMBOL(isA), type_)->as_bool())
          runner::raise_type_error(value/*->call(SYMBOL(type))?*/, type_);
      }
      timestamp_ = timestamp / 1000000.0;
      has_uvalue_ = false;
      // Apply rangemax/rangemin for float and encapsulated float
      // Do not bother with UValue for numeric types.
      if (rUValue uval = value->as<UValue>())
      {
        if (uval->value_get().type == urbi::DATA_DOUBLE)
        {
          ufloat f = uval->value_get().val;
          f = std::min(rangemax_, std::max(f, rangemin_));
          value = to_urbi(f);
        }
        else
          has_uvalue_ = true;
      }
      else if (rFloat vf = value->as<Float>())
      {
        ufloat f = vf->value_get();
        ufloat tf = std::min(rangemax_, std::max(f, rangemin_));
        // Do not touch the input if unchanged.
        if (tf != f)
        {
          // Ideally we should call new on vf, but we can't
          value = to_urbi(tf);
        }
      }
      if (set_)
      {
        object::objects_type args;
        args << value;
        rObject res = eval::call_apply(::kernel::runner(),
                         const_cast<Slot*>(this), set_, SYMBOL(set), args);
        if (res != void_object)
          value_ = res;
      }
      if (sender && oset_)
      {
        if (set_) // Re-fetch the value that might have been modified by set
          value = value_;
        object::objects_type args;
        args << sender << value << this;
        rObject res = eval::call_apply(::kernel::runner(),
                         oset_.get(), SYMBOL(oset), args, 0,
                         boost::optional< ::ast::loc>(),
                          Primitive::CALL_IGNORE_EXTRA_ARGS
                         );
        if (res != void_object)
          value_ = res;
      }
      if (!sender)
      {
        GD_INFO_TRACE("Set without seneder");
      }
      if (constant_)
        runner::raise_const_error();

      // Write input if no setter was called
      if (!set_ && (!oset_ || !sender))
        value_ = value;

      if (!split_) // input->output in non-split mode
        set_output_value(value);
    }

    void
    Slot::set_output_value(rObject v)
    {
      URBI_SCOPE_DISABLE_DEPENDENCY_TRACKER;
      GD_FINFO_DUMP("Slot::set_output_value, slot %s, val %s changed %s ",
                    this, v, changed_);
      output_value_ = v;
      has_uvalue_ = v->as<UValue>();
      check_waiters();
      // Both optim and let us run the init phase with no runner.
      if (!changed_)
        return;
      runner::Job& r = ::kernel::runner();
      bool isIn = libport::has(in_setter_, &r);
      if (!isIn)
      {
        GD_FINFO_DUMP("set_output_value on %s: disabling notifies", this);
        in_setter_.push_back(&r);
        FINALLY(((std::vector<void*>&, in_setter_))
                ((runner::Job&, r)),
                for (unsigned i=0; i<in_setter_.size(); ++i)
                  if (in_setter_[i] == &r)
                  {
                    if (i != in_setter_.size()-1)
                      in_setter_[in_setter_.size()-1] = in_setter_[i];
                    in_setter_.pop_back();
                  }
                  );
        objects_type nothing;
        if (changed_)
          changed_->as<object::Event>()->syncEmit(nothing);
      }
    }

    void
    Slot::check_waiters()
    {
      // If there are blocked reads, call extract to force caching of the
      // temporary value, and unblock them.
      if (waiter_count_)
      {
        // Split val declaration and assignment to work around g++
        // 4.3.3 which warns:
        // intrusive-ptr.hxx:89: error:
        //   'val.libport::intrusive_ptr<urbi::object::UValue>::pointee_'
        //   may be used uninitialized in this function.
        rUValue val;
        val = (split_ ? output_value_ : value_)->as<UValue>();
        if (val)
          val->extract();
        if (waiter_tag_)
          waiter_tag_->call(SYMBOL(stop));
      }
    }

    rObject
    Slot::init(bool fromModel)
    {
      has_uvalue_ = false;
      oget_ = oset_ = get_ = set_ = 0;
      type_ = 0;
      rSlot model;
      if (fromModel)
        model = protos_get_first()->as<Slot>();
      in_getter_ = 0;
      waiter_count_ = 0;
      dead_ = false;
      push_pull_loop_ = false;
      if (!model || model == this)
      {
        constant_ = false;
        copyOnWrite_ = true;
        split_ = false;
        rtp_ = false;
        value_ = void_class;
        output_value_ = void_class;
        timestamp_ = 0;
        rangemax_ = std::numeric_limits<libport::ufloat>::infinity();
        rangemin_ = -std::numeric_limits<libport::ufloat>::infinity();
      }
      else
      {
        constant_ = model->constant_;
        copyOnWrite_ = model->copyOnWrite_;
        split_ = model->split_;
        rtp_ = model->rtp_;
        value_ = model->value_;
        output_value_ = model->output_value_;
        timestamp_ = model->timestamp_;
        rangemax_ = model->rangemax_;
        rangemin_ = model->rangemin_;
        if (model->set_)
          set_ = model->set_->call(SYMBOL(new));
        if (model->get_)
          get_ = model->get_->call(SYMBOL(new));
        if (model->oset_)
          oset_ = model->oset_->call(SYMBOL(new));
        if (model->oget_)
          oget_ = model->oget_->call(SYMBOL(new));
      }
      return void_class;
    }

    Slot::Slot(rSlot model)
    {
      Ward w(this);
      aver(model);
      proto_set(model);
      init(true);
    }

    Slot::Slot(const Slot& model)
      : CxxObject()
      , has_uvalue_(false)
    {
      //std::cerr <<"slot copy " << &model <<" -> " << this
      //<< " oset " << model.oset_ << std::endl;
      Ward w(this);
      aver(&model);
      proto_set(rSlot(const_cast<Slot*>(&model)));
      init(true);
      //NM: I don't think calling the setter is a good idea here
      // Main usage for this function is the COW that will call
      // the setter immediately after. So calling it with an outdated value
      // seems bad.
    }

    rObject
    Slot::value_special(Object* sender, bool fromUObject) const
    {
      if (in_getter_ > 3)
      {
        // Some level of reentrency is possible when using a getter that writes
        // using set_output_value and watchers(at).
        if (in_getter_ > 3)
          GD_FWARN("Possible loop detected accessing slot %s", this);
        return split_ ? output_value_ : value_;
      }
      FINALLY(((int&, in_getter_)), --in_getter_);
      ++in_getter_;
      rObject res;
      if (sender && oget_)
      {
        object::objects_type args;
        args << sender << const_cast<Slot*>(this);
        /*
        if (rPrimitive p = oget_->as<Primitive>())
        {
          res = p->call_raw(args, Primitive::CALL_IGNORE_EXTRA_ARGS);
        }
        else*/
        {

         res = eval::call_apply(::kernel::runner(),
                                 oget_.get(),
                                 SYMBOL(oget),
                                 args,
                                 0,
                                 boost::optional< ::ast::loc>(),
                                 Primitive::CALL_IGNORE_EXTRA_ARGS
                                 );
        }
      }
      if (get_)
      {
        object::objects_type args;
        res = eval::call_apply(::kernel::runner(),
                              const_cast<Slot*>(this),
                              get_, SYMBOL(get), args);
      }
      if (!res)
        res = split_ ? output_value_ : value_;
      if (!fromUObject)
      {
        rUValue bv = res->as<UValue>();
        if (bv && bv != UValue::proto)
        {
          if (!bv->bypassMode_get() || bv->extract() != nil_class)
            res = bv->extract();
          else
          {
            URBI_SCOPE_DISABLE_DEPENDENCY_TRACKER;
            // This is a read on a bypass-mode UVar, from outside any
            // notifychange: the value is not available.
            // So we mark that we wait by inc-ing waiter_count, and wait
            // on waiterTag until we timeout, or someone writes to the UVar
            // and unlock us.

            // free the shared ptrs
            res.reset();
            bv.reset();
            ++waiter_count_;
            waiter_tag()->call(SYMBOL(waitUntilStopped), new Float(0.5));
            --waiter_count_;
            // The val slot likely changed, fetch it again.
            res = split_ ? output_value_ : value_;
            if (rUValue bv = res->as<UValue>())
            {
              res = bv->extract();
              if (res == nil_class)
                GD_WARN("Timeout on UVar in bypass mode.");
            }
          }
        }
      }
      aver(res);
      return res;
    }

    float
    Slot::normalized()
    {
      if (!std::isfinite(rangemin_) || !std::isfinite(rangemax_))
        RAISE("ranges are not finite");
      rObject v = value();
      if (rFloat rf = v->as<Float>())
      {
        ufloat f = rf->value_get();
        return (f - rangemin_) / (rangemax_ - rangemin_);
      }
      else
        FRAISE("Value is not a float");
    }

    void
    Slot::normalized_set(float v)
    {
      if (!std::isfinite(rangemin_) || !std::isfinite(rangemax_))
        FRAISE("ranges are not finite");
      ufloat tv = v*(rangemax_-rangemin_) + rangemin_;
      set(to_urbi(tv));
    }

    rTag
    Slot::waiter_tag() const
    {
      if (!waiter_tag_)
        waiter_tag_ = new Tag();
      return waiter_tag_->as<Tag>();
    }

    void
    Slot::get_set(const rObject& o)
    {
      bool had_one = get_ || oget_;
      get_ = o == nil_class ? 0 : o;
      push_pull_check(!had_one && (get_ || oget_));
    }

    void
    Slot::oget_set(const rObject& o)
    {
      bool had_one = get_ || oget_;
      oget_ = o == nil_class ? 0 : o;
      push_pull_check(!had_one && (get_ || oget_));
    }

    void
    Slot::oset_set(const rObject& o)
    {
      oset_ = o == nil_class ? 0 : o;
    }

    void
    Slot::set_set(const rObject& o)
    {
      set_ = o == nil_class ? 0 : o;
    }

    rObject
    Slot::push_pull_loop_run(runner::Job& r)
    {
      GD_FINFO_TRACE("Push-pull loop starting on %s", this);
      r.state.this_set(this);
      // Prepare a call to System.period.  Keep its computation in the
      // loop, so that we can change it at run time.
      CAPTURE_GLOBAL(System);
      while (true)
      {
        // UObjects need an access to the getter, and
        // watchers need a changed! . But if there is a watcher, the changed!
        // will trigger reevaluation which will call the getter.

        // We must protect against reetrant calls to changed as it confuses
        // the dependency tracker, as is done in setter().
        runner::Job& r = ::kernel::runner();
        bool isIn = libport::has(in_setter_, &r);
        if (!isIn)
        {
          GD_FINFO_DUMP("set_output_value on %s: disabling notifies", this);
          in_setter_.push_back(&r);
          FINALLY(((std::vector<void*>&, in_setter_))
            ((runner::Job&, r)),
            for (unsigned i=0; i<in_setter_.size(); ++i)
              if (in_setter_[i] == &r)
              {
                if (i != in_setter_.size()-1)
                  in_setter_[in_setter_.size()-1] = in_setter_[i];
                in_setter_.pop_back();
              }
              );
          objects_type nothing;
          if (changed_)
            changed_->as<object::Event>()->syncEmit(nothing);
        }
        //changed_->as<Event>()->syncEmit();
        rObject period = System->call(SYMBOL(period));
        r.yield_for(libport::utime_t(period->as<Float>()->value_get()
                                     * 1000000.0));
      }
      return object::void_class;
    }

    bool
    Slot::push_pull_check(bool first_getter)
    {

      // Activate if there is at least one getter and at least one
      // notifychange-like.
      bool need_loop =
       !push_pull_loop_ &&
       (get_ || oget_) &&
       (changed_ && changed_->as<Event>()->hasSubscribers()) &&
       hasLocalSlot(SYMBOL(watchIncompatible))
       ;
       /*std::cerr <<"ppchecking "<< push_pull_loop_ <<" " << need_loop << " "
       << (changed_ && changed_->as<Event>()->hasSubscribers())
       << std::endl;*/
      if (need_loop)
      {
        push_pull_loop_ = true;
        runner::Job* nr =
          new runner::Job(
            ::kernel::urbiserver->ghost_connection_get().lobby_get(),
                          kernel::runner().scheduler_get());
        nr->name_set("pushPullLoop");
        nr->set_action(boost::bind(&Slot::push_pull_loop_run, this, _1));
        // Remove the lobby's tag.
        nr->state.tag_stack_clear();
        nr->start_job();
      }
      if (!need_loop && first_getter && changed_)
      {
        GD_FINFO_TRACE("hookChanged %s", this);
        call(SYMBOL(hookChangedEvent));
      }
      GD_FINFO_TRACE("push_pull_check: %s", need_loop);
      //std::cerr <<"ppchecking done"<<std::endl;
      return need_loop;
    }

    rObject
    Slot::changed()
    {
      URBI_SCOPE_DISABLE_DEPENDENCY_TRACKER;
      CAPTURE_GLOBAL(Event);
      if (!changed_)
      {
        changed_ = Event->call(SYMBOL(new));
        GD_FPUSH_TRACE("Creating changed for %s: %s", this, changed_);
        push_pull_check(get_ || oget_);
      }
      return changed_;
    }

    void
    Slot::rtp_set(bool v)
    {
      if (v && !rtp_)
      {
        rtp_ = true;
        call(SYMBOL(enableRTP));
      }
      rtp_ = v;
    }

    Slot::Slot(rObject& val)
    {
      //Ward w(this);
      if (!proto)
        proto = new Slot(FirstPrototypeFlag());
      proto_set(proto);
      init();
      if (val)
        value_ = val;
      has_uvalue_ = val?(bool)val->as<UValue>():false;
    }

    const size_t Slot::allocator_static_max_size = sizeof(Slot);
  }
}
