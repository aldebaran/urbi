/**
 ** \file object/primitives.cc
 ** \brief Creation of the root Objects.
 */

#include <cmath>

#include <boost/lexical_cast.hpp>

#include "object/object.hh"
#include "object/atom.hh"

#include "kernel/uconnection.hh"
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
  rObject list_class;


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

  rObject
  object_class_wait (objects_type args)
  {
    // FIXME: Currently does nothing.  A stub so that we
    // accept "wait 2s" as is used in the test suite.
    return args[0];
  }


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
      DECLARE(wait);
#undef DECLARE
    }

  }


  /*-------.
  | Code.  |
  `-------*/

  namespace
  {

    rObject
    context_class_echo (objects_type args)
    {
      UConnection& c = args[0].cast<Context>()->value_get();
      c.send (boost::lexical_cast<std::string>(*args[1]).c_str());
      return args[0];
    }


    /// Initialize the Code class.
    static
    void
    code_class_initialize ()
    {
#define DECLARE(Name)							\
      context_class->slot_set (#Name,					\
			       new Primitive(context_class_ ## Name));
      DECLARE(echo);
#undef DECLARE
    }

  }

  /*----------.
  | Context.  |
  `----------*/

  namespace
  {

    /// Initialize the Integer class.
    static
    void
    context_class_initialize ()
    {
    }

  }


  /*--------.
  | Float.  |
  `--------*/

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

  static float
  float_sgn (libport::ufloat x)
  {
    if (x > 0)
      return 1;
    else if (x < 0)
      return -1;
    return 0;
  }

  static float
  float_random (libport::ufloat x)
  {
    float res = 0.f;
    const long long range = libport::to_long_long (x);
    if (range != 0)
      res = rand () % range;
    return res;
  }

  static float
  float_sqr (libport::ufloat x)
  {
    return x * x;
  }

#define DECLARE(Name, Call)				\
  rObject						\
  float_class_ ## Name (objects_type args)		\
  {							\
    assert(args[0]->kind_get() == Object::kind_float);	\
    assert(args[1]->kind_get() == Object::kind_float);	\
    rFloat l = args[0].unsafe_cast<Float> ();		\
    rFloat r = args[1].unsafe_cast<Float> ();		\
    return new Float(Call);				\
  }

#define DECLARE_U(Name, Call)				\
  rObject						\
  float_class_ ## Name (objects_type args)		\
  {							\
    assert(args[1]->kind_get() == Object::kind_float);	\
    rFloat x = args[1].unsafe_cast<Float> ();		\
    return new Float(Call);				\
  }

#define DECLARE_M(Name, Method)				\
  DECLARE(Name, Method(l->value_get(), r->value_get()))


  //FIXME: check if rvalue is 0 for % and / operators
#define DECLARE_OP(Name, Operator)			\
  DECLARE(Name, l->value_get() Operator r->value_get())

#define DECLARE_U_M(Name, Method)		\
  DECLARE_U(Name, Method(x->value_get()))


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


  DECLARE_U_M(sin, sin)
  DECLARE_U_M(asin, asin)
  DECLARE_U_M(cos, cos)
  DECLARE_U_M(acos, acos)
  DECLARE_U_M(tan, tan)
  DECLARE_U_M(atan, atan)
  DECLARE_U_M(sgn, float_sgn)
  DECLARE_U_M(abs, fabs)
  DECLARE_U_M(exp, exp)
  DECLARE_U_M(log, log)
  DECLARE_U_M(round, round)
  DECLARE_U_M(random, float_random)
  DECLARE_U_M(trunc, trunc)
  DECLARE_U_M(sqr, float_sqr)
  DECLARE_U_M(sqrt, sqrt)

#undef DECLARE_U_M
#undef DECLARE_M
#undef DECLARE_OP
#undef DECLARE
#undef DECLARE_U

  namespace
  {
    /// Initialize the Float class.
    static
    void
    float_class_initialize ()
    {
#define DECLARE(Name, Operator)						\
      float_class->slot_set (#Operator,					\
			     new Primitive(float_class_ ## Name))

#define DECLARE_(Name)				\
      DECLARE(Name, Name)

      DECLARE(add, +);
      DECLARE(div, /);
      DECLARE(mul, *);
      DECLARE(sub, -);
      DECLARE(pow, **);
      DECLARE(mod, %);

      DECLARE(land, &&);
      DECLARE(lor, ||);

      DECLARE(equ, ==);
      DECLARE(req, ~=);
      DECLARE(deq, =~=);
      DECLARE(peq, %=);
      DECLARE(neq, !=);

      DECLARE(lth, <);
      DECLARE(leq, <=);
      DECLARE(gth, >);
      DECLARE(geq, >=);

      DECLARE_(sin);
      DECLARE_(asin);
      DECLARE_(cos);
      DECLARE_(acos);
      DECLARE_(tan);
      DECLARE_(atan);
      DECLARE_(sgn);
      DECLARE_(abs);
      DECLARE_(exp);
      DECLARE_(log);
      DECLARE_(round);
      DECLARE_(random);
      DECLARE_(trunc);
      DECLARE_(sqr);
      DECLARE_(sqrt);

#undef DECLARE_
#undef DECLARE
    }
  }

  /*----------.
  | Integer.  |
  `----------*/

  namespace
  {

    /// Initialize the Integer class.
    static
    void
    integer_class_initialize ()
    {
    }

  }

  /*------------.
  | Primitive.  |
  `------------*/

  namespace
  {

    /// Initialize the Primitive class.
    static
    void
    primitive_class_initialize ()
    {
    }

  }

  /*--------.
  | String.  |
  `--------*/

  namespace
  {

    /// Initialize the String class.
    static
    void
    string_class_initialize ()
    {
    }

  }

  /*-------.
  | List.  |
  `-------*/

  namespace
  {
    /// Concatenate two list
    /**
     * @return A fresh list, concatenation of \a lsh and \a rhs
     */
    static rList list_concat(rList& lhs, const rList& rhs)
    {
      // Copy lhs
      list_traits::type res(lhs->value_get());

      // Append rhs

      // FIXME: I can't explain why, but the line below result in an
      // infinite loop. Use foreach instead for now.

//    res.insert(res.end(), rhs->value_get().begin(), rhs->value_get().end());
      BOOST_FOREACH (const rObject& o, rhs->value_get())
        res.push_back(o);

      return new List(res);
    }

    rObject
    list_class_ip_concat (objects_type args)
    {
      assert(args[0]->kind_get() == Object::kind_list);
      assert(args[1]->kind_get() == Object::kind_list);
      rList lhs = args[0].unsafe_cast<List>();
      rList rhs = args[1].unsafe_cast<List>();
      rList res = list_concat(lhs, rhs);
      return res;
    }


    /// Initialize the List class.
    static
    void
    list_class_initialize ()
    {
      list_class->slot_set ("+",
                            new Primitive(list_class_ip_concat));
    }

  }

    /*------------------------.
    | Global initialization.  |
    `------------------------*/

  namespace
  {

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
#define DECLARE(What, Name)					\
      What ## _class->slot_set("type", new String (#Name));
      APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

      // Now finalize the construction for each base class:
      // bind some initial methods.
#define DECLARE(What, Name)			\
      What ## _class_initialize ();
      APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

      // Register all these classes in Object, so that when we look up
      // for "Object" for instance, we find it.
#define DECLARE(What, Name)				\
      object_class->slot_set(#Name, What ## _class);
      APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

      return true;
    }

    /// Whether the root classes where initialized.
    // Actually made to run the function root_classes_initialize().
    // Not static so that GCC does not complain that it is unused.
    bool root_classes_initialized = root_classes_initialize();
  }

} // namespace object
