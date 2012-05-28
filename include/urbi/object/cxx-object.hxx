/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef CXX_OBJECT_HXX
# define CXX_OBJECT_HXX

# include <boost/function.hpp>

# include <libport/bind.hh>

# include <urbi/object/cxx-conversions.hh>
# include <urbi/object/cxx-primitive.hh>

# include <urbi/object/primitive.hh>
# include <urbi/object/string.hh>
# include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {

    inline void
    check_arg_count(unsigned effective, unsigned formal)
    {
      if (formal != effective)
        runner::raise_arity_error(effective, formal);
    }

    inline void
    check_arg_count(unsigned effective, unsigned min, unsigned max)
    {
      if (min == max)
        check_arg_count(effective, min);
      else if (effective < min || max < effective)
        runner::raise_arity_error(effective, min, max);
    }

    inline void
    check_arg_count(const objects_type& args, unsigned formal)
    {
      check_arg_count(args.size() - 1, formal);
    }

    inline void
    check_arg_count(const objects_type& args, unsigned min, unsigned max)
    {
      check_arg_count(args.size() - 1 , min, max);
    }

    namespace
    {
      template <typename T>
      rObject cxx_object_clone(objects_type args)
      {
        check_arg_count(args, 0);
        rObject tgt = args[0];
        aver(tgt->as<T>());
        libport::intrusive_ptr<T> res = new T(tgt->as<T>());
        // If the user didn't specify a prototype, use the model.
        if (!res->protos_has())
          res->proto_add(tgt);
        return res;
      }

      template <typename T>
      rObject cxx_object_id(objects_type args)
      {
        check_arg_count(args, 0);
        return args[0];
      }
    }

    template <typename T>
    void CxxObject::add(const std::string& ns)
    {
      using boost::bind;

      std::string name = (ns.empty() ? "" : ns + ".") + T::type_name();
      rObject dest = resolve_namespace(name);

      GD_CATEGORY(Urbi.CxxObject);

      if (ns.empty())
        GD_FINFO_TRACE("register C++ object %s", name);
      else
        GD_FINFO_TRACE("register C++ object %s as %s", name, ns);

      libport::intrusive_ptr<T> res;
      if (!T::proto)
      {
        res = new T(FirstPrototypeFlag());
        T::proto = res;
      }
      else
        res = T::proto;

      // If the user didn't specify a prototype, use Object.
      if (!res->protos_has())
        res->proto_add(Object::proto);
      aver(res);

      // type.
      static libport::Symbol type("type");
      if (!res->local_slot_get(type))
        res->slot_set_value(type,
                       new String(name), true);
      // clone.
      static libport::Symbol clone("clone");
      res->slot_set_value(clone,
                    new Primitive(bind(cxx_object_clone<T>, _1)), true);

      // asFoo.
      libport::Symbol as(std::string("as") + name);
      if (!res->slot_get_value(as, false))
        res->slot_set_value(as, new Primitive(bind(cxx_object_id<T>, _1)),
                            true);
      dest->slot_set_value(libport::Symbol(name), res);
    }

    // BINDER
    template <typename T>
    CxxObject::Binder<T>::Binder(rObject tgt)
      : tgt_(tgt)
    {}

    template <typename T>
    template <typename M>
    void
    CxxObject::Binder<T>::operator()(libport::Symbol name,
                                     M method)
    {
      tgt_->slot_set_value(name, primitive(method), true);
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

    template <typename C, typename T>
    static rObject
    refgetter(libport::intrusive_ptr<C> self, T* (C::* ref)())
    {
      return to_urbi(*((self.get()->*ref)()));
    }

    template <typename C, typename T>
    static rObject
    refsetter(T* (C::* ref)(), libport::intrusive_ptr<C> self,
           const std::string&, const T& value)
    {
      *((self.get()->*ref)()) = value;
      return 0;
    }

    template<typename T>
    inline libport::intrusive_ptr<T>
    type_check(const rObject& o)
    {
      if (T* res = o->as<T>())
        return res;
      // If we do not have the right type, bounce onto the slower version
      // which will take care of throwing the appropriate exception. The
      // check will be done again, but this is harmless and only happens
      // when there is actually a mismatch.
      type_check(o, T::proto);
      unreachable();
    }
  }
}

#endif
