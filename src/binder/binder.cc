/**
 ** \file binder/binder.cc
 ** \brief Implementation of binder::Binder.
 */

#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <libport/foreach.hh>

#include <ast/print.hh>
#include <binder/binder.hh>
#include <object/symbols.hh>
#include <object/object.hh>

namespace binder
{
  Binder::Binder()
    : unbind_()
    , env_()
    , depth_(1)
  {
    unbind_.push_back(libport::Finally());
    setOnSelf_.push_back(true);
  }

  Binder::~Binder()
  {}

  unsigned Binder::depth_get(const libport::Symbol& name)
  {
    if (env_[name].empty())
      return 0;
    else
    {
      assert(env_[name].back().second > 0);
      return env_[name].back().second;
    }
  }

  ast::rDeclaration Binder::decl_get(const libport::Symbol& name)
  {
    assert (!env_[name].empty());
    return env_[name].back().first;
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
    assert(!input->declaration_get());
    libport::Symbol name = input->what_get();
    if (unsigned depth = depth_get(name))
    {
      super_type::visit(input);
      ast::rAssignment res = result_.unsafe_cast<ast::Assignment>();
      res->depth_set(depth_ - depth);
      res->declaration_set(decl_get(name));
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
      super_type::visit(input);
      ast::rDeclaration res = result_.unsafe_cast<ast::Declaration>();
      bind(res);
      // FIXME: How should we handle the toplevel?
      if (!function_stack_.empty())
        function()->local_variables_get()->
          push_back(res);
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
      if (depth)
      {
        function_stack_type::reverse_iterator it = function_stack_.rbegin();

        ast::rDeclaration outer_decl = decl_get(name);
        ast::rDeclaration decl = outer_decl;

        const ast::exps_type* args = input->arguments_get();
        ast::rLocal res = new ast::Local(
          input->location_get(), name,
          args ? recurse_collection(*args) : 0,
          depth_ - depth);
        ast::rLocal current = res;

        if (depth_ > depth)
        {
          // The variable is captured
          decl->closed_set(true);

          function_stack_type::iterator f_it = function_stack_.end();
          --f_it;
          const ast::loc loc = input->location_get();
          for (int i = depth_ - depth; i; --i, ++f_it)
          {
            assert(f_it != function_stack_.begin() || i == 1);
            ast::rFunction f = *f_it;
            // Check whether it's already captured
            foreach (ast::rDeclaration dec, *f->captured_variables_get())
              if (dec->what_get() == name)
              {
                outer_decl = dec;
                goto stop;
              }

            decl = new ast::Declaration(loc, name, 0);
            decl->closed_set(true);

            current->declaration_set(decl_get(name));

            current = new ast::Local(loc, name, 0, i - 1);
            decl->value_set(current);

            f->captured_variables_get()->push_back(decl);
          }
          stop: ;
        }

        current->declaration_set(outer_decl);
        result_ = res;
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
    finally << boost::bind(&unbind_type::pop_back, &unbind_);

    setOnSelf_.push_back(false);
    finally << boost::bind(&set_on_self_type::pop_back, &setOnSelf_);

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
    finally << boost::bind(&unbind_type::pop_back, &unbind_);

    setOnSelf_.push_back(setOnSelf);
    finally << boost::bind(&set_on_self_type::pop_back, &setOnSelf_);

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

    // Clone and push the function, without filling its body and arguments
    ast::rFunction res = new ast::Function(input->location_get(), 0, 0);
    res->local_variables_set(new ast::declarations_type());
    push_function(res);
    finally << boost::bind(&Binder::pop_function, this);

    // Open a new scope
    unbind_.push_back(libport::Finally());
    finally << boost::bind(&unbind_type::pop_back, &unbind_);

    // Do not setOnSelf in this scope
    setOnSelf_.push_back(false);
    finally << boost::bind(&set_on_self_type::pop_back, &setOnSelf_);

    // Increase the nested functions depth
    depth_++;
    finally << boost::bind(decrement, &depth_);

    // Bind and clone arguments
    ast::declarations_type* formals =
      input->formals_get () ? recurse_collection (*input->formals_get ()) : 0;

    // Bind and clone the body
    ast::rAbstractScope body = recurse (input->body_get ());

    // Assemble the result
    res->formals_set(formals);
    res->body_set(body);
    result_ = res;

    // Index local and closed variables
    int local = 0;
    int closed = 0;
    foreach (ast::rDeclaration dec, *res->local_variables_get())
      if (dec->closed_get())
        dec->local_index_set(closed++);
      else
        dec->local_index_set(local++);

    // Index captured variables
    int captured = 0;
    foreach (ast::rDeclaration dec, *res->captured_variables_get())
      dec->local_index_set(captured++);

  }

  void Binder::visit(ast::rConstClosure input)
  {
    assert(!input->local_variables_get());
    ast::declarations_type* formals =
      input->formals_get () ? recurse_collection (*input->formals_get ()) : 0;
    if (formals)
      foreach (ast::rDeclaration arg, *formals)
	bind(arg);

    ast::rAbstractScope body = recurse (input->body_get ());
    ast::rClosure res = new ast::Closure (input->location_get(), formals, body);
    result_ = res;
  }

  void Binder::bind(ast::rDeclaration decl)
  {
    assert(decl);
    env_[decl->what_get()].push_back(std::make_pair(decl, depth_));
    unbind_.back() <<
      boost::bind(&Bindings::pop_back, &env_[decl->what_get()]);
  }

  void Binder::push_function(ast::rFunction f)
  {
    function_stack_.push_back(f);
  }

  void Binder::pop_function()
  {
    function_stack_.pop_back();
  }

  ast::rFunction Binder::function() const
  {
    assert(!function_stack_.empty());
    return function_stack_.back();
  }

} // namespace binder
