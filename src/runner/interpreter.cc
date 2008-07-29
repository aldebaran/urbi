/**
 ** \file runner/interpreter.cc
 ** \brief Implementation of runner::Interpreter.
 */

// #define ENABLE_DEBUG_TRACES

#include <libport/compiler.hh>

#include <algorithm>
#include <deque>

#include <boost/range/iterator_range.hpp>

#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/symbol.hh>

#include <kernel/exception.hh>
#include <kernel/uconnection.hh>

#include <ast/all.hh>
#include <ast/declarations-type.hh>
#include <ast/exps-type.hh>
#include <ast/print.hh>

#include <object/code-class.hh>
#include <object/float-class.hh>
#include <object/global-class.hh>
#include <object/list-class.hh>
#include <object/object.hh>
#include <object/primitive-class.hh>
#include <object/symbols.hh>
#include <object/tag-class.hh>
#include <object/urbi-exception.hh>

#include <runner/call.hh>
#include <runner/interpreter.hh>

#include <parser/uparser.hh>

namespace runner
{

  using boost::bind;
  using boost::ref;
  using libport::Finally;

/// Address of \a Interpreter seen as a \c Job (Interpreter has multiple inheritance).
#define JOB(Interpreter) static_cast<scheduler::Job*> (Interpreter)

/// Address of \c this seen as a \c Job (Interpreter has multiple inheritance).
#define ME JOB (this)

#define AST(Ast)                                \
  (Ast)->location_get ()                        \
  << libport::incendl                           \
  << "{{{"                                      \
  << libport::incendl                           \
  << Ast                                        \
  << libport::decendl                           \
  << "}}}"                                      \
  << libport::decindent

/// Job echo.
#define JECHO(Title, Content)                           \
  ECHO ("job " << ME << ", " Title ": " << Content)

/// Job & astecho.
#define JAECHO(Title, Ast)                      \
  JECHO (Title, AST(Ast))

/* Yield and trace. */
#define YIELD()                                 \
  do                                            \
  {                                             \
    ECHO ("job " << ME << " yielding on AST: "  \
	  << &e << ' ' << AST(e));              \
    yield ();                                   \
  } while (0)


  // This function takes an expression and attempts to decompose it
  // into a list of identifiers.
  typedef std::deque<libport::Symbol> tag_chain_type;
  static
  tag_chain_type
  decompose_tag_chain (ast::rConstExp e)
  {
    tag_chain_type res;
    while (!e->implicit())
    {
      ast::rConstCall c = e.unsafe_cast<const ast::Call>();
      if (!c || c->arguments_get())
        throw object::ImplicitTagComponentError(e->location_get());
      res.push_front (c->name_get());
      e = c->target_get();
    }
    return res;
  }




  /*--------------.
  | Interpreter.  |
  `--------------*/


  Interpreter::Interpreter (rLobby lobby,
			    scheduler::Scheduler& sched,
			    ast::rConstAst ast,
			    const libport::Symbol& name)
    : Runner(lobby, sched, name)
    , ast_(ast)
    , code_(0)
    , result_(0)
    , stacks_(lobby)
  {
    init();
  }

  Interpreter::Interpreter(const Interpreter& model, rObject code,
			   const libport::Symbol& name, const objects_type& args)
    : Runner(model, name)
    , ast_(0)
    , code_(code)
    , args_(args)
    , result_(0)
    , call_stack_(model.call_stack_)
    , stacks_(model.lobby_)
  {
    init();
  }

  Interpreter::Interpreter(const Interpreter& model,
			   ast::rConstAst ast,
			   const libport::Symbol& name)
    : Runner(model, name)
    , ast_(ast)
    , code_(0)
    , result_(0)
    , stacks_(model.lobby_)
  {
    init();
  }

  Interpreter::~Interpreter ()
  {
  }

  void
  Interpreter::init()
  {
    // If the lobby has a slot connectionTag, push it unless it is already
    // present.
    rObject connection_tag = lobby_->slot_locate(SYMBOL(connectionTag));
    if (connection_tag)
    {
      scheduler::rTag tag =
	extract_tag(connection_tag->slot_get(SYMBOL(connectionTag)));
      if (!libport::has(tags_get(), tag))
	push_tag(tag);
    }
    // Push a dummy scope tag, in case we do have an "at" at the
    // toplevel.
    create_scope_tag();
  }

