/**
 ** \file binder/binder.cc
 ** \brief Implementation of binder::Binder.
 */

// #define ENABLE_BIND_DEBUG_TRACES

#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <libport/foreach.hh>

#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <ast/print.hh>
#include <binder/binder.hh>
#include <binder/bind-debug.hh>
#include <object/symbols.hh>
#include <object/object.hh>

namespace binder
{

  /*----------.
  | Helpers.  |
  `----------*/

  namespace
  {
    static
    ast::rExp
    make_declaration(const ast::loc& l, libport::Symbol s)
    {
      static ast::ParametricAst a("nil");
      return new ast::Declaration(l, s, exp(a));
    }

    static
    ast::rExp
    make_closure(ast::rExp value)
    {
      static ast::ParametricAst a("closure () { %exp:1 }");
      return exp(a % value);
    }

    static
    ast::rExp
    make_assignment(const ast::loc& l, libport::Symbol s, ast::rExp value)
    {
      return make_closure(new ast::Assignment(l, s, value, 0));
    }
  }


  /*---------.
  | Binder.  |
  `---------*/

  Binder::Binder()
    : unbind_()
    , env_()
    , depth_(1)
    , toplevel_index_(0)
  {
    unbind_.push_back(libport::Finally());
    setOnSelf_.push_back(true);
  }

  Binder::~Binder()
  {}

  unsigned
  Binder::depth_get(const libport::Symbol& name)
  {
    if (env_[name].empty())
      return 0;
    else
    {
      assert(env_[name].back().second > 0);
      return env_[name].back().second;
    }
  }

  ast::rDeclaration
  Binder::decl_get(const libport::Symbol& name)
  {
    assert (!env_[name].empty());
    return env_[name].back().first;
  }

