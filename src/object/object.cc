/*
 * Copyright (C) 2007-2010, Gostai S.A.S.
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

#include <kernel/userver.hh>

#include <urbi/object/cxx-conversions.hh>
#include <urbi/object/dictionary.hh>
#include <urbi/object/event.hh>
#include <urbi/object/float.hh>
#include <urbi/object/global.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>
#include <object/root-classes.hh>
#include <object/symbols.hh>
#include <object/urbi-exception.hh>

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
    }


    Object::Object()
      : protos_(new protos_type)
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

      rList protos = new List(*protos_);
      protos_cache_ = protos;
      delete protos_;
      protos_ = &protos->value_get();
      return protos;
    }

    void
    Object::protos_set(const rList& l)
    {
      if (!protos_cache_)
        delete protos_;
      protos_cache_ = l;
      protos_ = &l->value_get();
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
      foreach (const rObject& proto, protos_get())
      {
        location_type rec = proto->slot_locate_(k, fallback);
        if (rec.first)
          return rec;
      }
      if (fallback)
        if (rSlot slot = local_slot_get(SYMBOL(fallback)))
          return location_type(const_cast<Object*>(this), slot);
      return location_type(0, 0);
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

    bool squash = false;
    Slot&
    Object::slot_get(key_type k)
    {
      rSlot res = safe_slot_locate(k).second;
      if (runner::Runner* r = ::kernel::urbiserver->getCurrentRunnerOpt())
      {
        if (!squash && r->dependencies_log_get())
        {
          bool prev = squash;
          FINALLY(((bool&, squash))((bool, prev)), squash = prev);
          squash = true;
          // I think this registration is useless, since the parent is
          // already registered and will trigger its 'changed'
          // event. Not absolutely sure for all cases though, so I let
          // it live for a while, in case some at start not being
          // triggered.
          // GD_FPUSH_TRACE("Register slot '%s' for at monitoring", k);
          // r->dependency_add(static_cast<Event*>(res->property_get(SYMBOL(changed)).get()));
          rObject changed = (*res)->call(SYMBOL(changed));
          assert(changed);
          rEvent evt = changed->as<Event>();
          assert(evt);
          r->dependency_add(evt.get());
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

    static bool redefinition_mode()
    {
      runner::Runner* r = ::kernel::urbiserver->getCurrentRunnerOpt();
      return r && r->redefinition_mode_get();
    }

    Object&
    Object::slot_set(key_type k, Slot* o)
    {
      if (!slots_.set(this, k, o, redefinition_mode()))
        runner::raise_urbi_skip(SYMBOL(Redefinition), to_urbi(k));
      changed();
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
      runner::Runner& r = ::kernel::runner();

      // The updated slot
      Slot& s = slot_get(k);
      // Its owner
      rObject owner = slot_locate(k).first;
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
          v = r.apply(hook, SYMBOL(updateHook), args);
          // If the updateHook returned void, do nothing. Otherwise let
          // the slot be overwritten.
          if (v == object::void_class)
            return o;
        }
      // If return-value of hook is not void, write it to slot.
      if (owner == this)
      {
        bool prev = squash;
        FINALLY(((bool&, squash))((bool, prev)), squash = prev);
        squash = true;
        s = v;
      }
      else
      {
        // Here comes the cow
        Slot* slot = new Slot(s);
        *slot = v;
        slot_set(k, slot);
      }
      changed();
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
        res[slot->first.second] = properties_get(slot->first.second);
      return new Dictionary(res);
    }

    rDictionary
    Object::properties_get(key_type k)
    {
      if (Slot::properties_type* props = slot_get(k).properties_get())
        return new Dictionary(*props);
      else
        return new Dictionary();
    }

    rObject
    Object::property_get(key_type k, key_type p)
    {
      return slot_get(k).property_get(p);
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
        slot_set(k, slot_get(k));
      Slot& slot = slot_get(k);
      if (slot.property_set(p, value))
        if (slot->slot_has(SYMBOL(newPropertyHook)))
          slot->call(SYMBOL(newPropertyHook),
                     this, new String(k), new String(p), value);
      return value;
    }

    rObject
    Object::property_remove(key_type k, key_type p)
    {
      Slot& slot = slot_get(k);
      rObject res = slot.property_get(p);
      slot.property_remove(p);
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
      type_check<String>(data);
      return o << data->as<String>()->value_get();
    }


    std::ostream&
    Object::protos_dump(std::ostream& o) const
    {
      if (!protos_->empty())
      {
        o << "protos = ";
        bool tail = false;
        foreach (const rObject& p, *protos_)
        {
          if (tail++)
            o << ", ";
          p->id_dump (o);
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
      type_check<String>(s);
      return s->as<String>();
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
      // Inheriting from atoms is a problem: we cannot morph in place
      // the C++ object to give him the right primitive type. For now,
      // we forbid inheriting from atoms.
      if (!p->valid_proto(*this))
      {
        boost::format fmt("cannot inherit from a %1% without being a %1% too");
        runner::raise_primitive_error((fmt % p->type_name_get()).str());
      }

      if (!libport::has(*protos_, p))
        push_front (*protos_, p);
      return *this;
    }

    std::string
    Object::type_name_get() const
    {
      return "Object";
    }

    /*------------------.
    | 'changed' event.  |
    `------------------*/

    void
    Object::changed()
    {
      if (changed_)
        changed_->emit();
    }

    const rEvent&
    Object::changed_get() const
    {
      if (!changed_)
        const_cast<Object*>(this)->changed_ = new Event;
      return changed_;
    }

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
    Object::urbi_removeSlot(key_type name)
    {
      slot_remove(name);
      return this;
    }

    rObject
    Object::setProperty(const std::string& slot, const std::string& prop, const rObject& value)
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
      runner::Runner& r = ::kernel::runner();
      rObject fun = slot_get(name);
      return r.apply(fun, name, args);
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

    std::ostream& operator<< (std::ostream& s, const Object& o)
    {
      return o.print(s);
    }

    rObject Object::proto;

  } // namespace object
}
