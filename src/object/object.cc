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
#include <runner/runner.hh>

GD_CATEGORY(Urbi);

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
        runner::Runner& r = ::kernel::runner();
        r.send_message("warning", libport::format("!!! %s", msg));
        r.show_backtrace("warning");
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
    Object::slot_locate_(key_type k, bool fallback) const
    {
      if (lookup_id_ == lookup_id)
        return location_type(0, 0);
      lookup_id_ = lookup_id;
      if (rSlot slot = local_slot_get(k))
        return location_type(const_cast<Object*>(this), slot);
      if (proto_)
      {
        location_type rec = proto_->slot_locate_(k, fallback);
        if (rec.first)
          return rec;
      }
      else if (protos_) // Braces to pacify G++.
      {
        foreach (const rObject& proto, *protos_)
        {
          location_type rec = proto->slot_locate_(k, fallback);
          if (rec.first)
            return rec;
        }
      }

      if (fallback)
        if (rSlot slot = local_slot_get(SYMBOL(fallback)))
          return location_type(const_cast<Object*>(this), slot);
      return location_type(0, 0);
    }

    rSlot
    Object::local_slot_get(key_type k) const
    {
      rSlot res = slots_.get(this, k);
      if (res)
        URBI_AT_HOOK(slotRemoved);
      else
        URBI_AT_HOOK(slotAdded);
      return res;
    }

    Object::location_type
    Object::slot_locate(key_type k,
                        bool fallback) const
    {
      ++lookup_id;
      return slot_locate_(k, fallback);
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
    Object::slot_get(key_type k) const
    {
      return const_cast<Object*>(this)->slot_get(k);
    }

    Slot&
    Object::slot_get(key_type k)
    {
      rSlot res = safe_slot_locate(k).second;
      if (runner::Runner* r = ::kernel::urbiserver->getCurrentRunnerOpt())
      {
        if (r->dependencies_log_get())
        {
          GD_CATEGORY(Urbi.At);
          GD_FPUSH_TRACE("Register slot '%s' for at monitoring", k);
          Event* e;
          {
            FINALLY(((runner::Runner*, r)), r->dependencies_log_set(true));
            r->dependencies_log_set(false);
            e = static_cast<Event*>
              (res->property_get(SYMBOL(changed)).get());
          }
          r->dependency_add(e);
        }
      }
      return *res;
    }


    Object&
    Object::slot_set(key_type k, rObject o, bool constant)
    {
      Slot* slot = new Slot(o);
      slot->constant_set(constant);
      return slot_set(k, slot);
    }

    namespace
    {
      inline
      bool redefinition_mode()
      {
        runner::Runner* r = ::kernel::urbiserver->getCurrentRunnerOpt();
        return r && r->redefinition_mode_get();
      }
    }

    Object&
    Object::slot_set(key_type k, Slot* o)
    {
      if (!slots_.set(this, k, o, redefinition_mode()))
      {
        GD_CATEGORY(Urbi.Error);
        GD_FINFO_DEBUG("Slot redefinition: %s", k);
        runner::raise_urbi_skip(SYMBOL(Redefinition), to_urbi(k));
      }
      slotAdded();
      return *this;
    }

    Object&
    Object::slot_copy(key_type name, const rObject& from)
    {
      this->slot_set(name, from->slot_get(name), redefinition_mode());
      return *this;
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
      // The updated slot
      Slot& s = slot_get(k);
      // Value to write to the slot
      rObject v = o;

      // If the current value in the slot to be written in has a slot
      // named 'updateHook', call it, passing the object owning the
      // slot, the slot name and the target.
      if (hook)
        if (rObject hook = s.property_get(SYMBOL(updateHook)))
        {
          objects_type args;
          args << rObject(this)
               << new String(k)
               << o;
          v = ::kernel::runner().apply(hook, SYMBOL(updateHook), args);
          // If the updateHook returned void, do nothing. Otherwise let
          // the slot be overwritten.
          if (v == object::void_class)
            return o;
        }

      // If return-value of hook is not void, write it to slot.
      if (slot_locate(k).first == this)
      {
        if (runner::Runner* r = ::kernel::urbiserver->getCurrentRunnerOpt())
        {
          bool d = r->dependencies_log_get();
          try
          {
            r->dependencies_log_set(false);
            s = v;
            r->dependencies_log_set(d);
          }
          catch (...)
          {
            r->dependencies_log_set(d);
            throw;
          }
        }
        else
          s = v;
      }
      else
      {
        // Here comes the cow
        Slot* slot = new Slot(s);
        *slot = v;
        slot_set(k, slot);
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
      if (Slot::properties_type* props = slot_get(k).properties_get())
      {
        Dictionary::value_type res;
        foreach (const Slot::properties_type::value_type& elt, *props)
          res.insert(std::make_pair(new String(elt.first), elt.second));
        return new Dictionary(res);
      }
      else
        return new Dictionary();
    }

    rObject
    Object::property_get(key_type slot, key_type prop)
    {
      if (rObject res = slot_get(slot).property_get(prop))
        return res;
      runner::raise_urbi_skip(SYMBOL(PropertyLookup),
                              this, to_urbi(slot), to_urbi(prop));
    }

    bool
    Object::property_has(key_type k, key_type p)
    {
      return slot_get(k).property_has(p);
    }

    rObject
    Object::property_set(key_type k,
                         key_type p,
                         const rObject& value)
    {
      // CoW
      if (safe_slot_locate(k).first != this)
        slot_set(k, &slot_get(k));
      Slot& slot = slot_get(k);
      if (slot.property_set(p, value)
          && slot->slot_has(SYMBOL(newPropertyHook)))
        slot->call(SYMBOL(newPropertyHook),
                   this, new String(k), new String(p), value);
      return value;
    }

    rObject
    Object::property_remove(key_type k, key_type p)
    {
      Slot& slot = slot_get(k);
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
                      const CentralizedSlots::q_slot_type& slot,
                      int depth_max) const
    {
      o << slot.first.second << " = ";
      if (slot.second->constant_get())
        o << "const ";
      slot.second->value()->dump(o, depth_max) << libport::iendl;

      Slot::properties_type props;
      if (Slot::properties_type* ps = slot.second->properties_get())
        props = *ps;
      props.erase(SYMBOL(constant));
      if (!props.empty())
      {
        o << "  /* Properties */" << libport::incendl;
        foreach (const Slot::properties_type::value_type& p, props)
        {
          o << p.first << " = ";
          p.second->dump(o, depth_max);
        }
        o << libport::decendl;
      }
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
      if (proto_ || !protos_->empty())
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
      special_slots_dump (o);

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
      passert(p,
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
      slot_set(name, void_class);
    }

    rObject
    Object::getProperty(const std::string& slot, const std::string& prop)
    {
      return property_get(libport::Symbol(slot), libport::Symbol(prop));
    }

    rObject
    Object::getSlot(key_type name)
    {
      return slot_get(name);
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
    Object::getLocalSlot(key_type name)
    {
      if (rSlot s = local_slot_get(name))
        return s->value();
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
      slot_set(slot, value);
      return value;
    }

    rObject
    Object::setSlot(const std::string& slot, const rObject& value)
    {
      return setSlot(libport::Symbol(slot), value);
    }

    rObject
    Object::urbi_setConstSlot(key_type name, const rObject& value)
    {
      slot_set(name, value, true);
      return value;
    }

    rObject
    Object::urbi_updateSlot(key_type name, const rObject& value)
    {
      return slot_update(name, value);
    }

    rObject
    Object::asPrintable() const
    {
      return asString();
    }

    rObject
    Object::asToplevelPrintable() const
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
      rObject fun = slot_get(name);
      return ::kernel::runner().apply(fun, name, args);
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
      setSlot(libport::Symbol(name), new Primitive(val));
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
      return str(uid % reinterpret_cast<long long>(this));
    }


    rObject Object::proto;
  } // namespace object
}
