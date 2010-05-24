/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/code-class.cc
 ** \brief Creation of the Urbi object code.
 */

#include <libport/lexical-cast.hh>

#include <ast/parametric-ast.hh>
#include <ast/print.hh>

#include <kernel/userver.hh>

#include <urbi/object/code.hh>
#include <urbi/object/list.hh>
#include <urbi/object/string.hh>
#include <urbi/object/object.hh>
#include <object/symbols.hh>

#include <urbi/runner/raise.hh>
#include <runner/runner.hh>

namespace urbi
{
  namespace object
  {
    Code::Code(ast_type a)
      : ast_(a)
      , captures_()
      , self_()
      , call_()
      , lobby_()
    {
      proto_add(proto);
      proto_remove(Object::proto);
    }

    Code::Code(rCode model)
      : ast_(model->ast_)
      , captures_(model->captures_)
      , self_(model->self_)
      , call_(model->call_)
      , lobby_(model->lobby_)
    {
      proto_add(model);
      proto_remove(Object::proto);
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

    rLobby Code::lobby_get() const
    {
      return lobby_;
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

    rLobby& Code::lobby_get()
    {
      return lobby_;
    }

    void Code::self_set(rObject v)
    {
      self_ = v;
    }

    rObject Code::apply(const objects_type& apply_args)
    {
      check_arg_count(apply_args.size(), 1, 2);
      rList args = type_check<List>(apply_args[0], 0u);
      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
      libport::Symbol s =
        1 < apply_args.size()
        ? libport::Symbol(type_check<String>(apply_args[1], 1u)->value_get())
        : SYMBOL(apply);
      if (args->value_get().empty())
        RAISE("argument list must begin with `this'");
      List::value_type a = args->value_get();

      return r.apply(this, s, a);
    }

    std::string Code::as_string(rObject what)
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
    Code::special_slots_dump(std::ostream& o) const
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
    Code::operator() (object::objects_type args)
    {
      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
      aver(!args.empty());
      return r.apply(this, libport::Symbol::make_empty(), args);
    }


    URBI_CXX_OBJECT_REGISTER(Code)
    {
      PARAMETRIC_AST(ast, "function () {}");
      ast_ = ast.result<const ast::Routine>();
      proto_add(Executable::proto);
      proto_remove(Object::proto);
    }

  } // namespace object
}
