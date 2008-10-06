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

#include <object/cxx-primitive.hh>
#include <object/float.hh>
#include <object/global.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/object.hh>
#include <object/string.hh>

#include <runner/call.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>

namespace object
{
  rObject object_class;

  /*--------------------.
  | Object primitives.  |
  `--------------------*/

  static rObject
  object_class_clone (runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 0);

    return args[0]->clone();
  }

  static rObject
  object_class_init (runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 0);
    return args[0];
  }

  /// Send dumped self on the connection.
  /// args[1], if present, can be the tag to use.
  static rObject
  object_class_dump (runner::Runner& r, objects_type& args)
  {
    check_arg_count(args.size() - 1, 1, 2);

    // Second argument is max depth.
    int depth_max = 0;
    if (args.size() >= 2)
    {
      type_check(args[1], Float::proto);
      const rFloat& arg1 = args[1]->as<Float>();
      try
      {
	depth_max = libport::ufloat_to_int(arg1->value_get());
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
      const rString& arg2 = args[2].unsafe_cast<String>();
      assert(arg2);
      tag = arg2->value_get();
    }

    std::ostringstream os;
    args[0]->dump(os, r, depth_max);
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
  object_class_uid (runner::Runner&, objects_type& args)
  {
    static boost::format uid("0x%x");
    check_arg_count(args.size() - 1, 0);
    return
      new String(str(uid % reinterpret_cast<long long>(args[0].get())));
  }

  /// Structural equality
  static rObject
  object_class_EQ_EQ(runner::Runner& r, objects_type& args)
  {
    // Unless overridden, structural equality is physical equality.
    check_arg_count(args.size() - 1, 1);
    return urbi_call(r, args[0], SYMBOL(EQ_EQ_EQ), args[1]);
  }

  /// Physical equality
  static rObject
  object_class_EQ_EQ_EQ(runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 1);
    return to_boolean(args[0] == args[1]);
  }

  static rObject
  object_class_apply(runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 1);
    type_check(args[1], List::proto);
    const rList& arg1 = args[1]->as<List>();
    if (arg1->value_get ().size () != 1
        || arg1->value_get().front() != args[0])
      runner::raise_primitive_error("first argument must be `[this]'");
    return arg1->value_get().front();
  }

  static rObject
  object_class_callMessage (runner::Runner& r, objects_type& args)
  {
    check_arg_count(args.size() - 1, 1);
    // We need to set the 'code' slot: make a copy of the call message.
    rObject call_message = args[1]->clone();
    const rObject& message = call_message->slot_get(SYMBOL(message));
    type_check(message, String::proto);
    const libport::Symbol msg(message->as<String>()->value_get());
    const rObject& target = args[0];
    const rObject& code = target->slot_get(msg);
    call_message->slot_update(r, SYMBOL(code), code);
    call_message->slot_update(r, SYMBOL(target), target);
    // FIXME: Sanity checks on the call message are probably required
    return r.apply_call_message(code, msg, call_message);
  }

  /*---------.
  | Protos.  |
  `---------*/

  /// Adding or removing protos. \a Verb is "add" or "remove".
