/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file urbi/object/object.cc
 ** \brief Implementation of object::Object.
 */

#include <algorithm>
#include <climits>

#include <boost/lambda/lambda.hpp>

#include <libport/cassert>
#include <libport/containers.hh>
#include <libport/debug.hh>
#include <libport/foreach.hh>
#include <libport/format.hh>
#include <libport/lexical-cast.hh>

#include <urbi/kernel/userver.hh>

#include <urbi/object/cxx-conversions.hh>
#include <urbi/object/dictionary.hh>
#include <urbi/object/event.hh>
#include <urbi/object/float.hh>
#include <urbi/object/global.hh>
#include <urbi/object/hash.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>
#include <object/root-classes.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/urbi-exception.hh>

#include <urbi/runner/raise.hh>
#include <runner/job.hh>

#include <eval/send-message.hh>
#include <eval/call.hh>

#include <urbi/object/centralized-slots.hxx>
GD_CATEGORY(Urbi.Object);


DECLARE_LOCATION_FILE;

namespace urbi
{
  namespace object
  {
    namespace
    {
      /// Use xalloc/iword to store our current depth within the stream object.
      static
      inline
      long&
      current_depth(std::ostream& o)
      {
        static const long idx = std::ios::xalloc();
        return o.iword(idx);
      }

      /// Send a warning about something that will be an error in a future
      /// release.
      static
      void
      warn_hard(const std::string& msg)
      {
        runner::Job& r = ::kernel::runner();
        eval::send_message(r, "warning", libport::format("!!! %s", msg));
        eval::show_backtrace(r, "warning");
      }

    }


    Object::Object()
      : slotAdded_(0)
      , slotRemoved_(0)
      , protos_(0)
      , slots_()
      , lookup_id_(INT_MAX)
    {
    }

    Object::~Object ()
    {
      slots_.finalize(this);
      if (!protos_cache_)
        delete protos_;
    }

    /*--------.
    | Slots.  |
    `--------*/

    rList
    Object::urbi_protos_get()
    {
      if (protos_cache_)
        return protos_cache_->as<List>();

      rList protos;
      if (protos_)
      {
        protos = new List(*protos_);
        delete protos_;
      }
      else
      {
        protos = new List();
        if (proto_)
          protos->value_get().push_back(proto_);
        proto_ = 0;
      }
      protos_cache_ = protos;
      protos_ = &protos->value_get();
      return protos;
    }

    void
    Object::proto_set(const rObject& o)
    {
      if (!protos_cache_)
        delete protos_;
      protos_cache_ = 0;
      protos_ = 0;
      proto_ = o;
    }

    void
    Object::protos_set(const rList& l)
    {
      proto_set(0);
      if (l->value_get().empty())
        return;
      if (l->value_get().size() == 1)
        proto_ = l->value_get().front();
      else
      {
        protos_cache_ = l;
        protos_ = &l->value_get();
        proto_ = 0;
      }
    }

    static int lookup_id = 0;

    inline Object::location_type
    Object::slot_locate_(key_type k) const
    {
      if (lookup_id_ == lookup_id)
        return location_type((Object*)0, rObject());
      lookup_id_ = lookup_id;
      if (rObject slot = local_slot_get(k))
        return location_type(const_cast<Object*>(this), slot);
      if (proto_)
      {
        location_type rec = proto_->slot_locate_(k);
        if (rec.first)
          return rec;
      }
      else if (protos_) // Braces to pacify G++.
      {
        foreach (const rObject& proto, *protos_)
        {
          location_type rec = proto->slot_locate_(k);
          if (rec.first)
            return rec;
        }
      }
      return location_type((Object*)0, rObject());
    }

    static bool fastHook = getenv("URBI_FAST_HOOK");
    rObject
    Object::local_slot_get(key_type k) const
    {
      rObject res = slots_.get(this, k);
      if(!fastHook)
      {
        if (res)
        {
          URBI_AT_HOOK(slotRemoved);
        }
        else
        {
          URBI_AT_HOOK(slotAdded);
        }
      }
      return res;
    }

