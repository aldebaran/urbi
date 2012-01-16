/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file binder/binder.cc
 ** \brief Implementation of binder::Binder.
 */

#include <boost/bind.hpp>
#include <boost/optional.hpp>

#include <libport/containers.hh>
#include <libport/foreach.hh>

#include <ast/cloner.hxx>       // Needed for recurse_collection templates
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <ast/print.hh>

#include <binder/binder.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/object.hh>

#include <ast/factory.hh>
#include <parser/parse.hh>

GD_CATEGORY(Urbi.Bind);
DECLARE_LOCATION_FILE;

namespace binder
{
  using libport::Finally;

  /*---------.
  | Binder.  |
  `---------*/

  Binder::Binder()
    : unbind_()
    , env_()
    , routine_depth_(1)
    , scope_depth_(1)
    , toplevel_index_(0)
    , report_errors_(true)
    , unscope_(0)
  {
    unbind_ << Finally();
    setOnSelf_ << true;
  }

  Binder::~Binder()
  {}

  unsigned
  Binder::routine_depth_get(libport::Symbol name)
  {
    if (env_[name].empty())
      return 0;
    else
    {
      aver(env_[name].back().second.first > 0);
      return env_[name].back().second.first;
    }
  }

  unsigned
  Binder::scope_depth_get(libport::Symbol name)
  {
    if (env_[name].empty())
      return 0;
    else
    {
      aver(env_[name].back().second.second > 0);
      return env_[name].back().second.second;
    }
  }

  ast::rLocalDeclaration
  Binder::decl_get(libport::Symbol name)
  {
    aver(!env_[name].empty());
    return env_[name].back().first;
  }

  ast::rCall
  Binder::changeSlot(const ast::loc& l,
                     const ast::rExp& target,
                     libport::Symbol name,
                     libport::Symbol method,
                     ast::rConstExp value)
  {
    PARAMETRIC_AST(call, "%exp:1 . %id:2 (%exps:3)");

    ast::exps_type* args = new ast::exps_type;
    *args << new ast::String(l, name);
    if (value)
      *args << const_cast<ast::Exp*>(value.get());
    call % target
      % method
      % args;
    return recurse(ast::rCall(call.result<ast::Call>()));
  }

  /*----------------.
  | Routine stack.  |
  `----------------*/

  void
  Binder::routine_push(ast::rRoutine f)
  {
    routine_stack_ << f;
  }

  void
  Binder::routine_pop()
  {
    routine_stack_.pop_back();
  }

  ast::rRoutine
  Binder::routine() const
  {
    aver(!routine_stack_.empty());
    return routine_stack_.back();
  }

  ast::rRoutine
  Binder::function() const
  {
    rforeach(ast::rRoutine r, routine_stack_)
      if (!r->closure_get())
        return r;
    return 0;
  }

  bool
  Binder::set_on_self(unsigned up)
  {
    aver(setOnSelf_.size() > up);
    return *(setOnSelf_.end() - up - 1);
  }

  void
  Binder::err(const ast::loc& loc, const std::string& msg)
  {
    errors_.err(loc, msg, "syntax error");
  }

  void
  Binder::visit(const ast::Declaration* input)
  {
    ast::loc loc = input->location_get();
    ast::rCall call = input->what_get()->call();
    ast::rExp value = input->value_get();

    libport::Symbol name = call->name_get();

    if (!call->target_implicit() || set_on_self(unscope_))
      if (value)
        result_ = changeSlot(loc, call->target_get(), name,
                             input->constant_get()
                             ? SYMBOL(setConstSlot)
                             : SYMBOL(setSlot), value);
      else
        result_ = changeSlot(loc, call->target_get(), name,
                             SYMBOL(createSlot));
    else
    {
      // Check this is not a redefinition
      if (scope_depth_ - unscope_ == scope_depth_get(name))
        if (report_errors_)
          err(loc, "variable redefinition: " + name.name_get());

      GD_SINFO_DUMP("Bind " << name);

      if (value)
      {
        operator()(value.get());
        value = result_.unchecked_cast<ast::Exp>();
      }
      ast::rLocalDeclaration res =
        new ast::LocalDeclaration(loc, name, value);
      res->constant_set(input->constant_get());
      bind(res);
      result_ = res;
    }
    result_->original_set(input);
  }

  // LocalDeclaration are present before binding, as function formal
  // arguments. We must bind them so as routines are aware of their
  // presence.
  void
  Binder::visit(const ast::LocalDeclaration* input)
  {
    super_type::visit(input);
    ast::rLocalDeclaration dec = result_.unsafe_cast<ast::LocalDeclaration>();
    bind(dec);
  }

