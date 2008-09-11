/**
 ** \file object/code-class.cc
 ** \brief Creation of the URBI object code.
 */

#include <libport/lexical-cast.hh>

#include <ast/function.hh>
#include <ast/print.hh>

#include <object/code.hh>
#include <object/list.hh>
#include <object/string.hh>
#include <object/object.hh>
#include <object/urbi-exception.hh>

#include <runner/runner.hh>

namespace object
{
  Code::Code()
  {
    throw PrimitiveError(SYMBOL(clone),
			 "`Code' objects cannot be cloned");
  }

  Code::Code(ast_type a)
    : ast_(a)
    , captures_()
    , self_()
    , call_()
  {
    assert(proto);
    proto_add(proto);
  }

  Code::Code(rCode model)
    : ast_(model->ast_)
    , captures_(model->captures_)
    , self_(model->self_)
    , call_(model->call_)
  {
    proto_add(model);
  }

  Code::ast_type Code::ast_get() const
  {
    return ast_;
  }

  rObject Code::call_get() const
  {
    return call_;
  }

  const Code::captures_type& Code::captures_get() const
  {
    return captures_;
  }

  rObject Code::self_get() const
  {
    return self_;
  }

  Code::ast_type& Code::ast_get()
  {
    return ast_;
  }

  rObject& Code::call_get()
  {
    return call_;
  }

  Code::captures_type& Code::captures_get()
  {
    return captures_;
  }

  rObject& Code::self_get()
  {
    return self_;
  }

  rObject Code::apply(runner::Runner& r, rList args)
  {
    if (args->value_get().empty())
      throw PrimitiveError(SYMBOL(apply),
                           "list of arguments must begin with this");
    List::value_type a = args->value_get();
    rObject tgt = a.front();
    a.pop_front();
    return r.apply(tgt, this, SYMBOL(apply), a);
  }

  std::string Code::as_string(runner::Runner& r, rObject what)
  {
    if (what == proto)
      return SYMBOL(LT_Code_GT);
    type_check(what, proto, r, SYMBOL(asString));
    return string_cast(*what->as<Code>()->ast_get());

  }

  std::string Code::body_string()
  {
    if (proto == this)
      return SYMBOL(LT_Code_GT);
    return
      string_cast(*ast_->body_get()->body_get());
  }

  std::ostream&
  Code::special_slots_dump(std::ostream& o, runner::Runner&) const
  {
    return o << "value = " << *ast_get() << libport::iendl;
  }


  template <typename T>
  void blerg()
  {
    T::gnark;
  }

  template <typename M>
  void blah(M)
  {
    blerg<typename AnyToBoostFunction<M>::type>();
  }

  void Code::initialize(CxxObject::Binder<Code>& bind)
  {
    bind(SYMBOL(apply), &Code::apply);
//     blah(&Code::as_string);
    bind(SYMBOL(asString), &Code::as_string);
    bind(SYMBOL(bodyString), &Code::body_string);
  }

  std::string Code::type_name_get() const
  {
    return type_name;
  }

  bool Code::code_added = CxxObject::add<Code>("Code", Code::proto);
  const std::string Code::type_name = "Code";
  rObject Code::proto;

}; // namespace object