#define CHANGE_PARENTS(Verb)                                            \
  static rObject                                                        \
  object_class_ ## Verb ## Proto (runner::Runner&,                      \
                                  objects_type& args)                   \
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
  object_class_protos (runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 0);
    return args[0]->urbi_protos_get ();
  }

  /// Recursively get protos list

  static bool
  proto_add(List::value_type& protos, const rObject& proto)
  {
    protos.push_back(proto);
    return false;
  }

  static rObject
  object_class_allProtos(runner::Runner&, objects_type& args)
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
  object_class_slotNames (runner::Runner&, objects_type& args)
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
  object_class_allSlotNames(runner::Runner& r, objects_type& args)
  {
    check_arg_count(args.size() - 1, 0);
    std::vector<libport::Symbol> slot_names;
    List::value_type res;
    objects_type protos =
      object_class_allProtos(r, args)->as<List>()->value_get();
    foreach (const rObject& proto, protos)
    {
      for_all_slot_names(proto, boost::bind(&maybe_add,
					    boost::ref(slot_names),
					    boost::ref(res),
					    _1));
    }
    return new List(res);
  }

  /// Get a slot content.
  static rObject
  object_class_getSlot (runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 1);
    const rObject& obj = args[0];
    const rString& arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    return obj->slot_get(libport::Symbol(arg1->value_get()));
  }

  // self.getLazyLocalSlot(SLOT-NAME, DEFAULT-VALUE, CREATE?).
  static rObject
  object_class_getLazyLocalSlot (runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 3);

    const rString& arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    const Object::key_type& slot_name = libport::Symbol(arg1->value_get());

    // If the slot already exists, return its content.
    if (rObject slot = args[0]->own_slot_get (slot_name))
      return slot;

    // The slot doesn't exist. Should we create it?
    if (is_true (args[3]))
      args[0]->slot_set (slot_name, args[2]);

    // Return the default value for this slot.
    return args[2];
  }

  /// Remove a slot.
  static rObject
  object_class_removeSlot (runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 1);
    const rObject& obj = args[0];
    const rString& arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    obj->slot_remove(libport::Symbol(arg1->value_get()));
    return obj;
  }

   /// Locate a slot.
  static rObject
  object_class_locateSlot (runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 1);
    const rString& arg1 = args[1].unsafe_cast<String>();
    assert(arg1);

    rObject o = args[0]->slot_locate(libport::Symbol(arg1->value_get()));
    return o ? o : nil_class;
  }

  static rObject
  object_class_setSlot (runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 2);
    const rString& arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    args[0]->slot_set(libport::Symbol(arg1->value_get()), args[2]);
    return args[2];
  }

  static rObject
  object_class_createSlot (runner::Runner&, objects_type& args)
  {
    check_arg_count(args.size() - 1, 1);
    const rString& arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    args[0]->slot_set(libport::Symbol(arg1->value_get()), void_class);
    return void_class;
  }

  static rObject
  object_class_updateSlot (runner::Runner& r, objects_type& args)
  {
    check_arg_count(args.size() - 1, 2);
    const rString& arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    return args[0]->slot_update(r, libport::Symbol(arg1->value_get()), args[2]);
  }

  static bool
  object_class_isA(rObject self, rObject proto)
  {
    return is_a(self, proto);
  }

  void
  object_class_initialize ()
  {
    object_class->slot_set(SYMBOL(isA),
                           make_primitive(object_class_isA, SYMBOL(isA)));

    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(object, Name)

    DECLARE(addProto);
    DECLARE(allProtos);
    DECLARE(allSlotNames);
    DECLARE(apply);
    DECLARE(callMessage);
    DECLARE(clone);
    DECLARE(dump);
    DECLARE(getLazyLocalSlot);
    DECLARE(getSlot);
    DECLARE(init);
    DECLARE(locateSlot);
    DECLARE(EQ_EQ_EQ);
    DECLARE(protos);
    DECLARE(removeProto);
    DECLARE(removeSlot);
    DECLARE(EQ_EQ);
    DECLARE(setSlot);
    DECLARE(createSlot);
    DECLARE(slotNames);
    DECLARE(uid);
    DECLARE(updateSlot);
#undef DECLARE

#define DECLARE(Name, Code)                     \
    object_class->slot_set(SYMBOL(Name), make_primitive(Code, SYMBOL(Name)))

    DECLARE(getProperty, &Object::property_get);
    DECLARE(hasProperty, &Object::property_has);
    DECLARE(hasSlot, &Object::slot_has);
    DECLARE(setProperty, &Object::property_set);
    DECLARE(removeProperty, &Object::property_remove);
#undef DECLARE
  }

}; // namespace object
