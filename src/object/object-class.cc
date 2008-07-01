/**
 ** \file object/object-class.cc
 ** \brief Creation of the URBI object object.
 */

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>
#include <libport/escape.hh>
#include <libport/foreach.hh>
#include <libport/tokenizer.hh>

#include <kernel/uconnection.hh>

#include <runner/runner.hh>

#include <object/atom.hh>
#include <object/float-class.hh>
#include <object/global-class.hh>
#include <object/list-class.hh>
#include <object/object-class.hh>
#include <object/object.hh>
#include <object/string-class.hh>

namespace object
{
  rObject object_class;

  /*--------------------.
  | Object primitives.  |
  `--------------------*/

  static rObject
  object_class_clone (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return args[0]->clone();
  }

  static rObject
  object_class_init (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return args[0];
  }

  /// Send dumped self on the connection.
  /// args[1], if present, can be the tag to use.
  static rObject
  object_class_dump (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE(1, 3);

    // Second argument is max depth.
    int depth_max = 0;
    if (args.size() >= 2)
    {
      type_check<Float>(args[1], SYMBOL(dump));
      rFloat arg1 = args[1]->as<Float>();
      try
      {
	depth_max = libport::ufloat_to_int(arg1->value_get());
      }
      catch (libport::bad_numeric_cast& ue)
      {
	throw BadInteger(arg1->value_get(), SYMBOL(dump));
      }
    }

    // Third argument is the tag name.
    std::string tag;
    if (args.size() >= 3)
    {
      rString arg2 = args[2].unsafe_cast<String>();
      assert(arg2);
      tag = arg2->value_get().name_get();
    }

    std::ostringstream os;
    args[0]->dump(os, r, depth_max);
    //for now our best choice is to dump line by line in "system" messages.
    const std::string stream = os.str();
    boost::tokenizer< boost::char_separator<char> > tok =
      libport::make_tokenizer(stream, "\n");
    std::string system_header("*** ");
    foreach(const std::string& line, tok)
      r.send_message(tag, system_header+line);
    return void_class;
  }

  /// Return the address of an object as a number, mostly
  /// for debugging purpose.
  static rObject
  object_class_uid (runner::Runner&, objects_type args)
  {
    static boost::format uid("0x%x");
    CHECK_ARG_COUNT(1);
    return
      new String(libport::Symbol
                 (str(uid % reinterpret_cast<long long>(args[0].get()))));
  }

  /// Structural equality
  static rObject
  object_class_sameAs(runner::Runner& r, objects_type args)
  {
    // Unless overridden, structural equality is physical equality.
    CHECK_ARG_COUNT (2);
    return urbi_call(r, args[0], SYMBOL(memSameAs), args[1]);
  }

  /// Physical equality
  static rObject
  object_class_memSameAs(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    return to_boolean(args[0] == args[1]);
  }

  static rObject
  object_class_apply(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    type_check<List>(args[1], SYMBOL(apply));
    rList arg1 = args[1]->as<List>();
    if (arg1->value_get ().size () != 1 || arg1->value_get().front() != args[0])
      throw PrimitiveError(SYMBOL(apply), "first argument must be [self]");
    return arg1->value_get().front();
  }

  static rObject
  object_class_callMessage (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    rObject message = args[1]->slot_get(SYMBOL(message));
    type_check<String>(message, SYMBOL(callMessage));
    libport::Symbol msg = message->as<String>()->value_get();
    rObject code = args[0]->slot_get(msg);
    // FIXME: Sanity checks on the call message are probably required
    objects_type self;
    self.push_back(args[0]);
    return r.apply(code, msg, self, args[1]);
  }

  /*---------.
  | Protos.  |
  `---------*/