    rObject
    Object::local_slot_get_value(key_type k) const
    {
      rObject s = local_slot_get(k);
      if (s)
      {
        if (rSlot rs = s->as<Slot>())
          return rs->value(const_cast<Object*>(this));
        else
          return s;
      }
      else
        return 0;
    }

    Object::location_type
    Object::slot_locate(key_type k,
                        bool fallback) const
    {
      ++lookup_id;
      Object::location_type res = slot_locate_(k);
      if (!res.first && fallback)
      {
        ++lookup_id;
        res = slot_locate_(SYMBOL(fallback));
      }
      return res;
    }

    Object::location_type
    Object::safe_slot_locate(key_type k) const
    {
      location_type r = slot_locate(k, true);
      if (!r.first)
        runner::raise_lookup_error(k, const_cast<Object*>(this));
      return r;
    }

    rObject
    Object::slot_get_value(key_type k, bool throwOnFailure) const
    {
      rObject o = slot_get(k, throwOnFailure);
      if (!o)
        return 0;
      Slot* s = o->as<Slot>();
      if (s)
        o = s->value(const_cast<Object*>(this));
      aver(o);
      return o;
    }

    rObject
    Object::slot_get(key_type k, bool throwOnFailure) const
    {
      location_type loc = throwOnFailure?safe_slot_locate(k):slot_locate(k);
      if (!loc.first)
        return 0;
      rObject res = loc.second;
      runner::Job* r = ::kernel::urbiserver->getCurrentRunnerOpt();
      if (r && r->dependencies_log_get() && !res->as<Slot>())
      {
        // We want to hook changed, so create on-demand slot
        rSlot rs = new Slot(res);
        GD_FINFO_TRACE("Transparent slot creation for %s: %s", k, rs);
        loc.first->slots_.set(loc.first, k, rs, true);
        res = rs;
        // no need to hook anything, slot getter will take care of that.
      }
      return res;
    }


    Object&
    Object::slot_set_value(key_type k, rObject o, bool constant)
    {
      if (!o)
        abort();
      // No slot by default
      if (constant || o->as<Slot>())
      {
        rSlot slot(new Slot(o));
        slot->constant_set(constant);
        return slot_set(k, slot);
      }
      else
        return slot_set(k, o);
    }

    namespace
    {
      inline
      bool redefinition_mode()
      {
        runner::Job* r = ::kernel::urbiserver->getCurrentRunnerOpt();
        return r && r->state.redefinition_mode_get();
      }
    }

    Object&
    Object::slot_set(key_type k, rObject o)
    {
      if (!o)
        abort();
      if (!slots_.set(this, k, o, redefinition_mode()))
      {
        GD_CATEGORY(Urbi.Error);
        GD_FINFO_DEBUG("Slot redefinition: %s", k);
        runner::raise_urbi_skip(SYMBOL(Redefinition), to_urbi(k));
      }
      if (!fastHook)
        slotAdded();
      return *this;
    }

    Object&
    Object::slot_set(key_type key, rObject getter, rObject setter)
    {
      rSlot s(new Slot);
      if (getter)
        s->oget_set(getter);
      if (setter)
        s->oset_set(setter);
      s->copyOnWrite_set(false);
      slot_set(key, s);
      return *s;
    }

    Object&
    Object::slot_copy(key_type name, const rObject& from)
    {
      this->slot_set(name, from->slot_get(name));
      return *this;
    }

    Slot&
    Object::slot_copy_on_write(key_type name, Slot& slot)
    {
      // Careful, we want to call the ctor(Slot&) that copies the slot.
      rSlot cow = slot.call(SYMBOL(new))->as<Slot>();
      slot_set(name, cow);
      return *cow;
    }

    bool
    Object::slot_has(key_type k) const
    {
      return slot_locate(k).first;
    }