  void
  Interpreter::show_error_ (object::UrbiException& ue)
  {
    if (ue.was_displayed())
      return;
    ue.set_displayed();
    std::ostringstream o;
    o << "!!! " << ue.location_get () << ": " << ue.what ();
    send_message("error", o.str ());
    show_backtrace(ue.backtrace_get(), "error");
  }

  void
  Interpreter::propagate_error_(object::UrbiException& ue, const ast::loc& l)
  {
    // Reset the current result: there was an error so whatever value
    // it has, it must not be used.
    result_.reset();
    if (!ue.location_is_set())
      ue.location_set(l);
    if (!ue.backtrace_is_set())
      ue.backtrace_set(call_stack_);
  }

  void
  Interpreter::work ()
  {
    try
    {
      assert (ast_ || code_);
      check_for_pending_exception();
      JAECHO ("starting evaluation of AST: ", ast_);
      if (ast_)
	result_ = operator()(ast_.get());
      else
      {
	args_.push_front(lobby_);
	result_ = apply(code_, SYMBOL(task), args_);
      }
    }
    catch(object::UrbiException& ue)
    {
      show_error_(ue);
      throw;
    }
  }

  // Apply a function written in Urbi.
  object::rObject
  Interpreter::apply_urbi (rRoutine func,
                           const libport::Symbol& msg,
                           const object::objects_type& args,
                           rObject call_message)
  {
    libport::Finally finally;

    // The called function.
    const object::Code::ast_type& ast = func->ast_get();

    // Whether it's an explicit closure
    bool closure = ast.unsafe_cast<const ast::Closure>();

    // If the function is lazy and there's no call message, forge
    // one. This happen when a lazy function is invoked with eval, for
    // instance.
    if (!ast->strict() && !call_message)
    {
      object::objects_type lazy_args;
      foreach (const rObject& o, args)
      {
        rObject lazy = object::global_class->slot_get(SYMBOL(Lazy))->clone();
        lazy->slot_set(SYMBOL(code), o);
        lazy_args.push_back(lazy);
      }
      call_message = build_call_message(args[0], func, msg, lazy_args);
    }

    // Determine the function's 'this' and 'call'
    rObject self;
    rObject call;
    if (closure)
    {
      self = func->self_get();
      assert(self);
      // FIXME: The call message can be undefined at the creation
      // site for now.
      // assert(fn.call);
      call = func->call_get();
    }
    else
    {
      self = args[0];
      call = call_message;
    }

    // Push new frames on the stacks
    finally << stacks_.push_frame(msg,
                                  ast->local_size_get(),
                                  ast->closed_size_get(),
                                  ast->captured_variables_get()->size(),
                                  self, call);

    // Bind arguments if the function is strict.
    if (ast->strict())
    {
      const ast::declarations_type& formals = *ast->formals_get();
      // Check arity
      object::check_arg_count (formals.size() + 1, args.size(), msg);
      // Skip 'this'
      object::objects_type::const_iterator it = args.begin() + 1;
      // Bind
      foreach (const ast::rConstDeclaration& s, formals)
        stacks_.def_arg(s, *(it++));
    }

    // Push captured variables
    foreach (const ast::rConstDeclaration& dec, *ast->captured_variables_get())
    {
      const rrObject& value = func->captures_get()[dec->local_index_get()];
      stacks_.def_captured(dec, value);
    }

    // Before calling, check that we are not exhausting the stack
    // space, for example in an infinite recursion.
    check_stack_space ();

    stacks_.execution_starts(msg);
    return eval (ast->body_get());
  }

  object::rObject
  Interpreter::apply(const rObject& func,
                     const libport::Symbol msg,
                     object::objects_type& args,
                     rObject call_message)
  {
    return apply(func, msg, args, boost::optional<ast::loc>(), call_message);
  }


