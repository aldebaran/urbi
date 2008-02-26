/**
 ** \file object/object-class.cc
 ** \brief Creation of the URBI object object.
 */

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>
#include <libport/throw-exception.hh>
#include <libport/tokenizer.hh>

#include "kernel/uconnection.hh"

#include "runner/runner.hh"

#include "object/atom.hh"
#include "object/object-class.hh"
#include "object/object.hh"

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
    return clone(args[0]);
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
    // Second argument is the tag name.
    std::string tag;
    if (args.size() == 2)
    {
      FETCH_ARG(1, String);
      tag = arg1->value_get().name_get();
    }
    std::ostringstream os;
    args[0]->dump(os);
    //for now our best choice is to dump line by line in "system" messages.
    const std::string stream = os.str();
    boost::tokenizer< boost::char_separator<char> > tok =
      libport::make_tokenizer(stream, "\n");
    std::string system_header("*** ");
    BOOST_FOREACH(std::string line, tok)
      r.lobby_get()->value_get().connection.send (
	(system_header+line+"\n").c_str(), tag.c_str());
    return void_class;
  }

  static rObject
  object_echo(runner::Runner& r, objects_type args, bool is_echo)
  {
    // Second argument is the tag name.
    std::string tag;
    if (args.size() == 3)
    {
      FETCH_ARG(2, String);
      tag = arg2->value_get().name_get();
    }
    //special case for Strings
    if (is_echo && args[1]->kind_is(Object::kind_string))
    {
      r.lobby_get()->value_get().connection.send((std::string("*** ")
			   + std::string(VALUE(args[1], String))
			   + "\n").c_str(),
			   tag.c_str());
    }
    else
      r.lobby_get()->value_get().connection.send (args[1], tag.c_str(),
	is_echo?"*** ":"");
    return void_class;
  }

  /// Send pretty-printed args[1] to the connection.
  static rObject
  object_class_echo (runner::Runner& r, objects_type args)
  {
    return object_echo(r, args, true);
  }

  /// Send pretty-printed self on the connection.
  /// args[1], if present, can be the tag to use.
  static rObject
  object_class_print (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE (1, 2);
    objects_type nargs;
    nargs.push_back(args[0]);
    nargs.insert(nargs.end(),  args.begin(), args.end());
    return object_echo(r, nargs, false);
  }

  /// Send pretty-printed self on the connection.
  /// args[1], if present, can be the tag to use.
  static rObject
  object_class_asString (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    std::ostringstream os;
    args[0]->print(os);
    return new String(libport::Symbol(os.str()));
  }

  /// Return the address of an object as a number, mostly
  /// for debugging purpose.
  static rObject
  object_class_uid (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    std::string str = (boost::format ("0x%x")
		       % reinterpret_cast<long long> (args[0].get ())).str ();
    return new String (libport::Symbol (str));
  }

  static rObject
  object_class_sameAs(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    return args[0]->slot_get((args[0] == args[1])
			     ? SYMBOL(true)
			     : SYMBOL(false));
  }

  static rObject
  object_class_apply (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, List);
    if (!arg1->value_get ().empty ())
      boost::throw_exception
	(PrimitiveError ("apply", "first argument must be an empty list"));
    return args[0];
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
    rObject obj = args[0];

    object::list_traits::type l;
    BOOST_FOREACH (const rObject o, obj->protos_get())
      l.push_back(o);

    return new object::List(l);
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

    object::list_traits::type l;
    BOOST_FOREACH (const Object::slot_type& p, obj->slots_get())
      l.push_back (new object::String (p.first));

    return new object::List (l);
  }

  /// Get a slot content.
  static rObject
  object_class_getSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    rObject obj = args[0];
    FETCH_ARG(1, String);
    return obj->slot_get(arg1->value_get());
  }

  static rObject
  object_class_getLazyLocalSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (4);

    FETCH_ARG (1, String);
    Object::key_type slot_name = arg1->value_get ();

    // If the slot already exists, return its content.
    rObject slot = args[0]->own_slot_get (slot_name, 0);
    if (slot)
      return slot;

    // The slot doesn't exist. Should we create it?
    if (IS_TRUE (args[3]))
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
    FETCH_ARG(1, String);
    obj->slot_remove(arg1->value_get());
    return obj;
  }

   /// Locate a slot.
  static rObject
  object_class_locateSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(1, String);

    rObject o = slot_locate(args[0], arg1->value_get());
    return o ? o : nil_class;
  }

  static rObject
  object_class_setSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (3);
    FETCH_ARG (1, String);
    args[0]->slot_set (arg1->value_get (), args[2]);
    return args[2];
  }

  static rObject
  object_class_updateSlot (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (3);
    FETCH_ARG (1, String);
    slot_update (r, args[0], arg1->value_get (), args[2]);
    return args[2];
  }

  static rObject
  object_class_makeScope (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    args[0]->locals_set (IS_TRUE (args[1]));
    return args[0];
  }

  static rObject
  object_class_isScope (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new Float (!!args[0]->locals_get ());
  }

  void
  object_class_initialize ()
  {
    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(object, Name)

    DECLARE(addProto);
    DECLARE(apply);
    DECLARE(asString);
    DECLARE(clone);
    DECLARE(dump);
    DECLARE(echo); // This guy should be in System.
    DECLARE(getLazyLocalSlot);
    DECLARE(getSlot);
    DECLARE(init);
    DECLARE(isScope);
    DECLARE(locateSlot);
    DECLARE(makeScope);
    DECLARE(print);
    DECLARE(protos);
    DECLARE(removeProto);
    DECLARE(removeSlot);
    DECLARE(sameAs);
    DECLARE(setSlot);
    DECLARE(slotNames);
    DECLARE(uid);
    DECLARE(updateSlot);
#undef DECLARE
  }

}; // namespace object
