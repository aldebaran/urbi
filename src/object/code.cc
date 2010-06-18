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
 ** \file object/code.cc
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
    using ::kernel::runner;

    Code::Code(ast_type a,
               rObject call, rLobby lobby, rObject ths,
               captures_type captures)
      : ast_(a)
      , call_(call)
      , captures_(captures)
      , lobby_(lobby)
      , this_(ths)
    {
      proto_add(proto);
      proto_remove(Object::proto);
    }

    Code::Code(rCode model)
      : ast_(model->ast_)
      , call_(model->call_)
      , captures_(model->captures_)
      , lobby_(model->lobby_)
      , this_(model->this_)
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

    void Code::lobby_set(const rLobby& l)
    {
      lobby_ = l;
    }

    rObject Code::this_get() const
    {
      return this_;
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

    void Code::this_set(rObject v)
    {
      this_ = v;
    }

    rObject Code::apply(const objects_type& apply_args)
    {
      check_arg_count(apply_args.size(), 1, 2);
      rList args = type_check<List>(apply_args[0], 0u);
      runner::Runner& r = runner();
      libport::Symbol s =
        1 < apply_args.size()
        ? libport::Symbol(type_check<String>(apply_args[1], 1u)->value_get())
        : SYMBOL(apply);
      if (args->value_get().empty())
        RAISE("argument list must begin with `this'");
      List::value_type a = args->value_get();

      return r.apply(this, s, a);
    }

    std::string
    Code::as_string() const
    {
      return string_cast(*ast_);
    }

    std::string
    Code::body_string() const
    {
      if (proto == this)
        return SYMBOL(LT_Code_GT);
      return
        string_cast(*ast_->body_get()->body_get());
    }

    std::ostream&
    Code::special_slots_dump(std::ostream& o) const
    {
#define DISP(Attr)                                                      \
      << #Attr " = " << libport::deref << Attr ## _get() << libport::iendl
      return o
        DISP(ast)
        DISP(call)
        DISP(this)
        DISP(lobby)
        DISP(captures);
#undef DISP
    }

    bool
    Code::operator==(const Code& that) const
    {
#define EQ(Attr)                                \
      (Attr ## _get() == that.Attr ## _get())
      return (EQ(captures)
              && EQ(this)
              && EQ(call)
              && EQ(lobby)
              && as_string() == that.as_string());
#undef EQ
    }

    bool
    Code::operator==(const rObject& that) const
    {
      return (that->is_a<Code>()
              && *this == *that->as<Code>());
    }

    void Code::initialize(CxxObject::Binder<Code>& bind)
    {
      bind(SYMBOL(EQ_EQ),
           static_cast<bool (self_type::*)(const rObject&) const>
           (&self_type::operator==));
      bind(SYMBOL(apply), &Code::apply);
      bind(SYMBOL(asString), &Code::as_string);
      bind(SYMBOL(bodyString), &Code::body_string);
    }

    rObject
    Code::operator() (object::objects_type args)
    {
      runner::Runner& r = runner();
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