  void
  Binder::visit(const ast::Match* match)
  {
    const ast::loc& location = match->location_get();
    ast::rExp pattern = recurse(match->pattern_get());
    ast::rExp bindings = recurse(match->bindings_get());
    ast::rExp guard = recurse(match->guard_get());
    ast::Match* res = new ast::Match (location, pattern, guard);
    res->bindings_set(bindings);
    result_ = res;
  }

  void
  Binder::visit(const ast::Assignment* input)
  {
    ast::loc loc = input->location_get();
    ast::rCall call = input->what_get()->call();
    libport::Symbol name = call->name_get();
    unsigned depth = routine_depth_get(name);
    // Whether this is an assignment to a local variable
    bool local = depth && call->target_implicit();

    // Assignment to a local variables
    if (local)
    {
      operator()(input->value_get().get());
      ast::rExp value = result_.unchecked_cast<ast::Exp>();
      ast::rLocalAssignment res =
        new ast::LocalAssignment(loc, name, value, routine_depth_ - depth);
      link_to_declaration(input, res, name, depth);
      result_ = res;
    }
    // Assignment to a slot
    else
      result_ = changeSlot(loc, call->target_get(), name, SYMBOL(updateSlot),
                           input->value_get());
    result_->original_set(input);
  }

  template <typename Node, typename NewNode>
  void
  Binder::link_to_declaration(Node input,
                              NewNode result,
                              libport::Symbol name,
                              unsigned depth)
  {
    GD_SINFO_DUMP("Linking " << name << " to its declaration");
    ast::rLocalDeclaration outer_decl = decl_get(name);
    ast::rLocalDeclaration decl = outer_decl;
    ast::rLocal current;

    if (routine_depth_ > depth)
    {
      // The variable is captured
      GD_PUSH_DUMP("It's captured");

      routine_stack_type::reverse_iterator f_it = routine_stack_.rbegin();
      const ast::loc loc = input->location_get();
      for (unsigned int i = routine_depth_ - depth; i; --i, ++f_it)
      {
        ast::rRoutine f = *f_it;
        // Check whether it's already captured
        foreach (ast::rLocalDeclaration dec, *f->captured_variables_get())
          if (dec->what_get() == name)
          {
            decl = outer_decl = dec;
            // Break foreach and for.
            goto stop;
          }

        decl = new ast::LocalDeclaration(loc, name, 0);

        if (current)
          current->declaration_set(decl);
        else
          result->declaration_set(decl);

        current = new ast::Local(loc, name, 0, i - 1);
        decl->value_set(current);

        f->captured_variables_get()->push_back(decl);
      }
    stop:;
    }

    if (current)
      current->declaration_set(outer_decl);
    else
      result->declaration_set(outer_decl);
  }

  void
  Binder::visit(const ast::Call* input)
  {
    ast::loc loc = input->location_get();
    libport::Symbol name = input->name_get();
    unsigned depth = routine_depth_get(name);
    bool local = depth && input->target_implicit();

    // If this is a qualified call, nothing particular to do
    if (local)
    {
      ast::rLocal res =
        new ast::Local(loc, name,
                       maybe_recurse_collection(input->arguments_get()),
                       routine_depth_ - depth);
      link_to_declaration(input, res, name, depth);
      result_ = res;
    }
    else
      super_type::visit(input);
  }


  void
  Binder::visit(const ast::CallMsg* input)
  {
    if (ast::rRoutine fun = function())
    {
      fun->uses_call_set(true);
      super_type::visit(input);
    }
    else if (report_errors_)
      err(input->location_get(), "call: used outside any function");
  }

  void
  Binder::visit(const ast::Catch* input)
  {
    // Create a scope to execute the "catch" action.
    Finally finally(scope_open(false));
    super_type::visit(input);
  }

  ast::rExp
  Binder::lazify (ast::rExp arg)
  {
    ast::rAst save;
    std::swap(save, result_);
    ast::Factory factory;
    ast::rExp res = recurse(factory.make_closure(arg));
    std::swap(save, result_);
    return res;
  }

  void
  Binder::visit (const ast::Foreach* input)
  {
    Finally finally;

    unbind_ << Finally();
    finally << boost::bind(&unbind_type::pop_back, &unbind_);

    setOnSelf_ << false;
    finally << boost::bind(&set_on_self_type::pop_back, &setOnSelf_);

    bind(input->index_get());
    super_type::visit(input);
  }

