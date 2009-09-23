/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#ifndef CXX_OBJECT_HXX
# define CXX_OBJECT_HXX

# include <libport/bind.hh>

# include <object/cxx-primitive.hh>
# include <object/object-class.hh>
# include <object/primitive.hh>
# include <object/string.hh>
# include <object/symbols.hh>
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
    if (min == max)
      check_arg_count(effective, min);
    else
      if (effective < min || effective > max)
        runner::raise_arity_error(effective, min, max);
  }

  template <typename T>
  bool CxxObject::add()
  {
    initializers_get().push_back(new TypeInitializer<T>());
    return true;
  }

  template <typename T>
  CxxObject::TypeInitializer<T>::TypeInitializer()
    : Initializer(T::proto)
  {}

  namespace
  {
    template <typename T>
    rObject cxx_object_clone(objects_type args)
    {
      check_arg_count(args.size() - 1, 0);
      rObject tgt = args[0];
      assert(tgt->as<T>());
      return new T(tgt->as<T>());
    }

    template <typename T>
    rObject cxx_object_id(objects_type args)
    {
      check_arg_count(args.size() - 1, 0);
      return args[0];
    }
  }

  template <typename T>
  void
  CxxObject::TypeInitializer<T>::create()
  {
    res_ = T::proto_make();
    assert(res_);
  }

  template <typename T>
  rObject
  CxxObject::TypeInitializer<T>::make_class()
  {
    using boost::bind;

    static libport::Symbol type("type");
    static libport::Symbol clone("clone");
    res_->slot_set(type, new String(T::type_name()), true);
    res_->slot_set(clone, new Primitive(bind(cxx_object_clone<T>, _1)), true);
    Binder<T> b(res_);
    T::initialize(b);

    libport::Symbol name = libport::Symbol(std::string("as") + T::type_name());
    if (!res_->slot_locate(name, false).first)
      res_->slot_set(name, new Primitive(bind(cxx_object_id<T>, _1)), true);
    return res_;
  }

  template <typename T>
  libport::Symbol
  CxxObject::TypeInitializer<T>::name()
  {
    return libport::Symbol(T::type_name());
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
    tgt_->slot_set(name, make_primitive(method), true);
  }

  template <typename C, typename T>
  static rObject
  getter(libport::intrusive_ptr<C> self, T (C::* value))
  {
    return to_urbi(self.get()->*value);
  }

  template <typename C, typename T>
  static rObject
  setter(T (C::* attr), libport::intrusive_ptr<C> self,
         const std::string&, const T& value)
  {
    self.get()->*attr = value;
    return 0;
  }

  template <typename T>
  template <typename A>
  void
  CxxObject::Binder<T>::var(const libport::Symbol& name,
                            A (T::*attr))
  {
    using libport::intrusive_ptr;
    using boost::function1;
    using boost::function3;

    typedef function1<rObject, intrusive_ptr<T> > urbi_getter_type;
    typedef function3<rObject, intrusive_ptr<T>, const std::string&, const A&>
      urbi_setter_type;

    urbi_getter_type get(boost::bind(&getter<T, A>, _1, attr));
    tgt_->slot_set(name, make_primitive(get), true);

    urbi_setter_type set(boost::bind(&setter<T, A>, attr, _1, _2, _3));
#define S(Name) Symbol(#Name)
    tgt_->property_set(name, libport::S(updateHook), make_primitive(set));
#undef S
  }

  template<typename T>
  inline libport::intrusive_ptr<T>
  type_check(const rObject& o,
	     boost::optional<unsigned> idx)
  {
    // If we do not have the right type, bounce onto the slower version
    // which will take care of throwing the appropriate exception. The
    // check will be done again, but this is harmless and only happens
    // when there is actually a mismatch.
    if (!is_a<T>(o))
      type_check(o, T::proto, idx);
    return o->as<T>();
  }

  template<typename T>
  void
  CxxObject::push_initializer_to_back()
  {
    CxxObject::initializers_type &l = CxxObject::initializers_get();
    for (CxxObject::initializers_type::iterator i = l.begin(); i!=l.end(); i++)
    {
      if (CxxObject::TypeInitializer<T>* v =
          dynamic_cast<CxxObject::TypeInitializer<T> *>(*i))
      {
        l.erase(i);
        l.push_back(v);
        break;
      }
    }
  }
}

#endif
