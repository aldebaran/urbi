/**
 ** \file runner/interpreter-apply.cc
 ** \brief Implementation of routine application related methods of
 ** the interpreter.
 */

#include <libport/finally.hh>

#include <ast/closure.hh>
#include <ast/exps-type.hh>
#include <ast/lazy.hh>
#include <ast/local-declarations-type.hh>
#include <ast/routine.hh>
#include <ast/print.hh>

#include <object/code-class.hh>
#include <object/global-class.hh>
#include <object/list-class.hh>
#include <object/object.hh>
#include <object/primitive-class.hh>
#include <object/symbols.hh>

#include <runner/call.hh>
#include <runner/interpreter.hh>

namespace runner
{
  using libport::Finally;

  // Apply methods summary:
  //
  // Location in all apply methods is used to register the call
  // location it the call stack. It is optional because call
  // originating from C++ have no locations.
  //
  // * apply_ast(target, message, args)
  //
  // Call %target.%message(%args), args being given as ast
  // chunks. This enable to either evaluate the arguments, either
  // build a call message.
  //
  // * apply_ast(target, function, function, message, args)
  //
  // Same as above, but both target and function are specified. This
  // enable to call a method with another target than the holder of
  // the function
  //
  // * apply(function, msg, args, call_message)
  //
  // Apply %function.  If the function is strict, you must give the
  // arguments in args, the first being the target.  If it is lazy,
  // you might either give the call message, either the arguments, in
  // which case a call message will be forged.  A call message must be
  // forged when a lazy function is called from C++ or with eval: we
  // only have the evaluated arguments.


  /*-------------------------------------.
  | Apply with arguments as ast chunks.  |
  `-------------------------------------*/

  Interpreter::rObject
  Interpreter::apply_ast (const rObject& target,
                      const libport::Symbol& message,
                      const ast::exps_type* arguments,
                      boost::optional<ast::loc> location)
  {
    // Accept to call methods on void only if void itself is holding
    // the method.
    if (target == object::void_class)
      if (!target->own_slot_get(message))
        throw object::WrongArgumentType (message);

    // Bounce on apply_ast overload
    return apply_ast(target,
                     target->slot_get(message),
                     message,
                     arguments, location);
  }

  Interpreter::rObject
  Interpreter::apply_ast (const rObject& target,
                          const rObject& routine,
                          const libport::Symbol& message,
                          const ast::exps_type* input_ast_args,
                          boost::optional<ast::loc> loc)
  {
    assertion(routine);
    assertion(target);

    // Evaluated arguments. Even if the function is lazy, it holds the
    // target.
    object::objects_type args;
    args.push_back(target);

    ast::exps_type ast_args =
      input_ast_args ? *input_ast_args : ast::exps_type();

    // FIXME: This is the target, for compatibility reasons. We need
    // to remove this, and stop assuming that arguments start at
    // calls.args.nth(1)
    ast_args.push_front(0);

    rObject call_message;

    const object::Code* c = routine->as<object::Code>().get();
    if (c && !c->ast_get()->strict())
      // If the function is lazy, build a call message.
      call_message = build_call_message (target, routine, message, ast_args);
    else
      // Otherwise, evaluate the arguments.
      push_evaluated_arguments (args, ast_args);
    return apply(routine, message, args, loc, call_message);
  }

  /*-----------------------------------------------------------.
  | Apply with either a call message, either already evaluated |
  | arguments                                                  |
  `-----------------------------------------------------------*/

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

    if (const rCode& c = func->as<object::Code>())
      return apply_urbi (c, msg, args, call_message);
    else if (const object::rPrimitive& p = func->as<object::Primitive>())
      return p->value_get()(*this, args);
    else
    {
      object::check_arg_count (1, args.size(), msg);
      return func;
    }
  }

  /*-----------------------------------------------.
  | Apply an urbi function (i.e., not a primitive) |
  `-----------------------------------------------*/

  object::rObject
  Interpreter::apply_urbi (rCode func,
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
      const ast::local_declarations_type& formals =
        *ast->formals_get();
      // Check arity
      object::check_arg_count (formals.size() + 1, args.size(), msg);
      // Skip 'this'
      object::objects_type::const_iterator it = args.begin() + 1;
      // Bind
      foreach (const ast::rConstLocalDeclaration& s, formals)
        stacks_.def_arg(s, *(it++));
    }

    // Push captured variables
    foreach (const ast::rConstLocalDeclaration& dec,
             *ast->captured_variables_get())
    {
      const rrObject& value = func->captures_get()[dec->local_index_get()];
      stacks_.def_captured(dec, value);
    }

    // Before calling, check that we are not exhausting the stack
    // space, for example in an infinite recursion.
    check_stack_space ();

    stacks_.execution_starts(msg);
    return operator()(ast->body_get().get());
  }

  /*--------.
  | Helpers |
  `--------*/

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
      rObject val = operator()(arg.get());
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
      rObject v = operator()(lazy->lazy_get().get());
      lazy_args.push_back(v);
    }

    return build_call_message(tgt, code, msg, lazy_args);
  }

  object::rCode
  Interpreter::make_routine(ast::rConstRoutine e) const
  {
    return new object::Code(e);
  }
}