    rObject
    Object::slot_update(key_type k, const rObject& o,
                        bool hook)
    {
      // The updated slot.
      location_type r = slot_locate(k, true);
       if (!r.first)
        runner::raise_lookup_error(k, const_cast<Object*>(this));
      return slot_update_with_cow(k, o, hook, r);
    }
    rObject
    Object::slot_update_with_cow(key_type k, const rObject& o,
                        bool hook, location_type& r)
    {
      // Value to write to the slot.
      rSlot s = r.second->as<Slot>();
      rObject v = o;

      // If the current value in the slot to be written in has a slot
      // named 'updateHook', call it, passing the object owning the
      // slot, the slot name and the target.
      if (hook && s)
        if (rObject hook = s->updateHook_get())
        {
          objects_type args;
          args << rObject(this)
               << new String(k)
               << o;
          v = eval::call_apply(::kernel::runner(),
                               hook, SYMBOL(updateHook), args);
          // If the updateHook returned void, do nothing. Otherwise let
          // the slot be overwritten.
          if (v == object::void_class)
            return o;
        }

      // If return-value of hook is not void, write it to slot.
      // Copy on write check
      // Assumes copy on write is on by default.
      if (r.first == this || (s && !s->copyOnWrite_get()))
      { // no-cow case
        if (s && s->constant_get())
          runner::raise_const_error();
        runner::Job* j = ::kernel::urbiserver->getCurrentRunnerOpt();
        bool d = false; // Initialize or GCC complains.
        if (j)
        {
          d = j->dependencies_log_get();
          j->dependencies_log_set(false);
        }
        FINALLY(((runner::Job*, j))((bool, d)),
                if (j) j->dependencies_log_set(d));

        if (!s)
        { // Direct object, no slot
          if (v->as<Slot>())
          {
            // Create a slot with the value in it.
            // This is the ctor taking a rObject as slot value.
            s = new Slot(v);
            slots_.set(r.first, k, s, true);
          }
          else
            slots_.set(r.first, k, v, true);
        }
        else // Slot present, update it.
          s->set(v, this);
      }
      else
      {// cow case
        if (s)
        {
          // Slot present in parent, copy it.
          Slot& slot = slot_copy_on_write(k, *s);
          bool cst = slot.constant_get();
          FINALLY(((Slot&, slot))((bool, cst)), slot.constant_set(cst));
          slot.constant_set(false);
          slot.set(v, this);
        }
        else
        { // No slot present
          if (v->as<Slot>())
          { // We need a slot.
            rSlot slot(new Slot(v));
            slots_.set(this, k, slot, true);
          }
          else
            slots_.set(this, k, v, true);
        }
      }
      return v;
    };


    /*-------------.
    | Properties.  |
    `-------------*/

    rDictionary
    Object::properties_get()
    {
      Dictionary::value_type res;
      for (slots_implem::const_iterator slot = slots_.begin(this);
           slot != slots_.end(this);
           ++slot)
        res[new String(slot->first.second)] = properties_get(slot->first.second);
      return new Dictionary(res);
    }

    rDictionary
    Object::properties_get(key_type k)
    {
      rObject o = slot_get(k);
      rSlot rs = o->as<Slot>();
      if (!rs)
      {
        Dictionary::value_type res;
        res[new String("constant")] = to_urbi(false);
        return new Dictionary(res);
      }
      Slot& s = *rs;
      Dictionary::value_type res;
      for (slots_implem::const_iterator slot = slots_.begin(&s);
           slot != slots_.end(&s);
           ++slot)
      {
        if (rSlot rs = slot->second->as<Slot>())
          res[new String(slot->first.second)] = rs->value(this);
        else
          res[new String(slot->first.second)] = slot->second;
      }
      // Add cached slots
      res[new String("constant")] = to_urbi(s.constant_get());
      if (s.get_get())
        res[new String("get")] = s.get_get();
      if (s.set_get())
        res[new String("set")] = s.set_get();
      if (s.oget_get())
        res[new String("oget")] = s.oget_get();
      if (s.oset_get())
        res[new String("oset")] = s.oset_get();
      // Should we display this?
      //if (s.value_get())
        //res[new String("value")] = s.value_get();
      return new Dictionary(res);
    }

