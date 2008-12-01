/**
 ** \file object/code-class.cc
 ** \brief Creation of the URBI object code.
 */

#include <libport/lexical-cast.hh>

#include <ast/function.hh>
#include <ast/parametric-ast.hh>
#include <ast/print.hh>

#include <object/code.hh>
#include <object/list.hh>
#include <object/string.hh>
#include <object/object.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>

namespace object
{
  Code::Code(ast_type a)
    : ast_(a)
    , captures_()
    , self_()
    , call_()
  {
    proto_add(proto ? proto : object_class);
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
      runner::raise_primitive_error("list of arguments "
				    "must begin with `this'");
    List::value_type a = args->value_get();
    rObject tgt = a.front();
    a.pop_front();
    return r.apply(tgt, this, SYMBOL(apply), a);
  }

  std::string Code::as_string(runner::Runner&, rObject what)
  {
    type_check<Code>(what);
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

  void Code::initialize(CxxObject::Binder<Code>& bind)
  {
    bind(SYMBOL(apply), &Code::apply);
    bind(SYMBOL(asString), &Code::as_string);
    bind(SYMBOL(bodyString), &Code::body_string);
  }

  rObject
  Code::proto_make()
  {
    PARAMETRIC_AST(ast, "function () {}");
    return new Code(ast.result<const ast::Routine>());
  }

  rObject
  Code::operator() (runner::Runner& r, object::objects_type args)
  {
    assert(!args.empty());
    rObject self = args[0];
    args.pop_front();
    return r.apply(self, this, libport::Symbol::make_empty(), args);
  }


  URBI_CXX_OBJECT_REGISTER(Code);
}; // namespace object
