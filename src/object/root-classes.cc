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
 ** \file object/root-classes.cc
 ** \brief Creation of the root Objects.
 */
#include <urbi/object/fwd.hh>
#include <object/object-class.hh>
#include <object/root-classes.hh>
#include <object/semaphore.hh>
#include <object/server.hh>
#include <object/socket.hh>
#include <urbi/object/symbols.hh>
#include <object/system.hh>
#include <object/uvar.hh>
#include <urbi/object/barrier.hh>
#include <object/code.hh>
#include <urbi/object/cxx-object.hh>
#include <urbi/object/date.hh>
#include <urbi/object/dictionary.hh>
#include <urbi/object/directory.hh>
#include <urbi/object/duration.hh>
#include <urbi/object/event.hh>
#include <urbi/object/file.hh>
#include <urbi/object/float.hh>
#include <urbi/object/global.hh>
#include <urbi/object/job.hh>
#include <urbi/object/list.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/path.hh>
#include <urbi/object/primitive.hh>
#include <urbi/object/string.hh>
#include <urbi/object/tag.hh>

namespace urbi
{
  namespace object
  {

#define CLASS_INITIALIZE(What)					\
    rObject What ## _class;					\
    namespace { static void What ## _class_initialize (); }

// Classes with nothing to initialize
#define CLASS_EMPTY_INITIALIZE(What)                            \
    rObject What ## _class;					\
    namespace { static void What ## _class_initialize () { } }

    /* Help the generation of symbols.

       SYMBOL(Dictionary);
       SYMBOL(Global);
       SYMBOL(System);
       SYMBOL(Tag);
       SYMBOL(acceptedVoid);
       SYMBOL(asCode);
       SYMBOL(asDictionary);
       SYMBOL(asFloat);
       SYMBOL(asGlobal);
       SYMBOL(asInteger);
       SYMBOL(asJob);
       SYMBOL(asList);
       SYMBOL(asLobby);
       SYMBOL(asObject);
       SYMBOL(asPrimitive);
       SYMBOL(asSystem);
       SYMBOL(asTag);
       SYMBOL(asfalse);
       SYMBOL(asnil);
       SYMBOL(astrue);
       SYMBOL(asvoid);
       SYMBOL(false);
       SYMBOL(nil);
       SYMBOL(true);
       SYMBOL(void);

    */

    CLASS_INITIALIZE(accepted_void);
    CLASS_EMPTY_INITIALIZE(false);
    CLASS_EMPTY_INITIALIZE(nil);
    CLASS_EMPTY_INITIALIZE(true);
    CLASS_INITIALIZE(void);

#undef CLASS_INITIALIZE

    /*------------------------.
    | Global initialization.  |
    `------------------------*/

    /// Whether the root classes where initialized.
    bool root_classes_initialized = false;

    static rObject
    id(objects_type args)
    {
      check_arg_count(args.size() - 1, 0);
      return args.front();
    }

