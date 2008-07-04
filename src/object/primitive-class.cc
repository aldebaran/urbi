/**
 ** \file object/primitive-class.cc
 ** \brief Creation of the URBI object primitive.
 */

#include <object/atom.hh>
#include <object/list-class.hh>
#include <object/object.hh>
#include <object/primitive-class.hh>

#include <runner/runner.hh>

namespace object
{
  rObject primitive_class;

  Primitive::Primitive()
  {
    pabort("You can't instanciate Primitives by hand!");
  }

  Primitive::Primitive(rPrimitive model)
    : content_(model->value_get())
  {
    proto_add(primitive_class);
  }

  Primitive::Primitive(value_type value)
    : content_(value)
  {
    proto_add(primitive_class);
  }

  Primitive::value_type Primitive::value_get() const
  {
    return content_;
  }

  const std::string Primitive::type_name = "Primitive";
  std::string Primitive::type_name_get() const
  {
    return type_name;
  }

  void Primitive::initialize(CxxObject::Binder<Primitive>& bind)
  {
    bind(SYMBOL(apply), &Primitive::apply);
  }

  bool Primitive::primitive_added =
    CxxObject::add<Primitive>("Primitive", primitive_class);

  // FIXME: Code duplication with Code::apply.  Maybe there are more
  // opportunity to factor.
  rObject
  Primitive::apply(runner::Runner& r, rList args)
  {
    if (args->value_get().empty())
      throw PrimitiveError(SYMBOL(apply),
                           "list of arguments must begin with this");
    return r.apply(this, SYMBOL(apply), args->value_get());
  }

}; // namespace object
