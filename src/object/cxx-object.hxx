#ifndef CXX_OBJECT_HXX
# define CXX_OBJECT_HXX

# include <boost/bind.hpp>

# include <object/atom.hh>
# include <object/cxx-primitive.hh>
# include <object/primitive-class.hh>
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
    rObject cxx_object_clone(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(1);
      rObject tgt = args[0];
      rObject res;
      if (tgt->is_a<T>())
        res = new T(tgt->as<T>());
      else
        res = new T();
      return res;
    }

    template <typename T>
    rObject cxx_object_id(runner::Runner&, objects_type args)
    {
      CHECK_ARG_COUNT(1);
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
    res_->slot_set(SYMBOL(protoName), new String(name_));
    res_->slot_set(SYMBOL(clone),
                   rPrimitive(new Primitive(boost::bind(cxx_object_clone<T>,
                                                        _1, _2))));
    Binder<T> b(res_);
    T::initialize(b);

    libport::Symbol conversion =
      libport::Symbol(std::string("as") + name_.name_get());
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
    return name_;
  }

  // BINDER
  template <typename T>
  CxxObject::Binder<T>::Binder(rObject tgt)
    : tgt_(tgt)
  {}

  template <typename T>
  template <typename M>
  void CxxObject::Binder<T>::operator()(const libport::Symbol& name,
                                        M method)
  {
    tgt_->slot_set(name, make_primitive(method, name));
  }


  template <typename T>
  inline void
  type_check(rObject o, const libport::Symbol fun)
  {
    libport::shared_ptr<CxxObject> co = o.unsafe_cast<CxxObject>();
     // FIXME: I can't fill all the source type for now since some old
     // atoms don't define type_name for now
    if (!co)
      throw object::WrongArgumentType(T::type_name, "Object", fun);
    else if (!o->is_a<T>())
      throw object::WrongArgumentType(T::type_name, co->type_name_get(), fun);
  }

}

#endif
