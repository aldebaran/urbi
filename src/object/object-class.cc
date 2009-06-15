/**
 ** \file object/object-class.cc
 ** \brief Creation of the URBI object object.
 */

//#define ENABLE_DEBUG_TRACES

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include <libport/compiler.hh>
#include <libport/containers.hh>
#include <libport/escape.hh>
#include <libport/foreach.hh>
#include <libport/tokenizer.hh>

#include <kernel/uconnection.hh>
#include <kernel/userver.hh>
#include <object/cxx-primitive.hh>
#include <object/float.hh>
#include <object/global.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/string.hh>
#include <object/symbols.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>

namespace object
{
  /*--------------------.
  | Object primitives.  |
  `--------------------*/

  static rObject
  object_class_clone (const objects_type& args)
  {
    check_arg_count(args.size() - 1, 0);

    return args[0]->clone();
  }

  static rObject
  object_class_init (const objects_type& args)
  {
    check_arg_count(args.size() - 1, 0);
    return args[0];
  }

  /// Send dumped self on the connection.
  /// args[1], if present, can be the tag to use.
  static rObject
  object_class_dump (const objects_type& args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    check_arg_count(args.size() - 1, 1, 2);

    // Second argument is max depth.
    int depth_max = 0;
    if (args.size() >= 2)
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
    if (args.size() >= 3)
    {
      type_check<String>(args[2]);
      const rString& arg2 = args[2].unsafe_cast<String>();
      assert(arg2);
      tag = arg2->value_get();
    }

    std::ostringstream os;
    args[0]->dump(os, depth_max);
    //for now our best choice is to dump line by line in "system" messages.
    const std::string stream = os.str();
    boost::tokenizer< boost::char_separator<char> > tok =
      libport::make_tokenizer(stream, "\n");
    const std::string system_header("*** ");
    foreach(const std::string& line, tok)
      r.send_message(tag, system_header+line);
    return void_class;
  }

  /// Return the address of an object as a number, mostly
  /// for debugging purpose.
  static rObject
  object_class_uid (const objects_type& args)
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
    if (arg1->value_get ().size () != 1
        || arg1->value_get().front() != args[0])
      RAISE("first argument must be `[this]'");
    return arg1->value_get().front();
  }

  static rObject
  object_class_callMessage (const objects_type& args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

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
    return r.apply_call_message(code, msg, call_message);
  }

  /*---------.
  | Protos.  |
  `---------*/

  /// Adding or removing protos. \a Verb is "add" or "remove".
#define CHANGE_PARENTS(Verb)                                            \
  static rObject                                                        \
  object_class_ ## Verb ## Proto (                     \
                                  const objects_type& args)                   \
  {									\
    check_arg_count(args.size() - 1, 1);                                \
    args[0]->proto_ ## Verb (args[1]);					\
    return args[0];							\
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
  object_class_slotNames (const objects_type& args)
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
  object_class_allSlotNames(const objects_type& args)
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

  void
  object_class_initialize ()
  {
    Object::proto->slot_set(SYMBOL(isA),
                           make_primitive(object_class_isA));
    Object::proto->slot_set(SYMBOL(hasLocalSlot),
                            make_primitive(&object_class_hasLocalSlot));
    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)                                                   \
    Object::proto->slot_set(SYMBOL(Name), new Primitive(object_class_##Name), true)

    DECLARE(addProto);
    DECLARE(allProtos);
    DECLARE(allSlotNames);
    DECLARE(apply);
    DECLARE(callMessage);
    DECLARE(clone);
    DECLARE(dump);
    DECLARE(init);
    DECLARE(EQ_EQ_EQ);
    DECLARE(protos);
    DECLARE(removeProto);
    DECLARE(EQ_EQ);
    DECLARE(slotNames);
    DECLARE(uid);
#undef DECLARE

#define DECLARE(Name, Code)                                     \
    Object::proto->slot_set(SYMBOL(Name), make_primitive(Code))

    DECLARE(createSlot, &Object::urbi_createSlot);
    DECLARE(getProperty, &Object::property_get);
    DECLARE(getSlot, &Object::urbi_getSlot);
    DECLARE(hasProperty, &Object::property_has);
    DECLARE(hasSlot, &Object::slot_has);
    DECLARE(locateSlot, &Object::urbi_locateSlot);
    DECLARE(setProperty, &Object::property_set);
    DECLARE(setSlot, &Object::urbi_setSlot);
    DECLARE(setConstSlot, &Object::urbi_setConstSlot);
    DECLARE(removeProperty, &Object::property_remove);
    DECLARE(removeSlot, &Object::urbi_removeSlot);
    DECLARE(updateSlot, &Object::urbi_updateSlot);
#undef DECLARE
  }

}; // namespace object
