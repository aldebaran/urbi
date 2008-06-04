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
    : unbind_()
    , env_()
    , depth_(1)
  {
    unbind_.push_back(libport::Finally());
    setOnSelf_.push_back(true);
    push_frame_size();
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

  unsigned Binder::depth_get(const libport::Symbol& name)
  {
    if (env_[name].empty())
      return 0;
    else
    {
      assert(env_[name].back().get<1>() > 0);
      return env_[name].back().get<1>();
    }
  }

  unsigned Binder::local_index_get(const libport::Symbol& name)
  {
    if (name == SYMBOL(self))
      return 0;
    if (name == SYMBOL(call))
      return 1;
    assert(!env_[name].empty());
    return env_[name].back().get<2>();
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
    if (unsigned depth = depth_get(input->what_get()))
    {
      super_type::visit(input);
      ast::rAssignment res = result_.unsafe_cast<ast::Assignment>();
      res->depth_set(depth_ - depth);
      res->local_index_set(local_index_get(input->what_get()));
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
      int idx = locals_size_.back().first;
      bind(input);
      super_type::visit(input);
      result_.unsafe_cast<ast::Declaration>()->local_index_set(idx);
    }
  }

  void Binder::visit (ast::rConstCall input)
  {
    libport::Symbol name = input->name_get();
    bool implicit = input->target_implicit();
    // If this is a qualified call, nothing particular to do
    if (implicit)
    {
      unsigned depth = depth_get(name);
      if (name == SYMBOL(call)
          || name == SYMBOL(locals)
          || name == SYMBOL(self))
        depth = depth_;
      if (depth)
      {
        // This is a closed variable
        for (unsigned i = 0; i < depth_ - depth; i++)
          (closed_variables_stack_.end() - 1 - i)->insert(name);
        const ast::exps_type* args = input->arguments_get();
        result_ = new ast::Local(
          input->location_get(), name,
          args ? recurse_collection(*args) : 0,
          depth_ - depth,
          local_index_get(name));
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
    libport::Finally finally;

    unbind_.push_back(libport::Finally());
    finally << boost::bind(&std::list<libport::Finally>::pop_back, &unbind_);

    setOnSelf_.push_back(false);
    finally << boost::bind(&std::list<bool>::pop_back, &setOnSelf_);

    bind(input->index_get());
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

    push_frame_size();
    finally << boost::bind(&Binder::pop_frame_size, this);

    push_closed_variables();
    finally << boost::bind(&Binder::pop_closed_variables, this);

    setOnSelf_.push_back(false);
    finally << boost::bind(&std::list<bool>::pop_back, &setOnSelf_);

    depth_++;
    finally << boost::bind(decrement, &depth_);

    if (input->formals_get())
      foreach (ast::rConstDeclaration arg, *input->formals_get())
	bind(arg);

    super_type::visit (input);
    ast::rFunction res = result_.unsafe_cast<ast::Function>();
    res->locals_size_set(frame_size());
    foreach (const libport::Symbol& var, closed_variables())
      res->closed_variables_get().push_back(var);
  }

  void Binder::visit(ast::rConstClosure input)
  {
    if (input->formals_get())
      foreach (ast::rConstDeclaration arg, *input->formals_get())
	bind(arg);
    super_type::visit(input);
  }

  void Binder::bind(ast::rConstDeclaration decl)
  {
    assert(decl);
    env_[decl->what_get()].push_back(boost::make_tuple(decl, depth_,
                                          locals_size_.back().first));
    unbind_.back() <<
      boost::bind(&Bindings::pop_back, &env_[decl->what_get()]);

    inc_frame_size();
  }

  void Binder::push_frame_size()
  {
    // The initial size is two, for self and call.
    // This might change when self becomes a keyword
    locals_size_.push_back(std::make_pair(2, 2));
  }

  void Binder::pop_frame_size()
  {
    locals_size_.pop_back();
  }

  unsigned Binder::frame_size() const
  {
    return locals_size_.back().second;
  }

  void Binder::inc_frame_size()
  {
    locals_size_.back().first++;
    unbind_.back() <<
      boost::bind(decrement, &locals_size_.back().first);

    if (locals_size_.back().first > locals_size_.back().second)
      locals_size_.back().second = locals_size_.back().first;
  }

  void Binder::push_closed_variables()
  {
    closed_variables_stack_.push_back(closed_variables_type());
  }

  void Binder::pop_closed_variables()
  {
    closed_variables_stack_.pop_back();
  }

  const Binder::closed_variables_type& Binder::closed_variables() const
  {
    return closed_variables_stack_.back();
  }

} // namespace binder
