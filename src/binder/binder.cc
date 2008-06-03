/**
 ** \file binder/binder.cc
 ** \brief Implementation of binder::Binder.
 */

#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <libport/foreach.hh>

#include "ast/print.hh"
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
    locals_size_.push_back(std::make_pair(0, 0));
  }

  Binder::~Binder()
  {}

  static inline
  boost::optional<libport::Symbol> getFirstArg(ast::rConstCall call)
  {
    ast::rConstAst arg1 = call->arguments_get()->front();
    if (!arg1.unsafe_cast<const ast::String>())
      return boost::optional<libport::Symbol>();
    return libport::Symbol(arg1.unsafe_cast<const ast::String>()->value_get());
  }

  unsigned Binder::isLocal(const libport::Symbol& name)
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
      args->push_back(new ast::String(l, name));
      super_type::operator() (value);
      args->push_back(result_.unsafe_cast<ast::Exp>());
      return new ast::Call(l, new ast::Implicit(l), method, args);
  }

  void Binder::visit(ast::rConstAssignment input)
  {
    if (unsigned depth = isLocal(input->what_get()))
    {
      super_type::visit(input);
      result_.unsafe_cast<ast::Assignment>()->depth_set(depth_ - depth);
    }
    else
    {
      ast::rCall res = changeSlot(input->location_get(),
                                  input->what_get(),
                                  SYMBOL(updateSlot),
                                  input->value_get());
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
    libport::Symbol name = input->name_get();
    bool implicit = input->target_implicit();
    // If this is a qualified call, nothing particular to do
    if (implicit)
    {
      unsigned depth = isLocal(name);
      if (name == SYMBOL(call)
          || name == SYMBOL(locals)
          || name == SYMBOL(self))
        depth = depth_;
      if (depth)
      {
        const ast::exps_type* args = input->arguments_get();
        result_ = new ast::Local(
          input->location_get(), name,
          args ? recurse_collection(*args) : 0, depth_ - depth);
        return;
      }
      else
        super_type::visit (input);
    }
    else
      super_type::visit (input);
  }

  void Binder::visit (ast::rConstForeach input)
  {
    bind(input->index_get(), input);
    super_type::visit(input);
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

  static void decrement(unsigned* n)
  {
    (*n)--;
  }

  void Binder::visit(ast::rConstFunction input)
  {
    libport::Finally finally;

    unbind_.push_back(libport::Finally());
    finally << boost::bind(&std::list<libport::Finally>::pop_back, &unbind_);

    locals_size_.push_back(std::make_pair(0, 0));
    finally << boost::bind(&locals_size_type::pop_back, &locals_size_);
    depth_++;
    finally << boost::bind(decrement, &depth_);
    if (input->formals_get())
    {
      foreach (const libport::Symbol& arg, *input->formals_get())
	bind(arg, input);
    }
    super_type::visit (input);
    result_.unsafe_cast<ast::Function>()->
      locals_size_set(locals_size_.back().second);
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

    locals_size_.back().first++;
    unbind_.back() <<
      boost::bind(decrement, &locals_size_.back().first);

    if (locals_size_.back().first > locals_size_.back().second)
      locals_size_.back().second = locals_size_.back().first;
  }

} // namespace binder
