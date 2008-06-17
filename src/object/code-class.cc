/**
 ** \file object/code-class.cc
 ** \brief Creation of the URBI object code.
 */

#include <libport/lexical-cast.hh>

#include <ast/function.hh>
#include <ast/print.hh>

#include <object/atom.hh>
#include <object/code-class.hh>
#include <object/object.hh>

#include <runner/runner.hh>

namespace object
{
  rObject code_class;

  Code::Code()
  {
    pabort("You can't instanciate Code manually.");
  }

  Code::Code(ast_type a)
    : ast_(a)
    , captures_()
    , self_()
    , call_()
  {
    assert(code_class);
    proto_add(code_class);
  }

  Code::Code(rCode model)
    : ast_(model->ast_)
    , captures_(model->captures_)
    , self_(model->self_)
    , call_(model->call_)
  {
    assert(code_class);
    proto_add(code_class);
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
      throw PrimitiveError ("apply", "list of arguments must begin with self");
    return r.apply (this, SYMBOL(apply), args->value_get());
  }

  rString Code::as_string()
  {
    if (code_class == this)
      return new String(SYMBOL(LT_Code_GT));
    return new String(libport::Symbol(string_cast(*ast_)));

  }

  rString Code::body_string()
  {
    if (code_class == this)
      return new String(SYMBOL(LT_Code_GT));
    return
      new String(
        libport::Symbol(
          string_cast(*ast_->body_get()->body_get())));
  }

  void Code::initialize(CxxObject::Binder<Code>& bind)
  {
    bind(SYMBOL(apply), &Code::apply);
    bind(SYMBOL(asString), &Code::as_string);
    bind(SYMBOL(bodyString), &Code::body_string);
  }

  std::string Code::type_name_get() const
  {
    return type_name;
  }

  bool Code::code_added = CxxObject::add<Code>("Code", code_class);
  const std::string Code::type_name = "Code";

}; // namespace object