  /// Adding or removing protos. \a Verb is "add" or "remove".
#define CHANGE_PARENTS(Verb)						\
  static rObject							\
  object_class_ ## Verb ## Proto (runner::Runner&, objects_type args)	\
  {									\
    CHECK_ARG_COUNT(2);							\
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
  object_class_protos (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    return args[0]->urbi_protos_get ();
  }


  /*--------.
  | Slots.  |
  `--------*/


  /// List of slot names.
  static rObject
  object_class_slotNames (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    rObject obj = args[0];

    List::value_type l;
    foreach (const Slots::slot_type& p, obj->slots_get())
      l.push_back (new String(p.first));

    return new List(l);
  }

  /// Get a slot content.
  static rObject
  object_class_getSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    rObject obj = args[0];
    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    return obj->slot_get(arg1->value_get());
  }

  // self.getLazyLocalSlot(SLOT-NAME, DEFAULT-VALUE, CREATE?).
  static rObject
  object_class_getLazyLocalSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (4);

    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    Object::key_type slot_name = arg1->value_get ();

    // If the slot already exists, return its content.
    if (rObject slot = args[0]->own_slot_get (slot_name))
      return slot;

    // The slot doesn't exist. Should we create it?
    if (is_true (args[3], SYMBOL(getLazyLocalSlot)))
      args[0]->slot_set (slot_name, args[2]);

    // Return the default value for this slot.
    return args[2];
  }

  /// Remove a slot.
  static rObject
  object_class_removeSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    rObject obj = args[0];
    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    obj->slot_remove(arg1->value_get());
    return obj;
  }

   /// Locate a slot.
  static rObject
  object_class_locateSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);

    rObject o = args[0]->slot_locate(arg1->value_get());
    return o ? o : nil_class;
  }

  static rObject
  object_class_setSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(3);
    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    args[0]->slot_set(arg1->value_get (), args[2]);
    return args[2];
  }

  static rObject
  object_class_updateSlot (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT(3);
    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    args[0]->slot_update(r, arg1->value_get (), args[2]);
    return args[2];
  }

  static rObject
  object_class_changeSlot (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT(3);
    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    args[0]->slot_update(r, arg1->value_get (), args[2], false);
    return args[2];
  }


  /*-------------.
  | Properties.  |
  `-------------*/

  static rObject
  object_class_getProperty (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(3);
    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    rString arg2 = args[2].unsafe_cast<String>();
    assert(arg2);
    rObject res =
      args[0]->property_get(arg1->value_get(), arg2->value_get());
    if (res)
      return res;
    else
      return void_class;
  }

  static rObject
  object_class_hasProperty (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(3);
    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    rString arg2 = args[2].unsafe_cast<String>();
    assert(arg2);
    return
      args[0]->property_get(arg1->value_get(), arg2->value_get())
      ? true_class
      : false_class;
  }

  static rObject
  object_class_setProperty (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(4);
    rString arg1 = args[1].unsafe_cast<String>();
    assert(arg1);
    rString arg2 = args[2].unsafe_cast<String>();
    assert(arg2);
    args[0]->property_set(arg1->value_get(), arg2->value_get(), args[3]);
    return args[3];
  }





  static rObject
  object_class_isA(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    return new Float(is_a(args[0], args[1])? 1.0:0.0);
  }

  void
  object_class_initialize ()
  {
    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(object, Name)

    DECLARE(addProto);
    DECLARE(apply);
    DECLARE(callMessage);
    DECLARE(changeSlot);
    DECLARE(clone);
    DECLARE(dump);
    DECLARE(getLazyLocalSlot);
    DECLARE(getProperty);
    DECLARE(getSlot);
    DECLARE(hasProperty);
    DECLARE(init);
    DECLARE(isA);
    DECLARE(locateSlot);
    DECLARE(memSameAs);
    DECLARE(protos);
    DECLARE(removeProto);
    DECLARE(removeSlot);
    DECLARE(sameAs);
    DECLARE(setProperty);
    DECLARE(setSlot);
    DECLARE(slotNames);
    DECLARE(uid);
    DECLARE(updateSlot);
#undef DECLARE
  }

}; // namespace object
