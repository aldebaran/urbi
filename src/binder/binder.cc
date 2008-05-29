/**
 ** \file binder/binder.cc
 ** \brief Implementation of binder::Binder.
 */

#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <libport/foreach.hh>

#include "binder/binder.hh"
#include "object/symbols.hh"
#include "object/object.hh"

namespace binder
{
  Binder::Binder()
    : env_()
    , unbind_()
    , depth_(1)
  {
    unbind_.push_back(libport::Finally());
    setOnSelf_.push_back(true);
  }

  Binder::~Binder()
  {}

  static inline
  boost::optional<libport::Symbol> getFirstArg(ast::rConstCall call)
  {
    ast::rConstAst arg1 = *++call->args_get().begin();
    if (!arg1.unsafe_cast<const ast::String>())
      return boost::optional<libport::Symbol>();
    return libport::Symbol(arg1.unsafe_cast<const ast::String>()->value_get());
  }

  int Binder::isLocal(const libport::Symbol& name)
  {
    if (env_[name].empty())
      return 0;
    else
    {
      assert(env_[name].back().second > 0);
      return env_[name].back().second;
    }
  }

  ast::rCall Binder::changeSlot (const ast::loc& l,
                                 const libport::Symbol& name,
                                 const libport::Symbol& method,
                                 ast::rConstExp value)
  {
      ast::exps_type* args = new ast::exps_type();
      args->push_back(new ast::Implicit(l));
      args->push_back(new ast::String(l, name));
      super_type::operator() (value);
      args->push_back(result_.unsafe_cast<ast::Exp>());
      return new ast::Call(l, method, args);
  }

  void Binder::visit(ast::rConstAssignment input)
  {
    int local = isLocal(input->what_get());
    if (local == depth_)
      super_type::visit(input);
    else
    {
      ast::rCall res = changeSlot(input->location_get(),
                                  input->what_get(),
                                  SYMBOL(updateSlot),
                                  input->value_get());
      if (local)
        targetContext(res, depth_ - local);
      else
        targetSelf(res);
      result_ = res;
    }
  }

  void Binder::visit(ast::rConstDeclaration input)
  {
    if (setOnSelf_.back())
    {
      ast::rCall res = changeSlot(input->location_get(),
                                  input->what_get(),
                                  SYMBOL(setSlot),
                                  input->value_get());
      targetSelf(res);
      result_ = res;
    }
    else
    {
      bind(input->what_get(), input);
      super_type::visit(input);
    }
  }

  void Binder::visit (ast::rConstCall input)
  {
    const ast::exps_type& args = input->args_get();
    if (args.size() == 1
        && args.front()->implicit()
        && isLocal(input->name_get()) == depth_)
    {
      result_ = new ast::Local(input->location_get(),
                               input->name_get());
      return;
    }
    super_type::visit (input);
    ast::rCall call = result_.unsafe_cast<ast::Call>();
    libport::Symbol name = call->name_get();
    // If this is a qualified call, nothing particular to do
    if (call->target_implicit())
    {
      if (name == SYMBOL(call)
          || name == SYMBOL(locals)
          || name == SYMBOL(self))
      {
        retarget(call, name);
        return;
      }
      else if (name == SYMBOL(getSlot)
               || name == SYMBOL(locateSlot)
               || name == SYMBOL(updateSlot)
               || name == SYMBOL(removeSlot))
      {
        if (boost::optional<libport::Symbol> var = getFirstArg(call))
          retarget(call, var.get());
      }
      else
        retarget(call, call->name_get());
    }
  }

  void Binder::retarget(ast::rCall call, const libport::Symbol& var)
  {
    if (var == SYMBOL(call)
        || var == SYMBOL(locals)
        || var == SYMBOL(self))
      return;
    if (int depth = isLocal(var))
    {
      if (depth < depth_)
        targetContext(call, depth_ - depth);
    }
    else
      targetSelf(call);
  }

  void Binder::visit (ast::rConstForeach input)
  {
    bind(input->index_get(), input);
    super_type::visit(input);
  }

  void Binder::targetSelf(ast::rCall call)
  {
    call->args_get().pop_front();
    ast::exps_type* args = new ast::exps_type;
    args->push_back(new ast::Implicit(call->location_get()));
    args->push_back(new ast::String(call->location_get(), "self"));
    ast::rCall self = new ast::Call(call->location_get(), SYMBOL(getSlot), args);
    call->args_get().push_front(self);
  }

  void Binder::targetContext(ast::rCall call, int depth)
  {
    call->args_get().pop_front();
    ast::rCall code;
    {
      ast::exps_type* args = new ast::exps_type;
      args->push_back(new ast::Implicit(call->location_get()));
      args->push_back(new ast::String(call->location_get(), "code"));
      code = new ast::Call(call->location_get(), SYMBOL(getSlot), args);
    }
    ast::rCall context;
    {
      ast::exps_type* args = new ast::exps_type;
      args->push_back(code);
      context = new ast::Call(call->location_get(), SYMBOL(context), args);
    }
    if (depth > 1)
      targetContext(code, depth - 1);
    call->args_get().push_front(context);
  }

  void Binder::visit (ast::rConstScope input)
  {
    result_ = new ast::Scope(input->location_get(), handleScope(input, false));
  }

  void Binder::visit (ast::rConstDo input)
  {
    operator() (input->target_get());
    ast::rExp target = result_.unsafe_cast<ast::Exp>();
    result_ = new ast::Do(input->location_get(),
                          handleScope(input, true),
                          target);
  }

  ast::rExp Binder::handleScope(ast::rConstAbstractScope scope, bool setOnSelf)
  {
    libport::Finally finally;

    // Push a finally on unbind_, and destroy it at the scope
    // exit. Since bound variables register themselves for unbinding
    // in unbind_'s top element, they will be unbound at scope exit.
    unbind_.push_back(libport::Finally());
    finally << boost::bind(&std::list<libport::Finally>::pop_back, &unbind_);

    setOnSelf_.push_back(setOnSelf);
    finally << boost::bind(&std::list<bool>::pop_back, &setOnSelf_);

    operator() (scope->body_get());
    return result_.unsafe_cast<ast::Exp>();
  }

  void Binder::visit(ast::rConstFunction input)
  {
    depth_++;
    if (input->formals_get())
    {
      foreach (const libport::Symbol& arg, *input->formals_get())
	bind(arg, input);
    }
    super_type::visit (input);
    depth_--;
  }

  void Binder::visit(ast::rConstClosure input)
  {
    if (input->formals_get())
    {
      foreach (const libport::Symbol& arg, *input->formals_get())
	bind(arg, input);
    }
    super_type::visit(input);
  }

  void Binder::bind(const libport::Symbol& var, ast::rConstAst decl)
  {
    env_[var].push_back(std::make_pair(decl, depth_));
    unbind_.back() <<
      boost::bind(&Bindings::pop_back, &env_[var]);
  }

} // namespace binder
