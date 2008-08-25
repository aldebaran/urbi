/**
 ** \file binder/binder.cc
 ** \brief Implementation of binder::Binder.
 */

// #define ENABLE_BIND_DEBUG_TRACES

#include <boost/bind.hpp>
#include <boost/optional.hpp>

#include <libport/containers.hh>
#include <libport/foreach.hh>

#include <ast/cloner.hxx>       // Needed for recurse_collection templates
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <ast/print.hh>

#include <binder/binder.hh>
#include <binder/bind-debug.hh>

#include <object/symbols.hh>
#include <object/object.hh>

#include <parser/ast-factory.hh>
#include <parser/parse.hh>

namespace binder
{
  using ast::ParametricAst;
  using parser::ast_lvalue_once;
  using parser::ast_lvalue_wrap;

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
  {
    unbind_.push_back(libport::Finally());
    setOnSelf_.push_back(true);
  }

  Binder::~Binder()
  {}

  unsigned
  Binder::routine_depth_get(const libport::Symbol& name)
  {
    if (env_[name].empty())
      return 0;
    else
    {
      assert(env_[name].back().second.first > 0);
      return env_[name].back().second.first;
    }
  }

  unsigned
  Binder::scope_depth_get(const libport::Symbol& name)
  {
    if (env_[name].empty())
      return 0;
    else
    {
      assert(env_[name].back().second.second > 0);
      return env_[name].back().second.second;
    }
  }

  ast::rLocalDeclaration
  Binder::decl_get(const libport::Symbol& name)
  {
    assert (!env_[name].empty());
    return env_[name].back().first;
  }

  ast::rCall
  Binder::changeSlot(const ast::loc& l,
                     const ast::rExp& target,
                     const libport::Symbol& name,
                     const libport::Symbol& method,
                     ast::rConstExp value)
  {
    static ast::ParametricAst call("%exp:1 . %id:2 (%exps:3)");

    ast::exps_type* args = new ast::exps_type;
    *args << new ast::String(l, name);
    if (value)
      *args << const_cast<ast::Exp*>(value.get());
    call % target
      % method
      % args;
    return recurse(ast::rCall(call.result<ast::Call>()));
  }

