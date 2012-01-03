/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file urbi/object/object.hxx
 ** \brief Definition of object::Object.
 */

#ifndef OBJECT_OBJECT_HXX
# define OBJECT_OBJECT_HXX

# include <ostream>
# include <typeinfo>

# include <boost/bind.hpp>
# include <boost/tr1/type_traits.hpp>

# include <libport/containers.hh>
# include <libport/debug.hh>
# include <libport/foreach.hh>
# include <libport/indent.hh>
# include <libport/intrusive-ptr.hh>
# include <libport/typelist.hh>

# include <urbi/object/object.hh>

# include <urbi/kernel/userver.hh>

# include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {
    /*---------.
    | Protos.  |
    `---------*/

    inline
    bool
    Object::protos_has() const
    {
      return proto_ || (protos_ && !protos_->empty());
    }

    inline
    rObject
    Object::protos_get_first() const
    {
      if (proto_)
        return proto_;
      else if (protos_ && !protos_->empty())
        return protos_->front();
      else
        return rObject();
    }

    inline
    Object&
    Object::unsafe_proto_add(const rObject& v)
    {
      if (protos_)
        protos_->push_front(v);
      else
        if (!proto_)
          proto_ = v;
        else
        {
          protos_ = new protos_type;
          protos_->push_back(v);
          protos_->push_back(proto_);
          proto_ = 0;
        }
      return *this;
    }

    inline
    Object&
    Object::proto_remove(const rObject& p)
    {
      aver(p);
      if (!protos_)
      {
        if (proto_ == p)
          proto_ = 0;
      }
      else
        for (protos_type::iterator i = protos_->begin();
           i != protos_->end();
           /* nothing */)
          if (*i == p)
            i = protos_->erase(i);
          else
            ++i;
      return *this;
    }

    /*--------.
    | Slots.  |
    `--------*/

    inline
    bool
    Object::slot_remove(key_type k)
    {
      return slots_.erase(this, k);
    }

    inline
    const Object::slots_implem&
    Object::slots_get() const
    {
      return slots_;
    }


    /*--------.
    | Clone.  |
    `--------*/

    inline rObject
    Object::clone() const
    {
      rObject res = new Object;
      // FIXME: clone should not be const!  This was already noted,
      // did the const come back at some point?
      res->proto_add(const_cast<Object*>(this));
      return res;
    }


    /*--------------------------.
    | Free standing functions.  |
    `--------------------------*/

    template<class F> bool
    for_all_protos(const rObject& r, F& f, Object::objects_set_type& objects)
    {
      if (libport::has(objects, r))
        return false;
      if (f(r))
        return true;
      objects.insert(r);
      if (r->protos_)
      {
        foreach(const rObject& p, *r->protos_)
          if (for_all_protos(p, f, objects))
            return true;
      }
      else if (r->proto_)
        if (for_all_protos(r->proto_, f, objects))
          return true;
      return false;
    }
    template<class F> bool
    for_all_protos(const rObject& r, F f)
    {
      Object::objects_set_type objects;
      return for_all_protos(r, f, objects);
    }

    inline const rObject&
    to_boolean(bool b)
    {
      return b ? true_class : false_class;
    }

    /*---------------.
    | Binding system |
    `---------------*/

    template <bool mem, typename T>
    struct DispatchBind_
    {
      static void
      res(Object* o, const std::string& name, T p)
      {
        o->bindfun_(name, p);
      }
    };

    template <typename T>
    struct DispatchBind_<true, T>
    {
      static void
      res(Object* o, const std::string& name, T p)
      {
        o->bindvar_(name, p);
      }
    };

    template <typename T>
    inline void
    Object::bind(const std::string& name, T p)
    {
      DispatchBind_<std::tr1::is_member_object_pointer<T>::value, T>
        ::res(this, name, p);
    }

    template <typename T>
    inline void
    Object::bindfun_(const std::string& name, T p)
    {
      GD_CATEGORY(Urbi);

      libport::Symbol sym(name);
      if (!local_slot_get(sym))
      {
        GD_FINFO_DEBUG("create primitive %s with C++ routine (type: %s)",
                       name, typeid(T).name());
        slot_set(sym, primitive(p), true);
      }
      else
      {
        GD_FINFO_DEBUG("extend primitive %s with C++ routine (type: %s)",
                       name, typeid(T).name());
        rSlot v = local_slot_get(sym);
        rPrimitive prim = (*v)->as<Primitive>();
        assert(prim);
        primitive(prim, p);

      }
    }

    template <typename Self, typename T>
    T bindvar_getter_(Self& self, T (Self::*Attr))
    {
      return self.*Attr;
    }

    template <typename T>
    struct Ref
    {
      typedef T& res;
    };

    template <typename T>
    struct Ref<T*>
    {
      typedef T* res;
    };

    template <typename Self, typename T>
    T bindvar_setter_(typename Ref<Self>::res  self, const std::string&, T val, T (Self::*Attr))
    {
      return self.*Attr = val;
    }

    template <typename Self, typename T>
    inline void
    Object::bindvar_(const std::string& name, T (Self::*attr))
    {
      boost::function1<T, Self&> getter(boost::bind(&bindvar_getter_<Self, T>, _1, attr));
      boost::function3<void, Self&, const std::string&, T> setter(boost::bind(&bindvar_setter_<Self, T>, _1, _2, _3, attr));
      bind(libport::Symbol(name), getter);
      setProperty(name, "updateHook", primitive(setter));
    }

    bool
    Object::as_check_(const std::type_info* req)
    {
      return req == &typeid(Object);
    }


    // Function heavily inlined, keep as short as possible (no GD_ !).
    template <typename T>
    libport::intrusive_ptr<T>
    Object::as() const
    {
      return const_cast<Object*>(this)->as<T>();
    }

    template <typename T>
    libport::intrusive_ptr<T>
    Object::as()
    {
      return reinterpret_cast<T*>(as_dispatch_(&typeid(T)));
    }

    // Function heavily inlined, keep as short as possible (no GD_ !).
    template <typename T>
    libport::intrusive_ptr<T>
    Object::as_checked() const
    {
      return const_cast<Object*>(this)->as_checked<T>();
    }

    template <typename T>
    libport::intrusive_ptr<T>
    Object::as_checked()
    {
      T* v = reinterpret_cast<T*>(as_dispatch_(&typeid(T)));
      if (!v)
        runner::raise_type_error(this, T::proto);
      return v;
    }

    // Specializations for object.

    template<>
    inline
    rObject
    Object::as<Object>()
    {
      return this;
    }

    template<>
    inline
    rObject
    Object::as<Object>() const
    {
      return const_cast<Object*>(this);
    }

    template<>
    inline
    libport::intrusive_ptr<const Object>
    Object::as<const Object>()
    {
      return this;
    }

    template<>
    inline
    libport::intrusive_ptr<const Object>
    Object::as<const Object>() const
    {
      return const_cast<Object*>(this);
    }
  } // namespace object
}

#endif // !OBJECT_OBJECT_HXX