  object::rObject
  Interpreter::apply(const rObject& func,
                     const libport::Symbol msg,
                     object::objects_type& args,
                     boost::optional<ast::loc> loc,
                     rObject call_message)
  {
    precondition(func);

    call_stack_.push_back(std::make_pair(msg, loc));
    Finally finally(bind(&call_stack_type::pop_back, &call_stack_));

    // If we try to call a C++ primitive with a call message, make it
    // look like a strict function call
    if (call_message &&
	(!func->is_a<object::Code>()
	 || func->as<object::Code>()->ast_get()->strict()))
    {
      rObject urbi_args = urbi_call(*this, call_message, SYMBOL(evalArgs));
      foreach (const rObject& arg,
	       urbi_args->as<object::List>()->value_get())
	args.push_back(arg);
      call_message = 0;
    }

    // Even with call message, there is at least one argument: self.
    assert (!args.empty());
    // If we use a call message, "self" is the only argument.
    assert (!call_message || args.size() == 1);

    // Check if any argument is void
    object::objects_type::iterator end = args.end();
    if (std::find(++args.begin(), end, object::void_class) != end)
      throw object::WrongArgumentType (msg);

    if (const rRoutine& c = func->as<object::Code>())
      return apply_urbi (c, msg, args, call_message);
    else if (const object::rPrimitive& p = func->as<object::Primitive>())
      return p->value_get()(*this, args);
    else
    {
      object::check_arg_count (1, args.size(), msg);
      return func;
    }
  }

  void
  Interpreter::push_evaluated_arguments (object::objects_type& args,
					 const ast::exps_type& ue_args)
  {
    bool tail = false;
    foreach (const ast::rConstExp& arg, ue_args)
    {
      // Skip target, the first argument.
      if (!tail++)
	continue;
      rObject val = eval (arg);
      // Check if any argument is void. This will be checked again in
      // Interpreter::apply_urbi, yet raising exception here gives
      // better location (the argument and not the whole function
      // invocation).
      if (val == object::void_class)
      {
	object::WrongArgumentType e(SYMBOL());
	e.location_set(arg->location_get());
	throw e;
      }

      passert (*arg, val);
      args.push_back (val);
    }
  }

  object::rObject
  Interpreter::build_call_message (const rObject& tgt,
				   const rObject& code,
                                   const libport::Symbol& msg,
				   const object::objects_type& args)
  {
    rObject res = object::global_class->slot_get(SYMBOL(CallMessage))->clone();

    // Set the sender to be the current self. self must always exist.
    res->slot_set (SYMBOL(sender), stacks_.self());

    // Set the target to be the object on which the function is applied.
    res->slot_set (SYMBOL(target), tgt);

    // Set the code slot.
    res->slot_set (SYMBOL(code), code);

    // Set the name of the message call.
    res->slot_set (SYMBOL(message), new object::String(msg));

    res->slot_set (SYMBOL(args), new object::List(args));

    return res;
  }

  object::rObject
  Interpreter::build_call_message (const rObject& tgt,
				   const rObject& code,
				   const libport::Symbol& msg,
				   const ast::exps_type& args)
  {
    // Build the list of lazy arguments
    object::objects_type lazy_args;
    boost::sub_range<const ast::exps_type> range(args);
    // The target can be unspecified.
    if (!args.front() || args.front()->implicit())
    {
      lazy_args.push_back(object::nil_class);
      range = make_iterator_range(range, 1, 0);
    }
    foreach (const ast::rConstExp& e, range)
    {
      /// Retreive and evaluate the lazy version of arguments.
      const ast::rConstLazy& lazy = e.unsafe_cast<const ast::Lazy>();
      assert(lazy);
      rObject v = eval(lazy->lazy_get());
      lazy_args.push_back(v);
    }

    return build_call_message(tgt, code, msg, lazy_args);
  }