  ast::rExp
  Binder::changeSlot(const ast::loc& l,
                     const ast::rExp& target,
                     const libport::Symbol& name,
                     const std::string& doc,
                     ast::rConstExp value)
  {
    static ast::ParametricAst document
      ("{"
       "  %exp:1 . setVoidSlot(%exp:2) | "
       "  %exp:3 . setProperty(%exp:4, %exp:5, %exp:6) |"
       "  %exp:7"
       "}");

    ast::rExp res = changeSlot(l, target, name, SYMBOL(updateSlot), value);
    document
      % target
      % new ast::String(l, name)
      % target
      % new ast::String(l, name)
      % new ast::String(l, SYMBOL(doc))
      % new ast::String(l, libport::Symbol(doc))
      % res;
    return (exp(document));
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
  Binder::visit(const ast::Declaration* input)
  {
    ast::loc loc = input->location_get();
    ast::rCall call = input->what_get()->call();
    ast::rExp value = input->value_get();

    libport::Symbol name = call->name_get();

    if (!call->target_implicit() || setOnSelf_.back())
      if (value)
      {
        std::string doc = input->doc_get();
        if (doc != "")
          result_ = changeSlot(loc, call->target_get(), name,
                               doc, value);
        else
          result_ = changeSlot(loc, call->target_get(), name,
                               SYMBOL(setSlot), value);
      }
      else
        result_ = changeSlot(loc, call->target_get(), name,
                             SYMBOL(setVoidSlot));
    else
    {
      // Check this is not a redefinition
      if (scope_depth_ == scope_depth_get(name))
        if (report_errors_)
          errors_.error(loc, "variable redefinition: " + name.name_get());

      BIND_ECHO("Bind " << name);

      if (value)
      {
        operator()(value.get());
        value = result_.unchecked_cast<ast::Exp>();
      }
      ast::rLocalDeclaration res =
        new ast::LocalDeclaration(loc, name, value);
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

    BIND_ECHO("Bind " << name);
    bind(dec);
  }

  void
  Binder::visit(const ast::Assignment* input)
  {
    ast::loc loc = input->location_get();
    ast::rCall call = input->what_get()->call();
    ast::rExp target_value = recurse(input->value_get());
    libport::Symbol name = call->name_get();

    ast::rExp modifiers = 0;
    if (const ast::modifiers_type* source = input->modifiers_get())
    {
      static ast::ParametricAst dict("Dictionary.new");

      modifiers = exp(dict);
      foreach (const ast::modifiers_type::value_type& elt, *source)
      {
        static ast::ParametricAst add("%exp:1.set(%exp:2, %exp:3)");

        add % modifiers
          % new ast::String(input->location_get(), elt.first)
          % recurse(elt.second);
        modifiers = exp(add);
      }
    }

    unsigned depth = routine_depth_get(name);

    if (modifiers)
    {
      ast::rLValue tgt = ast_lvalue_once(call);
      static ParametricAst trajectory(
        "TrajectoryGenerator.new("
        "  closure ( ) { %exp:1 }," // getter
        "  closure (v) { %lvalue:2 = v }," // Setter
        "  %exp:3," // Target value
        "  %exp:4" // modifiers
        ").run"
        );

      trajectory % tgt
        % tgt
        % target_value
        % modifiers;

      operator()(ast_lvalue_wrap(call, exp(trajectory).get()).get());
    }
    else if (depth && call->target_implicit())
    {
      // Assignment to a local variables

      operator()(input->value_get().get());
      ast::rExp value = result_.unchecked_cast<ast::Exp>();
      ast::rLocalAssignment res =
        new ast::LocalAssignment(loc, name, value, routine_depth_ - depth);
      link_to_declaration(input, res, name, depth);
      result_ = res;
    }
    else
        result_ = changeSlot(loc, call->target_get(), name,
                             SYMBOL(updateSlot), input->value_get());
    result_->original_set(input);
  }

  template <typename Node, typename NewNode>
  void
  Binder::link_to_declaration(Node input,
                              NewNode result,
                              const libport::Symbol& name,
                              unsigned depth)
  {
    BIND_ECHO("Linking " << name << " to its declaration");
    ast::rLocalDeclaration outer_decl = decl_get(name);
    ast::rLocalDeclaration decl = outer_decl;
    ast::rLocal current;

    if (routine_depth_ > depth)
    {
      // The variable is captured
      BIND_NECHO(libport::incindent);
      BIND_ECHO("It's captured");
      decl->closed_set(true);

      routine_stack_type::reverse_iterator f_it = routine_stack_.rbegin();
      const ast::loc loc = input->location_get();
      for (int i = routine_depth_ - depth; i; --i, ++f_it)
      {
        ast::rRoutine f = *f_it;
        // Check whether it's already captured
        foreach (ast::rLocalDeclaration dec, *f->captured_variables_get())
          if (dec->what_get() == name)
          {
            outer_decl = dec;
            // Break foreach and for
            goto stop;
          }

        decl = new ast::LocalDeclaration(loc, name, 0);
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
  Binder::visit(const ast::Call* input)
  {
    libport::Symbol name = input->name_get();
    ast::loc loc = input->location_get();

    {
      bool implicit;
      unsigned depth;
      // If this is a qualified call, nothing particular to do
      if ((implicit = input->target_implicit())
          && (depth = routine_depth_get(name)))
      {
        ast::rLocal res =
          new ast::Local(loc, name,
                         recurse_collection(input->arguments_get()),
                         routine_depth_ - depth);
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
      // Do not report errors while lazifying arguments, to avoid
      // reporting them twice.
      libport::Finally finally(libport::scoped_set(report_errors_, false));
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
  Binder::visit(const ast::CallMsg* input)
  {
    ast::rFunction fun = function();
    if (!fun)
      if (report_errors_)
        errors_.error(input->location_get(), "call: used outside any function");
    fun->uses_call_set(true);
    super_type::visit(input);
  }

  ast::rExp
  Binder::lazify (ast::rExp arg, const ast::loc& loc)
  {
    // build Lazy.clone.init(closure () { %arg })
    ast::rCall lazy = new ast::Call
      (loc, new ast::exps_type(), new ast::Implicit(loc), SYMBOL(Lazy));
    ast::rCall clone = new ast::Call(loc, new ast::exps_type(), lazy, SYMBOL(clone));
    ast::exps_type* init_args = new ast::exps_type();

    {
      // FIXME: Maybe started another Binder would be better?
      ast::rAst res;
      std::swap(res, result_);
      init_args->push_back(recurse(parser::ast_closure(arg)));
      std::swap(res, result_);
    }

    ast::rCall init = new ast::Call(loc, init_args, clone, SYMBOL(init));
    return init;
  }

  void
  Binder::visit (const ast::Foreach* input)
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
  Binder::visit (const ast::Scope* input)
  {
    result_ = new ast::Scope(input->location_get(), handleScope(input, false));
  }

  void
  Binder::visit (const ast::Do* input)
  {
    operator() (input->target_get().get());
    ast::rExp target = result_.unsafe_cast<ast::Exp>();
    result_ = new ast::Do(input->location_get(),
                          handleScope(input, true),
                          target);
  }

  static
  void
  decrement(unsigned* n)
  {
    (*n)--;
  }

  ast::rExp
  Binder::handleScope(ast::rConstScope scope, bool setOnSelf)
  {
    libport::Finally finally;

    scope_depth_++;
    finally << boost::bind(decrement, &scope_depth_);

    // Push a finally on unbind_, and destroy it at the scope
    // exit. Since bound variables register themselves for unbinding
    // in unbind_'s top element, they will be unbound at scope exit.
    unbind_.push_back(libport::Finally());
    finally << boost::bind(&unbind_type::pop_back, &unbind_);

    setOnSelf_.push_back(setOnSelf);
    finally << boost::bind(&set_on_self_type::pop_back, &setOnSelf_);

    operator() (scope->body_get().get());
    return result_.unsafe_cast<ast::Exp>();
  }

  template <typename Code>
  void
  Binder::handleRoutine(const Code* input)
  {
    BIND_ECHO("Push" << libport::incindent);
    libport::Finally finally;

    // Clone and push the function, without filling its body and arguments
    libport::shared_ptr<Code> res = new Code(input->location_get(), 0, 0);
    res->local_variables_set(new ast::local_declarations_type());
    routine_push(res);
    finally << boost::bind(&Binder::routine_pop, this);

    // Open a new scope
    unbind_.push_back(libport::Finally());
    finally << boost::bind(&unbind_type::pop_back, &unbind_);

    // Do not setOnSelf in this scope
    setOnSelf_.push_back(false);
    finally << boost::bind(&set_on_self_type::pop_back, &setOnSelf_);

    // Increase the nested functions and scopes depth
    routine_depth_++;
    finally << boost::bind(decrement, &routine_depth_);
    scope_depth_++;
    finally << boost::bind(decrement, &scope_depth_);

    // Clone and bind arguments
    res->formals_set(recurse_collection(input->formals_get()));

    // Bind and clone the body
    res->body_set(recurse(input->body_get ()));
    result_ = res;

    // Index local and closed variables
    int local = 0;
    int closed = 0;
    foreach (ast::rLocalDeclaration dec, *res->local_variables_get())
      if (dec->closed_get())
        dec->local_index_set(closed++);
      else
        dec->local_index_set(local++);

    res->local_size_set(local);
    res->closed_size_set(closed);

    // Index captured variables
    int captured = 0;
    foreach (ast::rLocalDeclaration dec, *res->captured_variables_get())
      dec->local_index_set(captured++);

    BIND_NECHO(libport::decindent);
    BIND_ECHO("Pop");

  }

  void
  Binder::visit(const ast::Function* input)
  {
    handleRoutine(input);
  }

  void
  Binder::visit(const ast::Closure* input)
  {
    handleRoutine(input);
  }

  void
  Binder::bind(ast::rLocalDeclaration decl)
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

    env_[decl->what_get()].push_back(
      std::make_pair(decl, std::make_pair(routine_depth_, scope_depth_)));
    unbind_.back() <<
      boost::bind(&Bindings::pop_back, &env_[decl->what_get()]);
  }

} // namespace binder
