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
#include <runner/job.hh>
#include <eval/call.hh>

GD_CATEGORY(Urbi.Object);

namespace urbi
{
  namespace object
  {
    Primitive::Primitive()
    : default_arity_(-1)
    {
      proto_add(proto);
    }

    Primitive::Primitive(rPrimitive model)
      : content_(model->value_get())
      , default_()
      , default_arity_(-1)
    {
      proto_add(proto);
      proto_remove(Object::proto);
    }

    Primitive::Primitive(const value_type& p)
      : content_()
      , default_(p)
      , default_arity_(-1)
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
      , default_()
    {
      Ward w(this);
      // Hack to avoid proto = 0,
      // proto will redefined after.
      proto = this;
      extend(nil);
      if (!Executable::proto)
        abort();
      proto_add(Executable::proto);
      proto_remove(Object::proto);

      BIND(apply);
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
      return
        eval::call_apply(::kernel::runner(), this, SYMBOL(apply), a);
    }

    rObject Primitive::operator() (object::objects_type args)
    {
      return call_raw(args);
    }


    rObject Primitive::call_raw(const object::objects_type& args,
      unsigned flags)
    {
      //flags = CALL_IGNORE_EXTRA_ARGS | CALL_IGNORE_MISSING_ARGS;
      size_t arity = args.size();
      if (default_
        && ( (size_t)default_arity_ == arity
          || (arity == 1 && default_arity_ == 0)))
        return default_(args);
      else if (libport::has(content_, arity))
        return content_[arity](args);
      // Self is always present, accept it even for 0-arity.
      else if (arity == 1 && libport::has(content_, 0))
        return content_[0](args);
      else if (default_ && default_arity_ == -1)
        return default_(args);

      // error handing
      size_t min = INT_MAX;
      size_t max = 0;
      size_t minAbove = INT_MAX;
      size_t maxBelow = 0;
      // FIXME: the valid arities is not necessarily a range.
      foreach (const Primitive::values_type::value_type& elt, content_)
      {
        if (elt.first >= arity)
          minAbove = std::min(minAbove, elt.first);
        if (elt.first <= arity)
          maxBelow = std::max(maxBelow, elt.first);
        min = std::min(min, elt.first);
        max = std::max(max, elt.first);
      }
      if (default_arity_ != -1)
      {
        minAbove = std::min(minAbove, (size_t)default_arity_);
        maxBelow = std::max(maxBelow, (size_t)default_arity_);
      }
      if ( (flags & CALL_IGNORE_EXTRA_ARGS) && maxBelow)
      {
        object::objects_type cp(args.begin(), args.begin() + maxBelow);
        return ((maxBelow == (size_t)default_arity_)?default_:content_[maxBelow])(cp);
      }
      if ((flags & CALL_IGNORE_MISSING_ARGS) && minAbove < INT_MAX)
      {
         object::objects_type cp = args;
         for (unsigned i = 0; i< minAbove - args.size(); ++i)
           cp.push_back(nil_class);
         return ((minAbove == (size_t)default_arity_)?default_:content_[minAbove])(cp);
      }
      if (min == max)
      {
        GD_FINFO_TRACE("arity error for primitive: expected %s: %s",
                       min, arity);
        runner::raise_arity_error(arity - 1, min - 1);
      }
      else
      {
        GD_FINFO_TRACE("arity error for primitive: "
                       "expected between %s and %s: %s",
                       min, max, arity);
        runner::raise_arity_error(arity - 1, min - 1, max - 1);
      }
      unreachable();
    }
  }; // namespace object
}