    rObject
    Object::property_get(key_type slot, key_type prop)
    {
      location_type loc = safe_slot_locate(slot);
      rObject val = loc.second;
      if (rSlot s = val->as<Slot>())
      {
        if (rObject res = s->property_get(prop))
          return res;
      }
      else if (prop == SYMBOL(oget)
            || prop == SYMBOL(oset)
            || prop == SYMBOL(get)
            || prop == SYMBOL(set))
        return void_class;
      else if (prop == SYMBOL(constant))
          return to_urbi(false);
      else if (prop == SYMBOL(copyOnWrite))
          return to_urbi(true);
      else if (prop == SYMBOL(changed))
      {
        // Create the slot
        rSlot rs(new Slot(val));
        slots_.set(loc.first, slot, rs, true);
        return rs->property_get(prop);
      }
      else if (prop == SYMBOL(rangemin))
        return to_urbi(-std::numeric_limits<libport::ufloat>::infinity());
      else if (prop == SYMBOL(rangemax))
        return to_urbi(std::numeric_limits<libport::ufloat>::infinity());
      runner::raise_urbi_skip(SYMBOL(PropertyLookup),
                              this, to_urbi(slot), to_urbi(prop));
    }

    bool
    Object::property_has(key_type k, key_type p)
    {
      if (rSlot s = slot_get(k)->as<Slot>())
        return s->property_has(p);
      else
        return false;
    }

    rObject
    Object::property_set(key_type k,
                         key_type p,
                         const rObject& value)
    {
      // CoW
      location_type loc = safe_slot_locate(k);
      if (loc.first != this)
      {
        slot_set(k, slot_get(k));
        loc = safe_slot_locate(k);
      }
      rSlot rs = loc.second->as<Slot>();
      if (!rs)
      {
        rs = new Slot(loc.second);
        slots_.set(this, k, rs, true);
      }
      if (rs->property_set(p, value)
          && rs->slot_has(SYMBOL(newPropertyHook)))
        rs->call(SYMBOL(newPropertyHook),
                 this, new String(k), new String(p), value);
      return value;
    }

    rObject
    Object::property_remove(key_type k, key_type p)
    {
      rObject o = slot_get(k);
      rSlot rs = o->as<Slot>();
      if (!rs)
      {
        warn_hard(libport::format("no such property: %s->%s", k, p));
        return nil_class;
      }
      Slot& slot = *rs;
      rObject res = slot.property_get(p);
      if (res)
        slot.property_remove(p);
      else
        warn_hard(libport::format("no such property: %s->%s", k, p));
      return res;
    }

    /*-----------.
    | Printing.  |
    `-----------*/

    std::ostream&
    Object::special_slots_dump(std::ostream& o) const
    {
      return o;
    }

