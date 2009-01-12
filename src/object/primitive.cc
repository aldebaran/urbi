/**
 ** \file object/primitive-class.cc
 ** \brief Creation of the URBI object primitive.
 */

#include <object/list.hh>
#include <object/object.hh>
#include <object/primitive.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>

namespace object
{
  Primitive::Primitive()
  {
    runner::raise_primitive_error("`Primitive' objects cannot be cloned");
  }

  Primitive::Primitive(rPrimitive model)
    : content_(model->value_get())
  {
    proto_add(proto);
  }

  Primitive::Primitive(value_type value)
    : content_(value)
  {
    proto_add(proto ? proto : object_class);
  }

  Primitive::value_type Primitive::value_get() const
  {
    return content_;
  }

  void Primitive::initialize(CxxObject::Binder<Primitive>& bind)
  {
    bind(SYMBOL(apply), &Primitive::apply);
  }

  URBI_CXX_OBJECT_REGISTER(Primitive);

  // FIXME: Code duplication with Code::apply.  Maybe there are more
  // opportunity to factor.
  rObject
  Primitive::apply(rList args)
  {
    if (args->value_get().empty())
      runner::raise_primitive_error("list of arguments "
				    "must begin with `this'");
    objects_type a = args->value_get();
    rObject tgt = a.front();
    a.pop_front();
    return ::urbiserver->getCurrentRunner().apply(tgt, this, SYMBOL(apply), a);
  }

  rObject Primitive::operator() (object::objects_type args)
  {
    return content_(args);
  }

  static void nil()
  {}

  rObject
  Primitive::proto_make()
  {
    rObject res = make_primitive(nil);
    return res;
  }

}; // namespace object
