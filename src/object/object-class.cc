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
 ** \file urbi/object/object.class.cc
 ** \brief Creation of the Urbi object object.
 */

//#define ENABLE_DEBUG_TRACES

#include <libport/bind.hh>
#include <libport/format.hh>

#include <libport/compiler.hh>
#include <libport/containers.hh>
#include <libport/escape.hh>
#include <libport/foreach.hh>
#include <libport/tokenizer.hh>

#include <kernel/uconnection.hh>
#include <kernel/userver.hh>
#include <object/object-class.hh>
#include <object/symbols.hh>
#include <runner/runner.hh>
#include <urbi/object/cxx-primitive.hh>
#include <urbi/object/dictionary.hh>
#include <urbi/object/event.hh>
#include <urbi/object/float.hh>
#include <urbi/object/global.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {
    /*--------------------.
    | Object primitives.  |
    `--------------------*/

    static rObject
    object_class_clone(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 0);
      return args[0]->clone();
    }

    static rObject
    object_class_init(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 0);
      return args[0];
    }

    /// Send dumped self on the connection.
    /// args[1], if present, can be the tag to use.
    static rObject
    object_class_dump(const objects_type& args)
    {
      runner::Runner& r = ::kernel::runner();

      check_arg_count(args.size() - 1, 1, 2);

      // Second argument is max depth.
      int depth_max = 0;
      if (2 <= args.size())
      {
        type_check<Float>(args[1]);
        const rFloat& arg1 = args[1]->as<Float>();
        try
        {
          depth_max = libport::numeric_cast<int>(arg1->value_get());
        }
        catch (libport::bad_numeric_cast& ue)
        {
          runner::raise_bad_integer_error(arg1->value_get());
        }
      }

      // Third argument is the tag name.
      std::string tag;
      if (3 <= args.size())
      {
        type_check<String>(args[2]);
        const rString& arg2 = args[2].unsafe_cast<String>();
        aver(arg2);
        tag = arg2->value_get();
      }

      // Do not use libport::lines on os.str().
      std::string dump;
      {
        std::ostringstream os;
        args[0]->dump(os, depth_max);
        dump = os.str();
      }
      // For now our best choice is to dump line by line in "system" messages.
      const std::string system_header("*** ");
      foreach (const std::string& line, libport::lines(dump))
        r.send_message(tag, system_header + line);
      return void_class;
    }

    /// Return the address of an object as a number, mostly
    /// for debugging purpose.
    static rObject
    object_class_uid(const objects_type& args)
    {
      static boost::format uid("0x%x");
      check_arg_count(args.size() - 1, 0);
      return
        new String(str(uid % reinterpret_cast<long long>(args[0].get())));
    }

    /// Structural equality
    static rObject
    object_class_EQ_EQ(const objects_type& args)
    {
      // Unless overridden, structural equality is physical equality.
      check_arg_count(args.size() - 1, 1);
      return args[0]->call(SYMBOL(EQ_EQ_EQ), args[1]);
    }

    /// Physical equality
    static rObject
    object_class_EQ_EQ_EQ(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 1);
      return to_boolean(args[0] == args[1]);
    }

    static rObject
    object_class_apply(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 1);
      type_check<List>(args[1]);
      const rList& arg1 = args[1]->as<List>();
      unsigned nargs = arg1->value_get().size();
      if (nargs != 1)
        runner::raise_arity_error(nargs, 1);
      return args[0];
    }

    static rObject
    object_class_callMessage (const objects_type& args)
    {
      runner::Runner& r = ::kernel::runner();

      check_arg_count(args.size() - 1, 1);
      // We need to set the 'code' slot: make a copy of the call message.
      rObject call_message = args[1]->clone();
      const rObject& message = call_message->slot_get(SYMBOL(message));
      type_check<String>(message);
      const libport::Symbol msg(message->as<String>()->value_get());
      const rObject& target = args[0];
      const rObject& code = target->slot_get(msg);
      call_message->slot_update(SYMBOL(code), code);
      call_message->slot_update(SYMBOL(target), target);
      // FIXME: Sanity checks on the call message are probably required
      return r.apply_call_message(code, msg, call_message, LOCATION_HERE);
    }

    /*---------.
    | Protos.  |
    `---------*/

    /// Adding or removing protos. \a Verb is "add" or "remove".
#define CHANGE_PARENTS(Verb)                                    \
    static rObject                                              \
    object_class_ ## Verb ## Proto (const objects_type& args)   \
    {                                                           \
      check_arg_count(args.size() - 1, 1);                      \
      args[0]->proto_ ## Verb (args[1]);                        \
      return args[0];                                           \
    }

    /// Add a proto.
    CHANGE_PARENTS(add);
    /// Remove a proto.
    CHANGE_PARENTS(remove);
