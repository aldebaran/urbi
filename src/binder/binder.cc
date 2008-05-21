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
  Binder::Binder(Mode m)
    : env_(), unbind_(), depth_(1), mode_(m)
  {
    unbind_.push_back(libport::Finally());
    setOnSelf_.push_back(true);
  }

  Binder::~Binder()
  {}

  static inline
  boost::optional<libport::Symbol> getFirstArg(const ast::Call& call)
  {
    const ast::Ast* arg1 = &*++call.args_get().begin();
    if (!dynamic_cast<const ast::String*>(arg1))
      return boost::optional<libport::Symbol>();
    return libport::Symbol(dynamic_cast<const ast::String*>(arg1)->value_get());
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

  void Binder::visit (ast::Call& call)
  {
    super_type::visit (call);
    libport::Symbol name = call.name_get();
    // If this is a qualified call, nothing particular to do
    if (dynamic_cast<ast::Implicit*>(&call.args_get().front()))
    {
      if (name == SYMBOL(call) ||
          name == SYMBOL(locals) ||
          name == SYMBOL(self))
      {
        retarget(call, name);
        return;
      }
      if (name == SYMBOL(setSlot))
      {
        if (boost::optional<libport::Symbol> var = getFirstArg(call))
          if (setOnSelf_.back())
            targetSelf(call);
          else
            bind(var.get(), &call);
        else
          targetSelf(call);
      }
      else if (name == SYMBOL(getSlot) ||
               name == SYMBOL(locateSlot) ||
               name == SYMBOL(updateSlot) ||
               name == SYMBOL(removeSlot))
      {
        if (boost::optional<libport::Symbol> var = getFirstArg(call))
          retarget(call, var.get());
      }
      else
        retarget(call, call.name_get());
    }
  }

  void Binder::retarget(ast::Call& call, const libport::Symbol& var)
  {
    if (var == SYMBOL(call) ||
        var == SYMBOL(locals) ||
        var == SYMBOL(self))
    {
      switch (mode_)
      {
        case normal:
          return;
        case context:
          targetContext(call);
          return;
      }
      return;
    }
    int depth = isLocal(var);
    if (depth)
    {
      if (depth < depth_)
        targetContext(call, depth_ - depth);
    }
    else
      switch (mode_)
      {
        case normal:
          targetSelf(call);
          break;
        case context:
          targetContext(call);
          break;
      }
  }

  void Binder::visit (ast::Foreach& f)
  {
    bind(f.index_get(), &f);
    super_type::visit(f);
  }

  void Binder::targetSelf(ast::Call& call)
  {
    call.args_get().pop_front();
    ast::exps_type* args = new ast::exps_type;
    args->push_back(new ast::Implicit(call.location_get()));
    args->push_back(new ast::String(call.location_get(), "self"));
    ast::Call* self = new ast::Call(call.location_get(), SYMBOL(getSlot), args);
    call.args_get().push_front(self);
  }

  void Binder::targetContext(ast::Call& call, int depth)
  {
    call.args_get().pop_front();
    ast::Call* code;
    {
      ast::exps_type* args = new ast::exps_type;
      args->push_back(new ast::Implicit(call.location_get()));
      args->push_back(new ast::String(call.location_get(), "code"));
      code = new ast::Call(call.location_get(), SYMBOL(getSlot), args);
    }
    ast::Call* context;
    {
      ast::exps_type* args = new ast::exps_type;
      args->push_back(code);
      context = new ast::Call(call.location_get(), SYMBOL(context), args);
    }
    if (depth > 1)
      targetContext(*code, depth - 1);
    call.args_get().push_front(context);
  }

  void Binder::visit (ast::Scope& scope)
  {
    handleScope(scope, false);
  }

  void Binder::visit (ast::Do& scope)
  {
    scope.target_get().accept(*this);
    handleScope(scope, true);
  }

  void Binder::handleScope(ast::AbstractScope& scope, bool setOnSelf)
  {
    libport::Finally finally;
    unbind_.push_back(libport::Finally());
    finally << boost::bind(&std::list<libport::Finally>::pop_back, &unbind_);
    setOnSelf_.push_back(setOnSelf);
    finally << boost::bind(&std::list<bool>::pop_back, &setOnSelf_);
    super_type::visit (scope);
  }

  void Binder::visit (ast::Function& f)
  {
    unbind_.push_back(libport::Finally());
    depth_++;
    if (f.formals_get())
      foreach (const libport::Symbol& arg, *f.formals_get())
	bind(arg, &f);
    super_type::visit (f);
    unbind_.pop_back();
    depth_--;
  }

  void Binder::bind(const libport::Symbol& var, ast::Ast* decl)
  {
    env_[var].push_back(std::make_pair(decl, depth_));
    unbind_.back() <<
      boost::bind(&Bindings::pop_back, &env_[var]);
  }

} // namespace binder
