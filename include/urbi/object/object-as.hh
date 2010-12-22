/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_AS_HH
# define OBJECT_AS_HH

#include <boost/type_traits.hpp>

#include <kernel/userver.hh>

#include <urbi/object/duration.hh>
#include <object/uvar.hh>

namespace urbi
{
  namespace object
  {

    /*-------------------.
    | Type conversions.  |
    `-------------------*/

    /* Dynamic_cast based implementation is surprisingly very slow.
    * Implementation using the proto field is possible, but also slow since
    * recursion is needed.
    * So we use a unique magic for each CxxObject type. The magic can b read
    * from a virtual method magic() giving the effective type of an instance,
    * and from a static staticMagic() giving the magic of any known type.
    *
    * We handle multiple layers of inheritance using a TypeList trait that lists
    * all the child classes for each class inheriting CxxObject.
    *
    */

    template<typename T>
    inline bool
    is_a(const rObject& c)
    { // FIXME: update to new system.
      // Atom case.
      if (dynamic_cast<T*>(c.get()))
        return true;
      // Through protos case.
      return is_a(c, T::proto);
    }

    template <typename T>
    bool
    Object::is_a() const
    { // FIXME: update to new system.
      return dynamic_cast<const T*>(this);
    }

    /// CxxObject inheritance traits.

    struct NoParent{};
    // Default
    template<typename T> struct CxxChildClasses
    {
      typedef TYPELIST(T) type;
    };
    // Executable can be Code or Primitive or UVar.
    template<> struct CxxChildClasses<Executable>
    {
      typedef TYPELIST(Executable, Code, Primitive, UVar) type;
    };
    // Primitive can be UVar
    template<> struct CxxChildClasses<Primitive>
    {
      typedef TYPELIST(Primitive, UVar) type;
    };
    // Float can be Duration
    template<> struct CxxChildClasses<Float>
    {
      typedef TYPELIST(Duration, Float) type;
    };
    // Stream can be InputStream or OutputStream (and not stream)
    template<> struct CxxChildClasses<Stream>
    {
      typedef TYPELIST(InputStream, OutputStream) type;
    };

    // Contextual information for TypeList iteration.
    struct ObjectAsContext
    {
      bool result;
      size_t magic;
    };

    // TypeList iteration callback function.
    template <typename T> struct ObjectAsCheck
    {
      void operator()(ObjectAsContext& ctx)
      {
        if (ctx.magic == T::magicStatic())
          ctx.result = true;
      }
    };

    // Function heavily inlined, keep as short as possible (no GD_ !).
    template <typename T>
    libport::intrusive_ptr<T>
    Object::as() const
    {
      ObjectAsContext ctx;
      ctx.result = false;
      ctx.magic = magic();
      libport::meta::typelist::ForEach<ObjectAsContext,
      typename CxxChildClasses<typename boost::remove_const<T> ::type
      >::type,
      ObjectAsCheck>()(ctx);
      if (ctx.result)
        return reinterpret_cast<const T*>(this);
      else
        return 0;
    }

    // Function heavily inlined, keep as short as possible (no GD_ !).
    template <typename T>
    libport::intrusive_ptr<T>
    Object::as()
    {
      ObjectAsContext ctx;
      ctx.result = false;
      ctx.magic = magic();
      libport::meta::typelist::ForEach<ObjectAsContext,
      typename CxxChildClasses<typename boost::remove_const<T> ::type
      >::type,
      ObjectAsCheck>()(ctx);
      if (ctx.result)
        return reinterpret_cast<T*>(this);
      else
        return 0;
    }

    /// Specific overloads for Object and CxxObject.

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

    /* Those are rare and maintaining the list of all CxxObject types
     * is error-prone, so fallback to dynamic_cast.
     */
    template<>
    inline
    libport::intrusive_ptr<CxxObject>
    Object::as<CxxObject>()
    {
      return dynamic_cast<CxxObject*>(this);
    }

    template<>
    inline
    libport::intrusive_ptr<CxxObject>
    Object::as<CxxObject>() const
    {
      return dynamic_cast<CxxObject*>(const_cast<Object*>(this));
    }
  }
}
#endif