#undef CHANGE_PARENTS

    /// Get protos' list.
    static rObject
    object_class_protos (const objects_type& args)
    {
      check_arg_count(args.size() - 1, 0);
      return args[0]->urbi_protos_get ();
    }

    /// Add a prototype
    static bool
    proto_add(List::value_type& protos, const rObject& proto)
    {
      protos.push_back(proto);
      return false;
    }

    /// Recursively get protos list
    static rObject
    object_class_allProtos(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 0);
      List::value_type res;
      for_all_protos(args[0], boost::bind(&proto_add, boost::ref(res), _1));
      return new List(res);
    }


    /*--------.
    | Slots.  |
    `--------*/

    template <typename F>
    static inline void for_all_slot_names(const rObject& o, F f)
    {
      for (Object::slots_implem::iterator slot = o->slots_get().begin(o.get());
           slot != o->slots_get().end(o.get());
           ++slot)
        f(slot->first.second);
    }

    static void
    add_as_rString(List::value_type& l, libport::Symbol slot_name)
    {
      l.push_back(new String(slot_name));
    }

    /// List of slot names.
    static rObject
    object_class_localSlotNames(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 0);
      const rObject& obj = args[0];

      List::value_type l;
      for_all_slot_names(obj, boost::bind(&add_as_rString, boost::ref(l), _1));
      return new List(l);
    }

    /// Recursive list of slot names.
    static void
    maybe_add(std::vector<libport::Symbol>& control, List::value_type& l,
              libport::Symbol slot_name)
    {
      if (!libport::has(control, slot_name))
      {
        control.push_back(slot_name);
        l.push_back(new String(slot_name));
      }
    }

    static rObject
    object_class_slotNames(const objects_type& args)
    {
      check_arg_count(args.size() - 1, 0);
      std::vector<libport::Symbol> slot_names;
      List::value_type res;
      objects_type protos =
        object_class_allProtos(args)->as<List>()->value_get();
      foreach (const rObject& proto, protos)
      {
        for_all_slot_names(proto, boost::bind(&maybe_add,
                                              boost::ref(slot_names),
                                              boost::ref(res),
                                              _1));
      }
      return new List(res);
    }

    static bool
    object_class_isA(rObject self, rObject proto)
    {
      return is_a(self, proto);
    }

    static bool
    object_class_hasLocalSlot(rObject self, const libport::Symbol& slot)
    {
      return self->local_slot_get(slot);
    }

#define OPERATOR(Name)                                          \
    static rObject                                              \
    object_class_ ## Name ## _EQ(rObject lhs, rObject rhs)      \
    {                                                           \
      return lhs->call(SYMBOL(Name), rhs);                      \
    }

    OPERATOR(CARET)
    OPERATOR(MINUS)
    OPERATOR(PERCENT)
    OPERATOR(PLUS)
    OPERATOR(SLASH)
    OPERATOR(STAR)
#undef OPERATOR

    static rObject
    object_class_PLUS_PLUS(rObject lhs)
    {
      static rObject one = to_urbi(1);
      return lhs->call(SYMBOL(PLUS_EQ), one);
    }

    static rObject
    object_class_MINUS_MINUS(rObject lhs)
    {
      static rObject one = to_urbi(1);
      return lhs->call(SYMBOL(MINUS_EQ), one);
    }

    void
    object_class_initialize()
    {
      Object::proto->slot_set(SYMBOL(isA),
                              make_primitive(object_class_isA));
      Object::proto->slot_set(SYMBOL(hasLocalSlot),
                              make_primitive(&object_class_hasLocalSlot));

#define DECLARE(Name)                                                   \
      Object::proto->slot_set(SYMBOL(Name),                             \
                              make_primitive(object_class_##Name),       \
                              true)

      DECLARE(EQ_EQ);
      DECLARE(EQ_EQ_EQ);
      DECLARE(addProto);
      DECLARE(allProtos);
      DECLARE(apply);
      DECLARE(callMessage);
      DECLARE(clone);
      DECLARE(dump);
      DECLARE(init);
      DECLARE(localSlotNames);
      DECLARE(protos);
      DECLARE(removeProto);
      DECLARE(slotNames);
      DECLARE(uid);
      DECLARE(PLUS_EQ);
      DECLARE(MINUS_EQ);
      DECLARE(CARET_EQ);
      DECLARE(PERCENT_EQ);
      DECLARE(STAR_EQ);
      DECLARE(SLASH_EQ);
      DECLARE(PLUS_PLUS);
      DECLARE(MINUS_MINUS);
#undef DECLARE

#define DECLARE(Name, Code)                                             \
      Object::proto->slot_set(SYMBOL(Name), make_primitive(Code))

      DECLARE(asBool             , &Object::as_bool);
      DECLARE(asPrintable        , &Object::asPrintable);
      DECLARE(asToplevelPrintable, &Object::asToplevelPrintable);
      DECLARE(changed            , &Object::changed_get);
      DECLARE(createSlot         , &Object::urbi_createSlot);
      DECLARE(getLocalSlot       , &Object::getLocalSlot);
      DECLARE(getProperty        , &Object::property_get);
      typedef rObject (Object::*get_slot_type)(Object::key_type);
      DECLARE(getSlot            ,
              static_cast<get_slot_type>(&Object::getSlot));
      DECLARE(hasProperty        , &Object::property_has);
      DECLARE(hasSlot            , &Object::slot_has);
      DECLARE(locateSlot         , &Object::urbi_locateSlot);
      DECLARE(properties         , &Object::urbi_properties);
      DECLARE(removeProperty     , &Object::property_remove);
      DECLARE(removeSlot         , &Object::urbi_removeSlot);
      DECLARE(setConstSlot       , &Object::urbi_setConstSlot);
      DECLARE(setProperty        , &Object::property_set);
      typedef rObject (Object::*set_slot_type)(Object::key_type,
                                               const rObject&);
      DECLARE(setSlot            ,
              static_cast<set_slot_type>(&Object::setSlot));
      DECLARE(updateSlot         , &Object::urbi_updateSlot);
#undef DECLARE
    }
  }; // namespace object
}
