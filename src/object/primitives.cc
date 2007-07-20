/**
 ** \file object/primitives.cc
 ** \brief Creation of the root Objects.
 */

#include <cmath>

#include "object/object.hh"
#include "object/atom.hh"

#include "kernel/userver.hh"

namespace object
{

  rObject context_class;
  rObject code_class;
  rObject float_class;
  rObject integer_class;
  rObject object_class;
  rObject primitive_class;
  rObject string_class;


  /*--------------------.
  | Object primitives.  |
  `--------------------*/

  // FIXME: I have put them here, but it's probably not the best
  // place.  We probably needed something like a Server object.

  rObject
  object_class_clone (objects_type args)
  {
    return clone(args[0]);
  }

  rObject
  object_class_init (objects_type args)
  {
    return args[0];
  }

  rObject
  object_class_print (objects_type args)
  {
    std::cout << *args[0] << std::endl;
    return args[0];
  }

#define SERVER_FUNCTION(Function)				\
  rObject							\
  object_class_ ## Function (objects_type args)			\
  {								\
    ::urbiserver->Function();					\
    /* Return the current object to return something. */	\
    return args[0];						\
  }
  SERVER_FUNCTION(reboot)
  SERVER_FUNCTION(shutdown)
#undef SERVER_FUNCTION

  namespace
  {

    /// Initialize the Object class.
    static
    void
    object_class_initialize ()
    {
#define DECLARE(Name)							\
      object_class->slot_set (#Name,					\
			      new Primitive(object_class_ ## Name));
      DECLARE(clone);
      DECLARE(init);
      DECLARE(print);
      DECLARE(reboot);
      DECLARE(shutdown);
#undef DECLARE
    }

  }


  namespace
  {

    /// Initialize the Code class.
    static
    void
    code_class_initialize ()
    {
    }

  }


  /*-------------------.
  | Float primitives.  |
  `-------------------*/

static libport::ufloat
float_req (libport::ufloat l, libport::ufloat r)
{
  // FIXME: get epsilontilde from environment

  // ENSURE_COMPARISON ("Approximate", l, r);
  // UVariable *epsilontilde =
  //   ::urbiserver->getVariable(MAINDEVICE, "epsilontilde");
  // if (epsilontilde)
  //   $EXEC$
  // else
  //   return 0;

# define epsilontilde 0.0001
  return fabs(l - r) <= epsilontilde;
# undef epsilontilde
}

static float
float_deq (libport::ufloat l, libport::ufloat r)
{
  // FIXME: get deltas for l and r

  // ENSURE_COMPARISON ("Approximate", l, r);
  libport::ufloat dl = 0.f;
  libport::ufloat dr = 0.f;
  // dl = 0, get l->delta
  // dr = 0, get r->delta

  return fabs(l - r) <= dl + dr;
}

static float
float_peq (libport::ufloat l, libport::ufloat r)
{
  // FIXME: get epsilonpercent from environment
  // FIXME: return error on div by 0

  // ENSURE_COMPARISON ("Approximate", l, r);
  // UVariable *epsilonpercent =
  //   ::urbiserver->getVariable(MAINDEVICE, "epsilonpercent");
  // if (epsilonpercent)
  //   $EXEC$
  // else
  //   return 0;

  if (r == 0)
    // FIXME: error
    return 0;

# define epsilonpercent 0.0001
  return fabs(1.f - l / r) < epsilonpercent;
# undef epsilonpercent
}

#define DECLARE(Name, Call)						\
    rObject								\
    float_class_ ## Name (objects_type args)				\
    {									\
      assert(args[0]->kind_get() == Object::kind_float);		\
      assert(args[1]->kind_get() == Object::kind_float);		\
      rFloat l = args[0].unsafe_cast<Float> ();				\
      rFloat r = args[1].unsafe_cast<Float> ();				\
      return new Float(Call);	\
    }

#define DECLARE_M(Name, Method)						\
  DECLARE(Name, Method(l->value_get(), r->value_get()))


#define DECLARE_OP(Name, Operator)					\
  DECLARE(Name, l->value_get() Operator r->value_get())


  DECLARE_OP(add, +)
  DECLARE_OP(div, /)
  DECLARE_OP(mul, *)
  DECLARE_OP(sub, -)
  DECLARE_M(pow, powf)
  DECLARE_M(mod, fmod)

  DECLARE_OP(land, &&)
  DECLARE_OP(lor, ||)

  DECLARE_OP(equ, ==)
  DECLARE_M(req, float_req) //REQ ~=
  DECLARE_M(deq, float_deq) //DEQ =~=
  DECLARE_M(peq, float_peq) //PEQ %=
  DECLARE_OP(neq, !=)

  DECLARE_OP(lth, <)
  DECLARE_OP(leq, <=)
  DECLARE_OP(gth, >)
  DECLARE_OP(geq, >=)

#undef DECLARE_M
#undef DECLARE_OP
#undef DECLARE

  namespace
  {
    /// Initialize the Float class.
    static
    void
    float_class_initialize ()
    {
#define DECLARE(Name, Operator)						\
      float_class->slot_set (#Operator,					\
			     new Primitive(float_class_ ## Name));

      DECLARE(add, +)
      DECLARE(div, /)
      DECLARE(mul, *)
      DECLARE(sub, -)
      DECLARE(pow, **)
      DECLARE(mod, %)

      DECLARE(land, &&)
      DECLARE(lor, ||)

      DECLARE(equ, ==)
      DECLARE(req, ~=)
      DECLARE(deq, =~=)
      DECLARE(peq, %=)
      DECLARE(neq, !=)

      DECLARE(lth, <)
      DECLARE(leq, <=)
      DECLARE(gth, >)
      DECLARE(geq, >=)

#undef DECLARE
    }
  }

  namespace
  {

    /// Initialize the Integer class.
    static
    void
    integer_class_initialize ()
    {
    }

    /// Initialize the Primitive class.
    static
    void
    primitive_class_initialize ()
    {
    }

    /// Initialize the Float class.
    static
    void
    string_class_initialize ()
    {
    }

    /// Initialize the root classes.
    /// There are some dependency issues.  For instance, String
    /// is a clone of Object, but Object[type] is a String.
    /// So we need to control the initialization sequence.
    static
    bool
    root_classes_initialize ()
    {
      object_class = new Object;
      // Construct the (empty) objects for the base classes.
      // They all derive from Object.
#define DECLARE(What, Name)			\
      What ## _class = clone(object_class);
      APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(DECLARE);
#undef DECLARE

      // Now that these classes exists, in particular string_class
      // from which any String is a clone, we can initialize the
      // "type" field for all of them, including Object.
#define DECLARE(What, Name)				\
      What ## _class->slot_set("type", new String (#Name));
      APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

      // Now finalize the construction for each base class:
      // bind some initial methods.
#define DECLARE(What, Name)			\
      What ## _class_initialize ();
      APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

      // Some plain classes, initialized by hand currently.
      context_class = clone (object_class);
      context_class->slot_set("type", new String ("Context"));

      // Register all these classes in Object, so that when we look up
      // for "Object" for instance, we find it.
#define DECLARE(What, Name)			\
      object_class->slot_set(#Name, What ## _class);
      APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE
      object_class->slot_set("Context", context_class);

      return true;
    }

    /// Whether the root classes where initialized.
    // Actually made to run the function root_classes_initialize().
    // Not static so that GCC does not complain that it is unused.
    bool root_classes_initialized = root_classes_initialize();
  }

} // namespace object
