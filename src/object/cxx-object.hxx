#ifndef CXX_OBJECT_HXX
# define CXX_OBJECT_HXX

# include <boost/bind.hpp>

# include <object/atom.hh>
# include <object/string-class.hh>
# include <object/object-class.hh>
# include <object/primitives.hh>

namespace object
{
  template <typename T>
  bool CxxObject::add(const std::string& name, rObject& tgt)
  {
    initializers_get().push_back(new TypeInitializer<T>(name, tgt));
    return true;
  }

  template <typename T>
  CxxObject::TypeInitializer<T>::TypeInitializer(const std::string& name,
                                                 rObject& tgt)
    : Initializer(tgt), name_(name)
  {}

  namespace
  {
    template <typename T>
    rObject cxx_object_clone(rObject parent, runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(1);
      rObject tgt = args[0];
      rObject res;
      if (tgt->is_a<T>())
        res = new T(tgt->as<T>());
      else
        res = new T();
      res->proto_add(parent);
      return res;
    }
  }

  template <typename T>
  void
  CxxObject::TypeInitializer<T>::create()
  {
    res_ = new Object();
  }

  template <typename T>
  rObject
  CxxObject::TypeInitializer<T>::make_class()
  {
    res_->proto_add(object_class);
    res_->slot_set(SYMBOL(protoName), new String(name_));
    res_->slot_set(SYMBOL(clone),
                   new Primitive(boost::bind(cxx_object_clone<T>,
                                             res_, _1, _2)));
    Binder<T> b(res_);
    T::initialize(b);
    return res_;
  }

  template <typename T>
  libport::Symbol
  CxxObject::TypeInitializer<T>::name()
  {
    return name_;
  }

  // BINDER

  template <typename T>
  CxxObject::Binder<T>::Binder(rObject tgt)
    : tgt_(tgt)
  {}

  namespace
  {

# define PRINT_true(X) X
# define PRINT_false(X)
# define PRINT_truetrue(X) X
# define PRINT_falsetrue(X)
# define PRINT_truefalse(X)
# define PRINT_falsefalse(X)
# define NPRINT_true(X)
# define NPRINT_false(X) X

# define WHEN(Cond, X) PRINT_##Cond(X)
# define WHEN_NOT(Cond, X) NPRINT_##Cond(X)

# define IF(Cond, Then, Else) WHEN(Cond, Then) WHEN_NOT(Cond, Else)

# define COMMA_ ,
# define COMMA(Cond) PRINT_##Cond(COMMA_)

# define COMMA2(Cond1, Cond2) PRINT_##Cond1##Cond2(COMMA_)

# define MET(Name, Ret, Run, Arg1, Arg2, Arg3)          \
    IF(Ret, libport::shared_ptr<R>, void)               \
    (T::*Name)(                                         \
      WHEN(Run, runner::Runner&) COMMA2(Run, Arg1)      \
      WHEN(Arg1, libport::shared_ptr<A1>)               \
      COMMA(Arg2) WHEN(Arg2, libport::shared_ptr<A2>)   \
      COMMA(Arg3) WHEN(Arg3, libport::shared_ptr<A3>)   \
      )

#define GET_ARG(N)                                                      \
    type_check<A##N>(args[N], name.name_get());                         \
    libport::shared_ptr<A##N> a##N =  args[N].unsafe_cast<A##N>();      \
    assert(a##N);

#define PRIMITIVE(Ret, ArgsC, Run, Arg1, Arg2, Arg3)                    \
    template <typename T                                                \
              COMMA(Ret) WHEN(Ret, typename R)                          \
      COMMA(Arg1) WHEN(Arg1, typename A1)                               \
      COMMA(Arg2) WHEN(Arg2, typename A2)                               \
      COMMA(Arg3) WHEN(Arg3, typename A3)                               \
      >                                                                 \
    struct primitive<T, MET(, Ret, Run, Arg1, Arg2, Arg3)>              \
    {                                                                   \
        static rObject make(                                            \
          MET(method, Ret, Run, Arg1, Arg2, Arg3),                      \
          const libport::Symbol& name,                                  \
          runner::Runner& WHEN(Run, r),                                 \
          objects_type args)                                            \
        {                                                               \
          assert(args.size() == ArgsC + 1);                             \
          type_check<T>(args[0], name.name_get());                      \
          libport::shared_ptr<T> tgt = args[0].unsafe_cast<T>();        \
          assert(tgt);                                                  \
          WHEN(Arg1, GET_ARG(1))                                        \
          WHEN(Arg2, GET_ARG(2))                                        \
          WHEN(Arg3, GET_ARG(3))                                        \
            WHEN(Ret, return)                                           \
            (tgt.get()->*method)(                                       \
              WHEN(Run, r) COMMA2(Run, Arg1)                            \
              WHEN(Arg1, a1)                                            \
              COMMA(Arg2) WHEN(Arg2, a2)                                \
              COMMA(Arg3) WHEN(Arg3, a3)                                \
              );                                                        \
          return void_class;                                            \
        }                                                               \
      }

    template <typename T, typename M>
    struct primitive
    {
    };

    /* Python for these:
        max = 3
        for ret in ('true', 'false'):
            for run in ('true', 'false'):
                for nargs in range(max + 1):
                    print "    PRIMITIVE(%s, %i, %s" % (ret, nargs, run),
                    for i in range(max):
                        print ", %s" % str(i < nargs).lower(),
                    print ");"
    */
    PRIMITIVE(true, 0, true , false , false , false );
    PRIMITIVE(true, 1, true , true , false , false );
    PRIMITIVE(true, 2, true , true , true , false );
    PRIMITIVE(true, 3, true , true , true , true );
    PRIMITIVE(true, 0, false , false , false , false );
    PRIMITIVE(true, 1, false , true , false , false );
    PRIMITIVE(true, 2, false , true , true , false );
    PRIMITIVE(true, 3, false , true , true , true );
    PRIMITIVE(false, 0, true , false , false , false );
    PRIMITIVE(false, 1, true , true , false , false );
    PRIMITIVE(false, 2, true , true , true , false );
    PRIMITIVE(false, 3, true , true , true , true );
    PRIMITIVE(false, 0, false , false , false , false );
    PRIMITIVE(false, 1, false , true , false , false );
    PRIMITIVE(false, 2, false , true , true , false );
    PRIMITIVE(false, 3, false , true , true , true );
  }

  template <typename T>
  template <typename M>
  void CxxObject::Binder<T>::operator()(const libport::Symbol& name,
                                        M method)
  {
    // If make is unfound here, you passed an unsupported pointer type
    // to the binder.
    rObject p =
      new Primitive(boost::bind(primitive<T, M>::make,
                                method, name, _1, _2));
    tgt_->slot_set(name, p);
  }


  template <typename T>
  inline void
  type_check(rObject o, const std::string& fun)
  {
    libport::shared_ptr<CxxObject> co = o.unsafe_cast<CxxObject>();
     // FIXME: I can't fill all the source type for now since some old
     // atoms don't define type_name for now
    if (!co)
      throw object::WrongArgumentType(T::type_name, "Object", fun);
    else if (!o->is_a<T>())
      throw object::WrongArgumentType(T::type_name, co->type_name_get(), fun);
  }

  template <>
  inline void
  type_check<Object>(rObject, const std::string&)
  {}

  template <>
  inline void
  type_check<List>(rObject, const std::string&)
  {}

}

#endif