  Interpreter::rObject
  Interpreter::apply (const rObject& tgt, const libport::Symbol& message,
                      const ast::exps_type* input_ast_args,
                      boost::optional<ast::loc> loc)
  {
    rObject value = tgt->slot_get(message);
    // Accept to call methods on void only if void itself is holding
    // the method.
    if (tgt == object::void_class)
      if (!tgt->own_slot_get(message))
        throw object::WrongArgumentType (message);
    assert(value);
    return apply(tgt, value, message, input_ast_args, loc);
  }

  Interpreter::rObject
  Interpreter::apply (const rObject& tgt, const rObject& val,
                      const libport::Symbol& message,
                      const ast::exps_type* input_ast_args,
                      boost::optional<ast::loc> loc)
  {
    assertion(val);

    /*-------------------------.
    | Evaluate the arguments.  |
    `-------------------------*/

    // Gather the arguments, including the target.
    object::objects_type args;
    args.push_back(tgt);

    ast::exps_type ast_args =
      input_ast_args ? *input_ast_args : ast::exps_type();

    // FIXME: This is the target, for compatibility reasons. We need
    // to remove this, and stop assuming that arguments start at
    // calls.args.nth(1)
    ast_args.push_front(0);

    // Build the call message for non-strict functions, otherwise
    // the evaluated argument list.
    rObject call_message;
    const object::Code* c = val->as<object::Code>().get();
    if (c && !c->ast_get()->strict())
      call_message = build_call_message (tgt, val, message, ast_args);
    else
      push_evaluated_arguments (args, ast_args);
    return apply(val, message, args, loc, call_message);
  }

  object::rRoutine
  Interpreter::make_routine(ast::rConstRoutine e) const
  {
    return new object::Code(e);
  }

  object::rObject
  Interpreter::eval_tag(ast::rConstExp e)
  {
    try {
      // Try to evaluate e as a normal expression.
      return eval(e);
    }
    catch (object::LookupError& ue)
    {
      ECHO("Implicit tag: " << *e);
      // We got a lookup error. It means that we have to automatically
      // create the tag. In this case, we only accept k1 style tags,
      // i.e. chains of identifiers, excluding function calls.
      // The reason to do that is:
      //   - we do not want to mix k1 non-declared syntax with k2
      //     clean syntax for tags
      //   - we have no way to know whether the lookup error arrived
      //     in a function call or during the direct resolution of
      //     the name

      // Tag represents the top level tag
      const rObject& toplevel = object::tag_class;
      rObject parent = toplevel;
      rObject where = stacks_.self();
      tag_chain_type chain = decompose_tag_chain(e);
      foreach (const libport::Symbol& elt, chain)
      {
	// Check whether the concerned level in the chain already
	// exists.
	if (const rObject& owner = where->slot_locate (elt))
        {
          ECHO("Component " << elt << " exists.");
	  where = owner->own_slot_get (elt);
	  object::Tag* parent_ = dynamic_cast<object::Tag*>(where.get());
	  if (parent_)
	  {
            ECHO("It is a tag, so use it as the new parent.");
	    parent = parent_;
	  }
        }
	else
	{
          ECHO("Creating component " << elt << ".");
	  // We have to create a new tag, which will be attached
	  // to the upper level (hierarchical tags, implicitly
	  // rooted by Tag).
          object::objects_type args;
          args.push_back(parent);
          args.push_back(new object::String(elt));
	  where =
	    object::Tag::_new(args);
	  parent->slot_set(elt, where);
	  parent = where;
	}
      }

      return where;
    }
  }

  void
  Interpreter::show_backtrace(const call_stack_type& bt,
                              const std::string& chan)
  {
    rforeach (call_type c, bt)
    {
      std::ostringstream o;
      o << "!!!    called from: ";
      if (c.second)
        o << *c.second << ": ";
      o << c.first;
      send_message(chan, o.str());
    }
  }

  void
  Interpreter::show_backtrace(const std::string& chan)
  {
    show_backtrace(call_stack_, chan);
  }

  Interpreter::backtrace_type
  Interpreter::backtrace_get() const
  {
    backtrace_type res;
    foreach (call_type c, call_stack_)
    {
      std::ostringstream o;
      o << c.second;
      res.push_back(std::make_pair(c.first.name_get(), o.str()));
    }
    return res;
  }

} // namespace runner
