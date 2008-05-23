/**
 ** \file object/root-classes.cc
 ** \brief Creation of the root Objects.
 */

#include "object/alien-class.hh"
#include "object/atom.hh"
#include "object/code-class.hh"
#include "object/delegate-class.hh"
#include "object/dictionary-class.hh"
#include "object/float-class.hh"
#include "object/global-class.hh"
#include "object/integer-class.hh"
#include "object/lobby-class.hh"
#include "object/object.hh"
#include "object/object-class.hh"
#include "object/primitive-class.hh"
#include "object/root-classes.hh"
#include "object/scope-class.hh"
#include "object/string-class.hh"
#include "object/system-class.hh"
#include "object/tag-class.hh"
#include "object/task-class.hh"

namespace object
{

  // The following initialization routines are useless, but allow a
  // more systematic initialization of all the base classes.
#define CLASS_INITIALIZE(What)					\
  rObject What ## _class;					\
  namespace { static void What ## _class_initialize () { } }

  /* Help the generation of symbols.

  SYMBOL(asAlien);
  SYMBOL(asCode);
  SYMBOL(asDelegate);
  SYMBOL(asDictionary);
  SYMBOL(asFloat);
  SYMBOL(asGlobal);
  SYMBOL(asInteger);
  SYMBOL(asList);
  SYMBOL(asLobby);
  SYMBOL(asPrimitive);
  SYMBOL(asScope);
  SYMBOL(asTag);
  SYMBOL(asTask);
  SYMBOL(asObject);
  SYMBOL(asnil);
  SYMBOL(asSystem);
  SYMBOL(asvoid);
  SYMBOL(nil);
  SYMBOL(Dictionary);
  SYMBOL(Global);
  SYMBOL(Scope);
  SYMBOL(System);
  SYMBOL(Tag);
  SYMBOL(void);

  */

  CLASS_INITIALIZE(nil);
  // Where we store all the primitives (objects and functions).
  CLASS_INITIALIZE(void);
#undef CLASS_INITIALIZE

  /*------------------------.
  | Global initialization.  |
  `------------------------*/

  /// Whether the root classes where initialized.
  bool root_classes_initialized = false;

  static rObject
  id(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return args.front();
  }

  template <typename T>
  static rObject
  compare(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    TYPE_CHECK(args[0], T);
    libport::shared_ptr<T> arg0 = args[0].unsafe_cast<T>();
    try
    {
      TYPE_CHECK(args[1], T);
    }
    catch (WrongArgumentType&)
    {
      // Comparing two atoms with different types isn't an error, it
      // just return false
      return Float::fresh(0);
    }
    libport::shared_ptr<T> arg1 = args[1].unsafe_cast<T>();
    return Float::fresh(arg0->value_get() == arg1->value_get());
  }

  /// Initialize the root classes.
  /// There are some dependency issues.  For instance, String
  /// is a clone of Object, but Object[protoName] is a String.
  /// So we need to control the initialization sequence.
  void
  root_classes_initialize ()
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
    // 2. Initialize the "protoName" field for all of them, including
    // Object (requires that these classes exists, in particular
    // string_class from which any String is a clone).
    //
    // 3. Finalize the construction for each base class: bind some
    // initial methods.
    //
    // 4. Register all these classes in Global, to make them
    // accessible from anywhere.
    //
    // CLASS_CREATE does 1, CLASS_INIT does 2 to 4, and CLASS_SETUP
    // runs 1 to 4.

#define CLASS_CREATE(What, Name)		\
    What ## _class = object_class->clone();

// Alias unmatched by symbols-generate.pl
#define SYMBOL_ SYMBOL

#define CLASS_INIT(What, Name)					\
    What ## _class->slot_set(SYMBOL(protoName),			\
			     String::fresh(SYMBOL(Name)));      \
    What ## _class->slot_set(SYMBOL_(as ## Name),		\
			     Primitive::fresh(id));             \
    What ## _class_initialize ();				\
    global_class->slot_set(SYMBOL(Name), What ## _class);


#define CLASS_SETUP(What, Name)			\
    CLASS_CREATE(What, Name)			\
    CLASS_INIT(What, Name)

    // Object is a special case: it is not built as a clone of itself.
    object_class = Object::fresh();
    // The other primitives.  Because primitive initialization depend
    // a lot on one another (e.g., String is used everywhere for slot
    // names, and Primitive is used for... all the primitive methods
    // in the primitive classes), first create them all, then bind
    // them all.
    APPLY_ON_ALL_ROOT_CLASSES_BUT_OBJECT(CLASS_CREATE);

    APPLY_ON_ALL_ROOT_CLASSES(CLASS_INIT);

    CLASS_SETUP(nil, nil);
    CLASS_SETUP(system, System);
    CLASS_SETUP(void, void);

#undef SYMBOL_

// This can't be APPLYED_ON_ALL_PRIMITIVES_BUT_OBJECT because some are
// false atoms
#define COMPARABLE(What, Name)		\
    What ## _class->slot_set(SYMBOL(sameAs), Primitive::fresh(compare<Name>));

    COMPARABLE(float,     Float);
    COMPARABLE(integer,   Integer);
    COMPARABLE(string,    String);

    // Object.addProto(Global)
    object_class->proto_add(global_class);
  }

} // namespace object