  void
  Binder::visit (const ast::Scope* input)
  {
    Finally f(scope_open(false));
    super_type::visit(input);
  }

  void
  Binder::visit (const ast::Do* input)
  {
    Finally f(scope_open(true));
    super_type::visit(input);
  }

  static
  void
  decrement(unsigned* n)
  {
    (*n)--;
  }

  Finally::action_type
  Binder::scope_open(bool set_on_self)
  {
    scope_depth_++;
    // Push a finally on unbind_, and destroy it at the scope
    // exit. Since bound variables register themselves for unbinding
    // in unbind_'s top element, they will be unbound at scope exit.
    unbind_ << Finally();
    setOnSelf_ << set_on_self;
    return boost::bind(&Binder::scope_close, this);
  }

  void
  Binder::scope_close()
  {
    scope_depth_--;
    unbind_.pop_back();
    setOnSelf_.pop_back();
  }

  void
  Binder::visit(const ast::Routine* input)
  {
    // Check whether default arguments are followed by non-default arguments
    if (input->formals_get())
    {
      bool found_def = false;
      bool found_list = false;
      foreach (ast::rLocalDeclaration decl, *input->formals_get())
      {
        if (found_list)
          err(decl->location_get(), "argument after list-argument");

        if (decl->value_get())
          found_def = true;
        else if (found_def && !decl->list_get())
          err(decl->location_get(),
                        "argument with no default value after arguments"
                        " with default value");
        if (decl->list_get())
          found_list = true;
      }
    }

    GD_PUSH_DUMP("Push");
    Finally finally(5);

    // Clone and push the function, without filling its body and arguments
    ast::rRoutine res =
      new ast::Routine(input->location_get(), input->closure_get(), 0, 0);
    res->local_variables_set(new ast::local_declarations_type());
    routine_push(res);
    finally << boost::bind(&Binder::routine_pop, this);

    // Open a new scope
    unbind_ << Finally();
    finally << boost::bind(&unbind_type::pop_back, &unbind_);

    // Do not setOnSelf in this scope
    setOnSelf_ << false;
    finally << boost::bind(&set_on_self_type::pop_back, &setOnSelf_);

    // Increase the nested functions and scopes depth
    routine_depth_++;
    finally << boost::bind(decrement, &routine_depth_);
    scope_depth_++;
    finally << boost::bind(decrement, &scope_depth_);

    // Clone and bind arguments
    res->formals_set(maybe_recurse_collection(input->formals_get()));

    // Bind and clone the body
    res->body_set(recurse(input->body_get ()));
    result_ = res;

    // Index local and closed variables
    unsigned int local = 0;
    foreach (ast::rLocalDeclaration dec, *res->local_variables_get())
      dec->local_index_set(local++);
    res->local_size_set(local);

    // Index captured variables
    unsigned int captured = 0;
    foreach (ast::rLocalDeclaration dec, *res->captured_variables_get())
      dec->local_index_set(captured++);

    GD_INFO_DUMP("Pop");
  }


  // Scope variables declared in conditions
#define SCOPE(Node)                             \
  void                                          \
  Binder::visit(const ast::Node* input)         \
  {                                             \
    Finally f(scope_open(false));               \
    super_type::visit(input);                   \
  }                                             \

  SCOPE(If);
  SCOPE(While);

#undef SCOPE

  void
  Binder::bind(ast::rLocalDeclaration decl)
  {
    aver(decl);

    // If we are at the toplevel, we can't determine the local index.
    if (routine_stack_.empty())
    {
      // This indexing merges locals and closed, thus entailing too
      // much toplevel stack allocation in Interpreter::local_set.
      decl->local_index_set(toplevel_index_++);
      unbind_.back() << boost::bind(decrement, &toplevel_index_);
    }
    else
      routine()->local_variables_get()->push_back(decl);

    env_[decl->what_get()] << std::make_pair
      (decl, std::make_pair(routine_depth_, scope_depth_ - unscope_));
    unbind_type::iterator it = unbind_.end();
    for (unsigned i = 0; i <= unscope_; ++i)
      it--;
    *it << boost::bind(&Bindings::pop_back, &env_[decl->what_get()]);
  }

  void
  Binder::visit(const ast::Unscope* unscope)
  {
    unbind_.back() << libport::scoped_set(unscope_, unscope->count_get());
    result_ = new ast::Noop(unscope->location_get(), 0);
  }

  void
  Binder::visit(const ast::Dictionary* dict)
  {
    super_type::visit(dict);
  }
} // namespace binder
