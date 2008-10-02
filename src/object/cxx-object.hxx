#ifndef CXX_OBJECT_HXX
# define CXX_OBJECT_HXX

# include <boost/bind.hpp>

# include <object/cxx-primitive.hh>
# include <object/primitive.hh>
# include <object/string.hh>
# include <object/object-class.hh>
# include <object/primitives.hh>
# include <runner/raise.hh>

namespace object
{
  inline void
  check_arg_count (unsigned effective, unsigned formal)
  {
    if (formal != effective)
      runner::raise_arity_error(effective, formal);
  }

  inline void
  check_arg_count (unsigned effective, unsigned min, unsigned max)
  {
    if (effective < min || effective > max)
      runner::raise_arity_error(effective, min, max);
  }

  template <typename T>
  bool CxxObject::add(rObject& tgt)
  {
    initializers_get().push_back(new TypeInitializer<T>(tgt));
    return true;
  }

  template <typename T>
  CxxObject::TypeInitializer<T>::TypeInitializer(rObject& tgt)
    : Initializer(tgt)
  {}

  namespace
  {
    template <typename T>
    rObject cxx_object_clone(runner::Runner&, objects_type args)
    {
      check_arg_count(args.size() - 1, 0);
      rObject tgt = args[0];
      return tgt->is_a<T>() ? new T(tgt->as<T>()) : new T();
    }

    template <typename T>
    rObject cxx_object_id(runner::Runner&, objects_type args)
    {
      check_arg_count(args.size() - 1, 0);
      return args[0];
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
    res_->slot_set(SYMBOL(protoName), new String(T::type_name));
    res_->slot_set(SYMBOL(clone),
                   rPrimitive(new Primitive(boost::bind(cxx_object_clone<T>,
                                                        _1, _2))));
    Binder<T> b(res_);
    T::initialize(b);

    libport::Symbol conversion =
      libport::Symbol(std::string("as") + T::type_name);
    if (!res_->slot_locate(conversion, 0))
      res_->slot_set(conversion,
                     rPrimitive(new Primitive(boost::bind(cxx_object_id<T>,
                                                          _1, _2))));
    return res_;
  }

  template <typename T>
  libport::Symbol
  CxxObject::TypeInitializer<T>::name()
  {
    return libport::Symbol(T::type_name);
  }

  // BINDER
  template <typename T>
  CxxObject::Binder<T>::Binder(rObject tgt)
    : tgt_(tgt)
  {}

  template <typename T>
  template <typename M>
  void
  CxxObject::Binder<T>::operator()(const libport::Symbol& name,
                                   M method)
  {
    tgt_->slot_set(name, make_primitive(method, name));
  }

}

#endif
