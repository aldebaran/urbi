/**
 ** \file runner/interpreter-apply.cc
 ** \brief Implementation of routine application related methods of
 ** the interpreter.
 */

#include <libport/range.hh>
#include <libport/finally.hh>

#include <ast/closure.hh>
#include <ast/exps-type.hh>
#include <ast/lazy.hh>
#include <ast/local-declarations-type.hh>
#include <ast/routine.hh>
#include <ast/print.hh>

#include <object/code.hh>
#include <object/global.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/primitive.hh>
#include <object/slot.hh>
#include <object/symbols.hh>

#include <runner/interpreter.hh>
#include <runner/raise.hh>

namespace runner
{
  using libport::Finally;
  using object::Slot;
  using object::rSlot;

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
	raise_unexpected_void_error();

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

    rObject call_message;

    const object::Code* c = routine->as<object::Code>().get();

    // Build a call message if the function uses it.
    if (c && c->ast_get()->uses_call_get())
      call_message = build_call_message(target, routine, message, ast_args);

    // Unless the function is lazy, evaluate the arguments.
    if (!c || c->ast_get()->strict())
      push_evaluated_arguments(args, ast_args);

    return apply(routine, message, args, call_message, loc);
  }

  /*-----------------------------------------------------------------.
  | Apply with evaluated arguments, and potentially a call message.  |
  `-----------------------------------------------------------------*/

  object::rObject
  Interpreter::apply(const rObject& function,
                     const libport::Symbol msg,
                     const object::objects_type& args,
                     const rObject& call_message)
  {
    return apply(function, msg, args, call_message,
                 boost::optional<ast::loc>());
  }

  object::rObject
  Interpreter::apply(const rObject& function,
                     const libport::Symbol msg,
                     const object::objects_type& args,
                     const rObject& call_message,
                     boost::optional<ast::loc> loc)
  {
    precondition(function);
    precondition(!args.empty());
    precondition(args.front());
    Finally finally;

    if (msg != libport::Symbol::make_empty())
    {
      call_stack_.push_back(std::make_pair(msg, loc));
      finally << (bind(&call_stack_type::pop_back, &call_stack_));
    }

    // Check if any argument is void
    foreach (const rObject& arg, libport::skip_first(args))
      if (arg == object::void_class)
	raise_unexpected_void_error();

    if (const rCode& code = function->as<object::Code>())
      return apply_urbi (code, msg, args, call_message);
    else if (const object::rPrimitive& p = function->as<object::Primitive>())
      return p->value_get()(args);
    else
    {
      if (args.size() != 1)
      {
	rObject call = function->slot_locate(SYMBOL(LPAREN_RPAREN), false, true);
        // FIXME: args is modified.
	if (call)
        {
//          args.front() = function;
	  return apply(call, SYMBOL(LPAREN_RPAREN), args, call_message, loc);
        }
      }
      object::check_arg_count(args.size()-1, 0);
      return function;
    }
  }

  /*--------------------------.
  | Apply with a call message |
  `--------------------------*/


  object::rObject
  Interpreter::apply_call_message(const rObject& function,
                                  const libport::Symbol msg,
                                  const rObject& call_message)
  {
    return apply_call_message(function, msg, call_message,
                              boost::optional<ast::loc>());
  }

  object::rObject
  Interpreter::apply_call_message(const rObject& function,
                                  const libport::Symbol msg,
                                  const rObject& call_message,
                                  boost::optional<ast::loc> loc)
  {
    rObject target = call_message->slot_get(SYMBOL(target));
    object::objects_type args;
    args.push_back(target);
    // This function is called when arguments haven't been evaluated:
    // only a call message is provided.  If the called function is
    // strict, we need to extract arguments values for it.  This can
    // happen when
    if (!function->is_a<object::Code>()
        || function->as<object::Code>()->ast_get()->strict())
    {
      rObject urbi_args = call_message->call(SYMBOL(evalArgs));
      foreach (const rObject& arg,
	       urbi_args->as<object::List>()->value_get())
	args.push_back(arg);
    }

    return apply(function, msg, args, call_message, loc);
  }

  /*-----------------------------------------------.
  | Apply an urbi function (i.e., not a primitive) |
  `-----------------------------------------------*/

  object::rObject
  Interpreter::apply_urbi(const rCode& function,
                          const libport::Symbol& msg,
                          const object::objects_type& args,
                          const rObject& call_message)
  {
    libport::Finally finally;

    // The called function.
    const object::Code::ast_type& ast = function->ast_get();

    // Whether it's an explicit closure
    bool closure = ast.unsafe_cast<const ast::Closure>();

    // If the function is lazy and there's no call message, forge
    // one. This happen when a lazy function is invoked with eval, for
    // instance.
    if (ast->uses_call_get() && !call_message)
    {
      object::objects_type lazy_args;
      lazy_args.push_back(args.front());
      foreach (const rObject& o, libport::skip_first(args))
      {
	CAPTURE_GLOBAL(PseudoLazy);
        rObject lazy = PseudoLazy->clone();
        lazy->slot_set(SYMBOL(code), o);
        lazy_args.push_back(lazy);
      }
      const_cast<rObject&>(call_message) =
        build_call_message(function, msg, lazy_args);
    }

    // Determine the function's 'this' and 'call'
    rObject self;
    rObject call;
    if (closure)
    {
      self = function->self_get();
      assert(self);
      call = function->call_get();
    }
    else
    {
      self = args.front();
      call = call_message;
    }

    // Push new frames on the stacks
    finally << stacks_.push_frame(msg,
                                  ast->local_size_get(),
                                  ast->captured_variables_get()->size(),
                                  self, call);

    // Bind arguments if the function is strict.
    if (ast->strict())
    {
      const ast::local_declarations_type& formals =
        *ast->formals_get();
      // Check arity
      object::check_arg_count (args.size() - 1, formals.size());
      object::objects_type::const_iterator it = args.begin();
      // skip target
      ++it;
      // Bind
      foreach (const ast::rConstLocalDeclaration& s, formals)
        stacks_.def_arg(s, *(it++));
    }

    // Push captured variables
    foreach (const ast::rConstLocalDeclaration& dec,
             *ast->captured_variables_get())
    {
      const rSlot& value = function->captures_get()[dec->local_index_get()];
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
  Interpreter::push_evaluated_arguments(object::objects_type& args,
					const ast::exps_type& ue_args)
  {
    foreach (const ast::rConstExp& arg, ue_args)
    {
      rObject val = operator()(arg.get());
      // Check if any argument is void. This will be checked again in
      // Interpreter::apply_urbi, yet raising exception here gives
      // better location (the argument and not the whole function
      // invocation).
      if (val == object::void_class)
	raise_unexpected_void_error();
      passert (*arg, val);
      args.push_back (val);
    }
  }

  object::rObject
  Interpreter::build_call_message (const rObject& code,
                                   const libport::Symbol& msg,
				   const object::objects_type& args)
  {
    CAPTURE_GLOBAL(CallMessage);
    rObject res = CallMessage->clone();

    // Set the sender to be the current self. self must always exist.
    res->slot_set (SYMBOL(sender), stacks_.self().get());

    // Set the target to be the object on which the function is applied.
    res->slot_set (SYMBOL(target), args.front());

    // Set the code slot.
    res->slot_set (SYMBOL(code), code);

    // Set the name of the message call.
    res->slot_set (SYMBOL(message), new object::String(msg));

    res->slot_set (SYMBOL(args), new object::List(
                     objects_type(
                       boost::begin(libport::skip_first(args)),
                       boost::end(libport::skip_first(args)))));

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
    lazy_args.push_back(tgt);
    foreach (const ast::rConstExp& e, args)
    {
      /// Retreive and evaluate the lazy version of arguments.
      const ast::rConstLazy& lazy = e.unsafe_cast<const ast::Lazy>();
      assert(lazy);
      rObject v = operator()(lazy->lazy_get().get());
      lazy_args.push_back(v);
    }

    return build_call_message(code, msg, lazy_args);
  }

  object::rCode
  Interpreter::make_routine(ast::rConstRoutine e) const
  {
    return new object::Code(e);
  }
}

