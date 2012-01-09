/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/primitive.cc
 ** \brief Creation of the Urbi object primitive.
 */

#include <urbi/kernel/userver.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>
#include <urbi/object/primitive.hh>
#include <urbi/object/symbols.hh>
#include <urbi/runner/raise.hh>
#include <runner/runner.hh>

GD_CATEGORY(Urbi);

namespace urbi
{
  namespace object
  {
    Primitive::Primitive()
    {
      proto_add(proto);
    }

    Primitive::Primitive(rPrimitive model)
      : content_(model->value_get())
    {
      proto_add(proto);
      proto_remove(Object::proto);
    }

    Primitive::Primitive(const value_type& p)
      : default_(p)
    {
      if (!proto)
        proto = new Primitive(FirstPrototypeFlag());
      proto_add(proto);
    }

    static rObject nil(const objects_type&)
    {
      return void_class;
    }

    URBI_CXX_OBJECT_INIT(Primitive)
      : content_()
    {
      extend(nil);
      proto_add(Executable::proto);
      proto_remove(Object::proto);

      // Hack to avoid proto = 0,
      // proto will redefined after.
      proto = this;
      bind(SYMBOL(apply), &Primitive::apply);
    }

    const Primitive::values_type&
    Primitive::value_get() const
    {
      return content_;
    }

    Primitive::values_type&
    Primitive::value_get()
    {
      return content_;
    }
    // FIXME: Code duplication with Code::apply.  Maybe there are more
    // opportunity to factor.
    rObject
    Primitive::apply(rList args)
    {
      if (args->value_get().empty())
        RAISE("list of arguments must begin with `this'");
      objects_type a = args->value_get();
      return (::kernel::runner()
              .apply(this, SYMBOL(apply), a));
    }

    rObject Primitive::operator() (object::objects_type args)
    {
      return call_raw(args);
    }



    rObject Primitive::call_raw(const object::objects_type& args)
    {
      size_t arity = args.size();
      if (!libport::mhas(content_, arity))
      {
        // Self is always present, accept it even for 0-arity.
        if (arity == 1 && libport::mhas(content_, 0))
          return content_[0](args);
        if (default_)
          return default_(args);

        size_t min = INT_MAX;
        size_t max = 0;

        // FIXME: the valid arity range is not necessarilly continue
        foreach (const Primitive::values_type::value_type& elt, content_)
        {
          min = std::min(min, elt.first);
          max = std::max(max, elt.first);
        }
        if (min == max)
        {
          GD_FINFO_TRACE("arity error for primitive: expected %s, got %s.",
                         min, arity);
          runner::raise_arity_error(arity - 1, min - 1);
        }
        else
        {
          GD_FINFO_TRACE("arity error for primitive: "
                         "expected between %s and %s, got %s.",
                         min, max, arity);
          runner::raise_arity_error(arity - 1, min - 1, max - 1);
        }
      }
      return content_[arity](args);
    }
  }; // namespace object
}