    /// Initialize the root classes.  There are some dependency issues.
    /// For instance, String is a clone of Object, but Object.type is a
    /// String.  So we need to control the initialization sequence.
    void
    root_classes_initialize()
    {
      if (root_classes_initialized)
        return;
      root_classes_initialized = true;

      // The construction of the primitive classes goes in several
      // steps.
      //
      // 1. Construct the (empty) objects for the base classes.  They
      // all derive from Object... except Object.
      //
      // 2. Initialize the "type" field for all of them, including
      // Object (requires that these classes exists, in particular
      // string_class from which any String is a clone).
      //
      // 3. Finalize the construction for each base class: bind some
      // initial methods.
      //
      // 4. Register all these classes in Global, to make them
      // accessible from anywhere.
      //
      // CLASS_CREATE does 1, CLASS_INIT does 2, CLASS_REGISTER does 3
      // and 4.  CLASS_SETUP runs 1 to 4.

#define CLASS_CREATE(What, Name)                \
      What = Object::proto->clone()

#define CLASS_INIT(What, Name)                          \
      What->slot_set(SYMBOL(type),                      \
                     new String(SYMBOL_(Name)));        \
      What->slot_set(SYMBOL(as ## Name),                \
                     new Primitive(id))

#define CLASS_REGISTER(What, Name)                      \
      What ## _initialize();                            \
      global_class->slot_set(SYMBOL_(Name), What, true)


#define CLASS_SETUP(What, Name)                 \
      CLASS_CREATE(What, Name);                 \
      CLASS_INIT(What, Name);                   \
      CLASS_REGISTER(What, Name)

#define ANONYMOUS_CLASS_SETUP(What, Name)       \
      CLASS_CREATE(What, Name);                 \
      CLASS_REGISTER(What, Name)

#define EXISTING_CLASS_SETUP(What, Name)        \
      CLASS_INIT(What, Name);                   \
      CLASS_REGISTER(What, Name)

      // Object is a special case: it is not built as a clone of itself.
      Object::proto = new Object();

      // These guys are used in the property system, we need them
      // early.  But completing their initialization requires String
      // support.  So this is performed below.
      true_class = new Object();
      true_class->proto_add(Object::proto);
      false_class = new Object();
      false_class->proto_add(Object::proto);

      global_class = Object::proto->clone();

      // Run the initialization hook that where setup at static build
      // time.
      object::initializations_run();

      // The other primitives.  Because primitive initialization depend
      // a lot on one another (e.g., String is used everywhere for slot
      // names, and Primitive is used for... all the primitive methods
      // in the primitive classes), first create them all, then bind
      // them all.
      // Setup boolean entities.
      CLASS_INIT(global_class, Global);
      CLASS_INIT(Object::proto, Object);
      global_class_initialize();
      global_class->slot_set(SYMBOL(Global), global_class, true);
      object_class_initialize();
      global_class->slot_set(SYMBOL(Object), Object::proto, true);

      // Completion cannot be done before String is complete.
      EXISTING_CLASS_SETUP(true_class, true);
      EXISTING_CLASS_SETUP(false_class, false);

      CLASS_SETUP(nil_class, nil);
      CLASS_SETUP(system_class, System);
      CLASS_SETUP(void_class, void);

      ANONYMOUS_CLASS_SETUP(accepted_void_class, acceptedVoid);

      // Object.addProto(Global)
      Object::proto->proto_add(global_class);
    }

    // This is only used to created references on unused classes, so
    // they get initialized anyway.
    void
    dummy_references()
    {
      Barrier     *b;
      LIBPORT_USE(b);
      Date        date;
      Directory   d;
      Event       e(0);
      LIBPORT_USE(e);
      File        f;
      Path        p;
      Semaphore   s;
      UVar        v;
      Socket      so;
      Tag         t;
    }

    namespace
    {
      template <typename T>
      static inline void
      cleanup_object(libport::intrusive_ptr<T>& o)
      {
        o->protos_set(new object::List(object::List::value_type()));
        o.reset();
      }
    }

    void
    cleanup_existing_objects()
    {
      cleanup_object(Barrier::proto);
      cleanup_object(Code::proto);
      cleanup_object(Dictionary::proto);
      cleanup_object(Float::proto);
      cleanup_object(Job::proto);
      cleanup_object(Lobby::proto);
      cleanup_object(Object::proto); // FIXME
      cleanup_object(Primitive::proto);
      cleanup_object(Semaphore::proto);
      cleanup_object(String::proto);
      cleanup_object(Tag::proto);
      cleanup_object(false_class);
      cleanup_object(global_class);
      cleanup_object(system_class);
      cleanup_object(true_class);
      cleanup_object(void_class);
      // List must be last, because it is used in cleanup_object.
      cleanup_object(List::proto);
    }

    /*-------.
    | void.  |
    `-------*/
    namespace
    {
      static rObject
      void_class_acceptVoid(objects_type args)
      {
        check_arg_count(args.size() - 1, 0);

        return accepted_void_class;
      }

      static void
      void_class_initialize()
      {
        void_class->slot_set(SYMBOL(asString), new String(SYMBOL(void)));
        void_class->slot_set(SYMBOL(DOLLAR_id), new String(SYMBOL(void)));
        // void prints nothing in the toplevel
        void_class->slot_set(SYMBOL(asToplevelPrintable), new String(""));
        passert("void must be initialized after true", true_class);
        void_class->slot_set(SYMBOL(isVoid), true_class);
        void_class->slot_set
          (SYMBOL(acceptVoid), new Primitive(void_class_acceptVoid));
      }

      static void
      accepted_void_class_initialize()
      {
        accepted_void_class->proto_remove(Object::proto);
        passert("void must be initialized before acceptedVoid", void_class);
        accepted_void_class->proto_add(void_class);
      }
    }

  } // namespace object
}