  ast::rCall
  Binder::changeSlot(const ast::loc& l,
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


  /*----------------.
  | Routine stack.  |
  `----------------*/

  void
  Binder::routine_push(ast::rRoutine f)
  {
    routine_stack_.push_back(f);
  }

  void
  Binder::routine_pop()
  {
    routine_stack_.pop_back();
  }

  ast::rRoutine
  Binder::routine() const
  {
    assert(!routine_stack_.empty());
    return routine_stack_.back();
  }

  ast::rFunction
  Binder::function() const
  {
    rforeach(ast::rRoutine r, routine_stack_)
      if (ast::rFunction res = r.unsafe_cast<ast::Function>())
        return res;
    return 0;
  }

  void
  Binder::visit(ast::rConstDeclaration input)
  {
    if (setOnSelf_.back())
      result_ = changeSlot(input->location_get(), input->what_get(),
                           SYMBOL(setSlot), input->value_get());
    else
    {
      BIND_ECHO("Bind " << input->what_get());
      super_type::visit(input);
      ast::rDeclaration res = result_.unsafe_cast<ast::Declaration>();
      bind(res);
    }
  }

  template <typename Node, typename NewNode>
  void
  Binder::link_to_declaration(Node input,
                              NewNode result,
                              const libport::Symbol& name,
                              unsigned depth)
  {
    BIND_ECHO("Linking " << name << " to its declaration");
    ast::rDeclaration outer_decl = decl_get(name);
    ast::rDeclaration decl = outer_decl;
    ast::rLocal current;

    if (depth_ > depth)
    {
      // The variable is captured
      BIND_NECHO(libport::incindent);
      BIND_ECHO("It's captured");
      decl->closed_set(true);

      routine_stack_type::reverse_iterator f_it = routine_stack_.rbegin();
      const ast::loc loc = input->location_get();
      for (int i = depth_ - depth; i; --i, ++f_it)
      {
        ast::rRoutine f = *f_it;
        // Check whether it's already captured
        foreach (ast::rDeclaration dec, *f->captured_variables_get())
          if (dec->what_get() == name)
          {
            outer_decl = dec;
            // Break foreach and for
            goto stop;
          }

        decl = new ast::Declaration(loc, name, 0);
        decl->closed_set(true);

        if (current)
          current->declaration_set(decl);
        else
          result->declaration_set(decl);

        current = new ast::Local(loc, name, 0, i - 1);
        decl->value_set(current);

        f->captured_variables_get()->push_back(decl);
      }
    stop:
      BIND_NECHO(libport::decindent);
    }

    if (current)
      current->declaration_set(outer_decl);
    else
      result->declaration_set(outer_decl);
  }


  void
  Binder::visit(ast::rConstAssignment input)
  {
    assert(!input->declaration_get());
    libport::Symbol name = input->what_get();
    if (unsigned depth = depth_get(name))
    {
      super_type::visit(input);
      ast::rAssignment res = result_.unsafe_cast<ast::Assignment>();
      res->depth_set(depth_ - depth);
      link_to_declaration(input, res, input->what_get(), depth);
    }
    else
      result_ = changeSlot(input->location_get(),
                           input->what_get(),
                           SYMBOL(updateSlot),
                           input->value_get());
  }


  void
  Binder::visit(ast::rConstCall input)
  {
    libport::Symbol name = input->name_get();
    ast::loc loc = input->location_get();

    {
      bool implicit;
      unsigned depth;
      // If this is a qualified call, nothing particular to do
      if ((implicit = input->target_implicit())
          && (depth = depth_get(name)))
      {
        ast::rLocal res =
          new ast::Local(loc, name,
                         recurse_collection(input->arguments_get()),
                         depth_ - depth);
        link_to_declaration(input, res, name, depth);
        result_ = res;
      }
      else
        super_type::visit(input);
    }

    ast::exps_type* args = 0;
    if (ast::rCall res = result_.unsafe_cast<ast::Call>())
      args = res->arguments_get();
    else if (ast::rLocal res = result_.unsafe_cast<ast::Local>())
      args = res->arguments_get();

    if (args)
    {
      assert(args->size() == input->arguments_get()->size());
      for (unsigned i = 0; i < args->size(); ++i)
      {
        ast::rExp& arg = (*args)[i];
        ast::loc loc = arg->location_get();
        arg = new ast::Lazy(loc,
                            lazify((*input->arguments_get())[i], loc),
                            arg);
      }
    }
  }


  void
  Binder::visit(ast::rConstCallMsg input)
  {
    ast::rFunction fun = function();
    if (!fun)
      errors_.error(input->location_get(), "call: used outside any function");
    else if (fun->strict())
      errors_.error(input->location_get(), "call: used in a strict function");
    else
      return super_type::visit(input);
  }

  ast::rExp
  Binder::lazify (ast::rExp arg, const ast::loc& loc)
  {
    // build Lazy.clone.init(closure () { %arg })
    ast::rCall lazy = new ast::Call
      (loc, new ast::Implicit(loc), SYMBOL(Lazy), 0);
    ast::rCall clone = new ast::Call(loc, lazy, SYMBOL(clone), 0);
    ast::exps_type* init_args = new ast::exps_type();

    {
      // FIXME: Maybe started another Binder would be better?
      ast::rAst res;
      std::swap(res, result_);
      init_args->push_back(recurse(make_closure(arg)));
      std::swap(res, result_);
    }

    ast::rCall init = new ast::Call(loc, clone, SYMBOL(init), init_args);
    return init;
  }

  void
  Binder::visit (ast::rConstForeach input)
  {
    libport::Finally finally;

    unbind_.push_back(libport::Finally());
    finally << boost::bind(&unbind_type::pop_back, &unbind_);

    setOnSelf_.push_back(false);
    finally << boost::bind(&set_on_self_type::pop_back, &setOnSelf_);

    bind(input->index_get());
    super_type::visit(input);
  }

  void
  Binder::visit (ast::rConstScope input)
  {
    result_ = new ast::Scope(input->location_get(), handleScope(input, false));
  }

  void
  Binder::visit (ast::rConstDo input)
  {
    operator() (input->target_get());
    ast::rExp target = result_.unsafe_cast<ast::Exp>();
    result_ = new ast::Do(input->location_get(),
                          handleScope(input, true),
                          target);
  }

  ast::rExp
  Binder::handleScope(ast::rConstAbstractScope scope, bool setOnSelf)
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

  static
  void
  decrement(unsigned* n)
  {
    (*n)--;
  }

  template <typename Code>
  void
  Binder::handleRoutine(libport::shared_ptr<const Code> input)
  {
    BIND_ECHO("Push" << libport::incindent);
    libport::Finally finally;

    // Clone and push the function, without filling its body and arguments
    libport::shared_ptr<Code> res = new Code(input->location_get(), 0, 0);
    res->local_variables_set(new ast::declarations_type());
    routine_push(res);
    finally << boost::bind(&Binder::routine_pop, this);

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
    res->formals_set(recurse_collection(input->formals_get()));

    // Bind and clone the body
    res->body_set(recurse(input->body_get ()));
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

    BIND_NECHO(libport::decindent);
    BIND_ECHO("Pop");

  }

  void
  Binder::visit(ast::rConstFunction input)
  {
    handleRoutine(input);
  }

  void
  Binder::visit(ast::rConstClosure input)
  {
    handleRoutine(input);
  }

  void
  Binder::bind(ast::rDeclaration decl)
  {
    assert(decl);

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

    env_[decl->what_get()].push_back(std::make_pair(decl, depth_));
    unbind_.back() <<
      boost::bind(&Bindings::pop_back, &env_[decl->what_get()]);
  }

  void
  Binder::visit(ast::rConstAnd input)
  {
    static ast::ParametricAst closure("closure () { %exp:1 }");

    ast::rAnd res = new ast::And(input->location_get(), ast::exps_type());
    foreach (ast::rExp child, input->children_get())
      // Wrap every children in a closure
      res->children_get().push_back(recurse(make_closure(child)));
    result_ = res;
  }

  void
  Binder::visit(ast::rConstNary input)
  {
    ast::rNary res = new ast::Nary(input->location_get());
    foreach (ast::rExp child, (input->children_get()))
    {
      ast::rStmt stm = child.unsafe_cast<ast::Stmt>();
      if (stm && stm->flavor_get() == ast::flavor_comma)
      {
        if (ast::rDeclaration dec =
            stm->expression_get().unsafe_cast<ast::Declaration>())
        {
          const ast::loc l = dec->location_get();
          const libport::Symbol s = dec->what_get();
          res->push_back(recurse(make_declaration(l, s)),
                         ast::flavor_semicolon);
          res->push_back(recurse(make_assignment(l, s, dec->value_get())),
                         ast::flavor_comma);
        }
        else
          res->push_back(recurse(make_closure(child)), ast::flavor_comma);
      }
      else
        res->push_back(recurse(child));
    }
    result_ = res;
  }

} // namespace binder