    std::ostream&
    Object::slot_dump(std::ostream& o,
                      const CentralizedSlots::q_slot_type& s,
                      int depth_max) const
    {
      rSlot slot = s.second->as<Slot>();
      rObject val = slot ? slot->value(const_cast<Object*>(this)) : s.second;
      o << s.first.second << " = ";
      if (slot && slot->constant_get())
        o << "const ";
      val->dump(o, depth_max)
        << libport::iendl;
      bool started = false;
      for (slots_implem::const_iterator slot = slots_.begin(s.second);
           slot != slots_.end(s.second);
           ++slot)
      {
        libport::Symbol k = libport::Symbol(slot->first.second);
        if (k == SYMBOL(constant))
          continue;
        if (!started)
          o << "  /* Properties */" << libport::incendl;
        else
          o << libport::iendl;
        o << k << " = ";
        if (rSlot s = slot->second->as<Slot>())
          s->value(val)->dump(o, depth_max);
        else
          slot->second->dump(o, depth_max);
        started = true;
      }
      if (slot)
      {
#define CHECK(N)                                                \
        if (slot->N ## _get())                                  \
        {                                                       \
          if (!started)                                         \
            o << "  /* Properties */" << libport::incendl;      \
          else                                                  \
            o << libport::iendl;                                \
          o << #N " = ";                                        \
          slot->N ## _get()->dump(o, depth_max);                \
          started = true;                                       \
        }
        CHECK(value);
        CHECK(get);
        CHECK(set);
        CHECK(oget);
        CHECK(oset);
#undef CHECK
      }
      if (started)
         o << libport::decendl;
      return o;
    }

    bool
    Object::operator< (const Object& rhs) const
    {
      return this < &rhs;
    }

    std::ostream&
    Object::id_dump(std::ostream& o) const
    {
      rObject data = const_cast<Object*>(this)->call(SYMBOL(DOLLAR_id));
      return o << from_urbi<std::string>(data);
    }


    std::ostream&
    Object::protos_dump(std::ostream& o) const
    {
      if (proto_ || (protos_ && !protos_->empty()))
      {
        o << "protos = ";
        bool first = true;
        if (proto_)
          proto_->id_dump(o);
        else
        {
          foreach (const rObject& p, *protos_)
          {
            if (!first)
              o << ", ";
            first = false;
            p->id_dump (o);
          }
        }
        o << libport::iendl;
      }
      return o;
    }

    std::ostream&
    Object::dump(std::ostream& o, int depth_max) const
    {
      id_dump(o);

      // Stop recursion at depth_max.
      if (depth_max < current_depth(o))
        return o << " <...>";
      ++current_depth(o);

      o << " {" << libport::incendl
        << "/* Special slots */" << libport::iendl;
      protos_dump(o);
      special_slots_dump(o);

      o << "/* Slots */" << libport::iendl;
      for (slots_implem::const_iterator i = slots_.begin(this);
           i != slots_.end(this);
           ++i)
        slot_dump(o, *i, depth_max);

      o << libport::decindent << '}';
      --current_depth(o);
      return o;
    }

    std::ostream&
    Object::print(std::ostream& o) const
    {
      return o << as_string();
    }

    bool
    is_a(const rObject& c, const rObject& p)
    {
      return for_all_protos(c, boost::lambda::_1 == p);
    }

    bool
    Object::as_bool() const
    {
      if (this == nil_class.get()
          || this == false_class.get())
        return false;

      if (this == void_class.get())
        runner::raise_unexpected_void_error();

      return true;
    }

    rString
    Object::asString() const
    {
      rObject s = const_cast<Object*>(this)->call(SYMBOL(asString));
      return from_urbi<rString>(s);
    }


    std::string
    Object::as_string() const
    {
      return asString()->value_get();
    }

    bool Object::valid_proto(const Object&) const
    {
      return true;
    }

    Object&
    Object::proto_add(const rObject& p)
    {
      aver(p.get(),
           "referring to a not-yet-initialized class\n"
           "See the stack trace to find the dependency to add in "
           "root_classes_initialize().");
      if (!protos_)
      {
        if (proto_ == p)
          return *this;
        else if (!proto_)
        {
          proto_ = p;
          return *this;
        }
        else
        {
          protos_ = new protos_type;
          protos_->push_back(p);
          protos_->push_back(proto_);
          proto_ = 0;
          return *this;
        }
      }
      if (!libport::has(*protos_, p))
        protos_->push_front(p);
      return *this;
    }

    std::string
    Object::type_name_get() const
    {
      return "Object";
    }

    /*---------.
    | Events.  |
    `---------*/

    /*
       SYMBOL(slotAdded)
       SYMBOL(slotRemoved)
     */
    URBI_ATTRIBUTE_ON_DEMAND_IMPL(Object, Event, slotAdded);
    URBI_ATTRIBUTE_ON_DEMAND_IMPL(Object, Event, slotRemoved);

    /*---------------.
    | Urbi methods.  |
    `---------------*/

    void
    Object::urbi_createSlot(key_type name)
    {
      slot_set_value(name, void_class);
    }

    rObject
    Object::getProperty(const std::string& slot, const std::string& prop)
    {
      return property_get(libport::Symbol(slot), libport::Symbol(prop));
    }

    rObject
    Object::getSlot(key_type name)
    {
      location_type r = safe_slot_locate(name);
      if (rSlot s = r.second->as<Slot>())
        return s;
      // Convert it to a slot.
      rSlot s(new Slot(r.second));
      slots_.set(r.first, name, s, true);
      return s;
    }

    rObject
    Object::getSlot(const std::string& name)
    {
      return getSlot(libport::Symbol(name));
    }

    bool
    Object::hasSlot(const std::string& name)
    {
      return slot_has(libport::Symbol(name));
    }

    bool
    Object::hasLocalSlot(const std::string& name)
    {
      return local_slot_get(libport::Symbol(name));
    }

    rObject
    Object::getLocalSlotValue(key_type name)
    {
      if (rObject s = local_slot_get_value(name))
        return s;
      else
        runner::raise_lookup_error(name, const_cast<Object*>(this), false);
    }

    rObject
    Object::getLocalSlot(key_type name)
    {
      if (rObject s = local_slot_get(name))
      {
        if (!s->as<Slot>())
        {
          rSlot rs(new Slot(s));
          slots_.set(this, name, rs, true);
          return rs;
        }
        else
          return s;
      }
      else
        runner::raise_lookup_error(name, const_cast<Object*>(this), false);
    }

    rObject
    Object::urbi_locateSlot(key_type name)
    {
      rObject o = slot_locate(name).first;
      return o ? o : nil_class;
    }

    rDictionary
    Object::urbi_properties(key_type name)
    {
      return properties_get(name);
    }

    rObject
    Object::urbi_removeLocalSlot(key_type name)
    {
      if (!slot_remove(name))
        runner::raise_lookup_error(name, const_cast<Object*>(this), false);
      return this;
    }

    rObject
    Object::urbi_removeSlot(key_type name)
    {
      if (!slot_remove(name))
        warn_hard(libport::format("no such local slot: %s", name));
      else
        if (!fastHook)
          slotRemoved();
      return this;
    }

    rObject
    Object::setProperty(const std::string& slot,
                        const std::string& prop, const rObject& value)
    {
      return property_set(libport::Symbol(slot), libport::Symbol(prop), value);
    }

    rObject
    Object::setSlot(key_type slot, const rObject& value)
    {
      //FIXME: for now, old setSlot...
      slot_set(slot, value);
      return value;
    }

    rObject
    Object::setSlot(const std::string& slot, const rObject& value)
    {
      return setSlot(libport::Symbol(slot), value);
    }

    rObject
    Object::setSlotValue(const std::string& slot, const rObject& value)
    {
      slot_set_value(libport::Symbol(slot), value);
      return value;
    }

    rObject
    Object::getSlotValue(const std::string& slot)
    {
      return slot_get_value(libport::Symbol(slot));
    }

    rObject
    Object::urbi_setConstSlotValue(key_type name, const rObject& value)
    {
      slot_set_value(name, value, true);
      return value;
    }

    rObject
    Object::urbi_updateSlot(key_type name, const rObject& value)
    {
      return slot_update(name, value);
    }

    rSlot
    Object::findSlot(const std::string& str)
    {
      libport::Symbol k(str);
      object::Object::location_type loc;
      loc = slot_locate(k, false);
      rObject tgt(this);
      if (!loc.first)
        loc = eval::import_stack_lookup(
          ::kernel::urbiserver->getCurrentRunner().state,
          k, tgt, false);
      if (!loc.first)
        loc = slot_locate(k, true);
      if (!loc.first)
        runner::raise_lookup_error(k, this);
      if (rSlot s = loc.second->as<Slot>())
        return s;
      // Convert it to a slot.
      rSlot s(new Slot(loc.second));
      slots_.set(loc.first, k, s, true);
      return s;
    }

    rObject
    Object::asPrintable() const
    {
      return asString();
    }

    rObject
    Object::asTopLevelPrintable() const
    {
      return const_cast<Object*>(this)->call(SYMBOL(asPrintable));
    }

    rHash
    Object::hash() const
    {
      std::size_t h = boost::hash_value(this);
      Hash* res = new Hash(h);
      return res;
    }

    rObject
    Object::addProto(rObject proto)
    {
      assert(proto);
      // Inheriting from atoms is a problem: we cannot morph in place
      // the C++ object to give him the right primitive type. For now,
      // we forbid inheriting from atoms.
      if (!proto->valid_proto(*this))
        FRAISE("cannot inherit from a %1% without being one",
               proto->type_name_get());
      proto_add(proto);
      return this;
    }

    rObject
    Object::removeProto(rObject proto)
    {
      proto_remove(proto);
      return this;
    }

#define CHECK_ARG(N)				\
    if (!arg ## N)				\
      goto done;                                \
    args.push_back(arg ## N)

    rObject
    Object::call(libport::Symbol name,
                 rObject arg1,
                 rObject arg2,
                 rObject arg3,
                 rObject arg4,
                 rObject arg5)
    {
      objects_type args;
      args.push_back(this);
      CHECK_ARG(1);
      CHECK_ARG(2);
      CHECK_ARG(3);
      CHECK_ARG(4);
      CHECK_ARG(5);
      done:

      return call_with_this(name, args);
    }

#undef CHECK_ARG

    rObject
    Object::call(libport::Symbol name,
                 const objects_type& args)
    {
      objects_type actual_args;
      actual_args << this;
      foreach (const object::rObject& arg, args)
        actual_args << arg;
      return call_with_this(name, actual_args);
    }

    rObject
    Object::call_with_this(libport::Symbol name,
                           const objects_type& args)
    {
      rObject fun = slot_get_value(name);
      return eval::call_apply(::kernel::runner(), fun, name, args);
    }

    rObject
    Object::call(const std::string& name,
                 rObject arg1,
                 rObject arg2,
                 rObject arg3,
                 rObject arg4,
                 rObject arg5)
    {
      return call(libport::Symbol(name), arg1, arg2, arg3, arg4, arg5);
    }

    std::ostream& operator<<(std::ostream& s, const Object& o)
    {
      return o.print(s);
    }

    void
    Object::bind_variadic(const std::string& name,
                          const boost::function1<rObject, const objects_type&>& val)
    {
      rObject r(new Primitive(val));
      setSlot(libport::Symbol(name), r);
    }

    void*
    Object::as_dispatch_(const std::type_info* requested)
    {
      return this->as_check_(requested) ? this : 0;
    }

    std::string
    Object::uid() const
    {
      static boost::format uid("0x%x");
      return str(uid % reinterpret_cast<size_t>(this));
    }

    rObject
    Object::new_(const objects_type& args)
    {
      rObject res = call(SYMBOL(clone));
      if (rObject init = slot_get_value(SYMBOL(init), false))
        eval::call_apply(::kernel::runner(), res, init,
                         SYMBOL(init), args);
      return res;
    }

    rObject
    Object::package_root_get()
    {
      static rObject res = 0;
      if (!res)
      {
        res = Object::proto->clone();
        res->slot_set_value(SYMBOL(package), res);
        res->slot_set_value(SYMBOL(lang), Object::proto->clone());
        //Object::proto->slot_set_value(SYMBOL(Package), res);
      }
      return res;
    }

    rObject
    Object::package_lang_get()
    {
      static rObject res = 0;
      if (!res)
      {
        res = package_root_get()->slot_get_value(SYMBOL(lang));
        res->slot_set_value(SYMBOL(package), package_root_get());
      }
      return res;
    }

    rObject Object::proto;
  } // namespace object
}
