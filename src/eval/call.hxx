/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */


/**
 ** \file eval/call.hxx
 ** \brief Implementation of routine application
 */

#ifndef EVAL_CALL_HXX
# define EVAL_CALL_HXX

#include <libport/compiler.hh>
#include <libport/compilation.hh>
#include <libport/range.hh>

#include <ast/exps-type.hh>
#include <ast/local-declarations-type.hh>
#include <ast/factory.hh>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>
#include <ast/print.hh>
#include <ast/routine.hh>
#include <ast/transformer.hh>

#include <object/system.hh>
#include <object/code.hh>

#include <urbi/kernel/userver.hh>
#include <urbi/object/event.hh>
#include <urbi/object/global.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>
#include <urbi/object/primitive.hh>
#include <urbi/object/slot.hh>
#include <urbi/object/symbols.hh>

# include <urbi/runner/raise.hh>

#include <runner/state.hh>
#include <runner/job.hh>

#include <eval/ast.hh>
#include <eval/call.hh>

# if defined _MSC_VER || defined __arm__ || defined __clang__ || defined URBI_NO_VLENGTH_ARRAY
// Use malloc with CL.
#  define URBI_DYNAMIC_STACK_NONE   1
#  define URBI_DYNAMIC_STACK_VECTOR 0
# else
// Use variable size vectors on the stack with GCC.
#  define URBI_DYNAMIC_STACK_NONE   0
#  define URBI_DYNAMIC_STACK_VECTOR 1
# endif

namespace eval
{
  using object::rLobby;
  using object::rSlot;
  using object::Slot;
  using object::rCode;
  using object::Code;

  /*----------.
  | Helpers.  |
  `----------*/

  static inline
  void
  lazy_args(Job& job,
            object::objects_type& args,
            const object::objects_type& exp_args);

  static inline
  object::rObject
  build_call_message(Job& job,
                     object::Object* code,
                     libport::Symbol msg,
                     const object::objects_type& args);

  static inline
  object::rObject
  build_call_message(Job& job,
                     object::Object* tgt,
                     object::Object* code,
                     libport::Symbol msg,
                     const ::ast::exps_type& args);



  LIBPORT_SPEED_INLINE
  Action  call(object::rObject function,
               const object::objects_type& args)
  {
    return boost::bind(&call, _1, function, args);
  }

  LIBPORT_SPEED_INLINE
  rObject call(Job& job,
               object::rObject function,
               const object::objects_type& args)
  {
    // GD_CATEGORY(Urbi.Eval.Call);
    // GD_INFO_TRACE("Call rObject function");
    // FIXME: Duplication is the root of all Evil.
    object::objects_type args_(args);
    args_.push_front(rObject(job.state.this_get()));
    return call_apply(job, function, libport::Symbol::make_empty(), args_);
  }

  /*-----------------------------------------------------------------.
  | Apply with evaluated arguments, and potentially a call message.  |
  `-----------------------------------------------------------------*/

  LIBPORT_SPEED_INLINE
  Action  call_apply(object::rObject target,
                     object::rObject routine,
                     libport::Symbol message,
                     const object::objects_type& args)
  {
    typedef rObject (*bound)(Job& job,
                             object::rObject target,
                             object::rObject routine,
                             libport::Symbol message,
                             const object::objects_type& args);
    return boost::bind((bound) &call_apply, _1,
                       target, routine, message, args);
  }

  LIBPORT_SPEED_INLINE
  rObject call_apply(Job& job,
                     object::rObject target,
                     object::rObject routine,
                     libport::Symbol message,
                     const object::objects_type& args)
  {
    aver(routine);

    object::objects_type args_with_target(args);
    args_with_target.push_front(
      target ? target : rObject(job.state.lobby_get()));

    return call_apply(job, routine.get(), message, args_with_target, 0);
  }

  LIBPORT_SPEED_INLINE
  Action  call_apply(object::rObject function,
                     libport::Symbol msg,
                     const object::objects_type& args,
                     object::rObject call_message)
  {
    typedef rObject (*bound)(Job& job,
                             object::rObject function,
                             libport::Symbol msg,
                             const object::objects_type& args,
                             object::rObject call_message);
    return boost::bind((bound) &call_apply, _1,
                       function, msg, args,
                       call_message);
  }

  LIBPORT_SPEED_INLINE
  rObject call_apply(Job& job,
                     object::rObject function,
                     libport::Symbol msg,
                     const object::objects_type& args,
                     object::rObject call_message)
  {
    // Add empty location.
    return call_apply(job, function, msg, args, call_message,
                      boost::optional< ::ast::loc>());
  }

// We must check before poping the call stack, as it is a circular
// buffer.
#define FINALLY_Stack(DefineOrUse)              \
    FINALLY_ ## DefineOrUse                     \
    (Stack,                                     \
      ((Job&, job))((bool, reg))                \
      ((runner::Profile::idx, profile_prev)),   \
     if (reg && !job.state.call_stack_get().empty()) \
       job.state.call_stack_get().pop_back();   \
     if (job.is_profiling())                    \
       job.profile_leave(profile_prev);         \
    )

  FINALLY_Stack(DEFINE);

  LIBPORT_SPEED_INLINE
  rObject call_apply(Job& job,
                     object::Object* function,
                     libport::Symbol msg,
                     const object::objects_type& args,
                     object::Object* call_message,
                     boost::optional< ::ast::loc> loc,
                     unsigned call_flags)
  {
    GD_CATEGORY(Urbi.Call);
    GD_FPUSH_TRACE("Call %s (%s)", msg, loc ? string_cast(loc.get()) : "?");
    aver(function);
    aver(!args.empty());
    aver(args.front());

    bool reg = !msg.empty() && loc;
    // GD_FINFO_DEBUG("reg = %d", reg);
    // GD_FINFO_DEBUG("profile = %p", job.profile);

    if (reg)
      job.state.call_stack_get() << std::make_pair(msg, loc);
    runner::Profile::idx profile_prev = 0;

    if (job.is_profiling())
      profile_prev = job.profile_enter(function, msg);

    FINALLY_Stack(USE);

    // GD_INFO_DEBUG("Check for void arguments");
    // Check if any argument is void.
    foreach (object::Object* arg, libport::skip_first(args))
      if (arg == object::void_class)
	runner::raise_unexpected_void_error();

    object::rObject res;
    // GD_FINFO_DEBUG("Function f = %p", function);
    if (object::rCode code = function->as<object::Code>())
    {
      // GD_INFO_DEBUG("Function is a Code object");
      res = call_apply_urbi(job, code, msg, args, call_message, call_flags);
      // GD_FINFO_DEBUG("Function returned = %p", res.get());$
    }
    else if (const object::rPrimitive& p = function->as<object::Primitive>())
    {
      // GD_INFO_DEBUG("Function is a Primitive object");
      // GD_FINFO_DEBUG("Args: %d", args.size() - 1);
      res = p->call_raw(args, call_flags);
      // GD_FINFO_DEBUG("Function returned = %p", res.get());
    }
    else // access a slot.
    {
      // GD_INFO_DEBUG("Function is a Slot");
      if (args.size() != 1)
      {
        // GD_INFO_DEBUG("Function has arguments.");
        // FIXME: args is modified.
        if (rObject call =
            function->slot_get_value(SYMBOL(LPAREN_RPAREN), false))
	  return call_apply(job, call,
                            SYMBOL(LPAREN_RPAREN), args, call_message, loc);
      }
      // GD_FINFO_DEBUG("Ensure that no arguments are provided (%d == 0)", args.size() - 1);
      if (! (call_flags & object::Primitive::CALL_IGNORE_EXTRA_ARGS))
        object::check_arg_count(args, 0);
      return function;
    }

    return res;
  }

  /*----------------------------.
  | Apply with a call message.  |
  `----------------------------*/


  LIBPORT_SPEED_INLINE
  rObject call_msg(Job& job,
                   object::Object* function,
                   libport::Symbol msg,
                   object::Object* call_message,
                   boost::optional< ::ast::loc> loc)
   {
    rObject target = call_message->slot_get_value(SYMBOL(target));
    object::objects_type args;
    args << target;
    // This function is called when arguments haven't been evaluated:
    // only a call message is provided.  If the called function is
    // strict, we need to extract arguments values for it.  This can
    // happen when
    object::Code* c = function->as<object::Code>();
    if (!c || c->ast_get()->strict())
    {
      rObject urbi_args = call_message->call(SYMBOL(evalArgs));
      foreach (object::Object* arg,
               urbi_args->as<object::List>()->value_get())
      args << arg;
    }
    return call_apply(job, function, msg, args, call_message, loc);
  }


  /*-------------------------------------------------.
  | Apply an urbi function (i.e., not a primitive).  |
  `-------------------------------------------------*/


#define FINALLY_Frame(DefineOrUse)                      \
    FINALLY_ ## DefineOrUse                             \
    (Frame,                                             \
      ((Job&, job))                                     \
      ((libport::Symbol, msg))                          \
      ((runner::State::var_frame_type, previous_frame)) \
      ((rLobby, caller_lobby))                          \
      ((rSlot*, local_stack))                           \
      ((rSlot*, captured_stack))                        \
      ((runner::State::import_captured_type, import_captured))  \
      ((rCode, function))                               \
      ((bool, prev_has_import_stack))                   \
    ,                                                   \
     job.state.pop_frame(msg, previous_frame);          \
     job.state.lobby_set(caller_lobby);                 \
     std::swap(job.state.import_captured, import_captured); \
     /* Warning, has_import_stack may change (see load)*/   \
     /* So re-read it, do not rely on what function says*/  \
     if (job.state.has_import_stack)                        \
       job.state.import_stack.pop_back();               \
     job.state.has_import_stack = prev_has_import_stack;  \
     BOOST_PP_IF(URBI_DYNAMIC_STACK_NONE,               \
                 {                                      \
                   delete [] local_stack;               \
                   delete [] captured_stack;            \
                 },                                     \
       );                                               \
    )

  FINALLY_Frame(DEFINE);

  // !!! GD_* macros are commented because this consume stack space in speed
  // mode, even if messages are not printed.

  LIBPORT_SPEED_INLINE
  rObject call_apply_urbi(Job& job,
                          object::Code* function,
                          libport::Symbol msg,
                          const object::objects_type& args,
                          object::Object* call_message_,
                          unsigned call_flags)
  {
    // GD_CATEGORY(Urbi.Eval.Call);

    rObject call_message = call_message_;

    // The called function.
    const object::Code::ast_type& ast = function->ast_get();

    // If the function uses call and there's no call message, forge
    // one. This happen when a lazy function is invoked with eval, for
    // instance.
    if (ast->uses_call_get() && !call_message)
    {
      // GD_INFO_DEBUG("Forge callMessage");
      object::objects_type lz_args;
      lazy_args(job, lz_args, args);
      call_message = build_call_message(job, function, msg, lz_args);
    }

    // Determine the function's 'this' and 'call'
    rObject self;
    rObject call;
    rLobby caller_lobby = job.state.lobby_get();
    if (ast->closure_get())
    {
      // GD_INFO_DEBUG("Function is a closure");
      self = function->this_get();
      aver(self);
      call = function->call_get();
    }
    else
    {
      // GD_INFO_DEBUG("Function is a function");
      self = args.front();
      call = call_message;
    }

    size_t local = ast->local_size_get();
    size_t captured = ast->captured_variables_get()->size();
    // GD_FINFO_DEBUG("Function has %d local variables and %d captured variables",
    //                local, captured);
    // Use closure's lobby if there is one.
    if (ast->closure_get() && function->lobby_get())
      job.state.lobby_set(function->lobby_get());

    // Push new frames on the stacks
    local += 2;
# if URBI_DYNAMIC_STACK_VECTOR
    rSlot local_stack_space[local];
    rSlot captured_stack_space[captured];
    rSlot* local_stack = &local_stack_space[0];
    rSlot* captured_stack = &captured_stack_space[0];
#elif URBI_DYNAMIC_STACK_NONE
    // FIXME: What about alloca?
    rSlot* local_stack = new rSlot[local];
    rSlot* captured_stack = new rSlot[captured];
#else
# error "No dynamic stack policy defined."
#endif

    typedef runner::State::var_frame_type var_frame_type;
    var_frame_type previous_frame =
      job.state.push_frame(
        msg, var_frame_type(local_stack, captured_stack),
        self, call);
    // Save import capture for this frame
    runner::State::import_captured_type import_captured;
    std::swap(import_captured, job.state.import_captured);
    // Push the one to come.
    job.state.import_captured = function->imports_get();
    // If binder says we will need an import stack, push it
    if (function->ast_get()->has_imports_get())
      job.state.import_stack.push_back(std::vector<rObject>());
    bool prev_has_import_stack = job.state.has_import_stack;
    job.state.has_import_stack = function->ast_get()->has_imports_get();

    // GD_INFO_DEBUG("Push frame");
    FINALLY_Frame(USE);

    // Push captured variables
    // GD_INFO_DEBUG("Push captured variables");
    foreach (const ::ast::rConstLocalDeclaration& dec,
             *ast->captured_variables_get())
    {
      Slot* value = function->captures_get()[dec->local_index_get()];
      job.state.def_captured(dec, value);
    }

    // Bind arguments if the function is strict.
    if (ast->strict())
    {
      // GD_INFO_DEBUG("Strict function => bind arguments");
      const ::ast::local_declarations_type& formals =
        *ast->formals_get();
      unsigned int max = formals.size();
      unsigned int min = max;
      if (!formals.empty() && formals.back()->list_get())
        max = UINT_MAX;
      rforeach (const ::ast::rLocalDeclaration& dec, formals)
      {
        if (!dec->list_get() && !dec->value_get())
          break;
        --min;
      }
      if (call_flags & object::Primitive::CALL_IGNORE_EXTRA_ARGS)
        max = UINT_MAX;
      if (call_flags & object::Primitive::CALL_IGNORE_MISSING_ARGS)
        min = 0;
      // Check arity
      // GD_FINFO_DEBUG("Check args: %d in [ %d .. %d ]", args.size() - 1, min, max);
      object::check_arg_count(args, min, max);
      object::objects_type::const_iterator effective = args.begin();
      // skip target
      ++effective;
      unsigned pos = 0;
      // Bind
      foreach (::ast::rLocalDeclaration formal, formals)
        if (effective != args.end())
          if (formal->list_get())
          {
            // Remaining list arguments.
            object::rList arg = new object::List;
            do
            {
              arg->insertBack(*effective);
            } while (++effective != args.end());
            job.state.def_arg(formal, arg);
          }
          else
          {
            ++pos;
            // Validate type if specified
            if (formal->type_get())
            {
              rObject oType = eval::ast(job, formal->type_get());
              rObject res = (*effective)->call(SYMBOL(isA), oType);
              if (!res->as_bool())
              {
                runner::raise_argument_type_error(pos, *effective, oType);
              }
            }
            // Classical argument.
            job.state.def_arg(formal, *(effective++));
          }
        else
          if (formal->list_get())
            // Empty list argument.
            job.state.def_arg(formal, new object::List);
          else
          {
            // Take default value.
            // FIXME: !!! remove this cast.
            if (formal->value_get())
              job.state.def_arg(
                formal,
                eval::ast(job, ::ast::rConstAst(formal->value_get().get())));
            else
                job.state.def_arg(
                formal,
                object::nil_class);
          }
    }

    // Before calling, check that we are not exhausting the stack
    // space, for example in an infinite recursion.
    job.check_stack_space();

    // GD_INFO_DEBUG("Execution start");
    job.state.execution_starts(msg);
    return eval::ast(job, ast->body_get().get());
  }

  LIBPORT_SPEED_INLINE
  rObject call_funargs(Job& job,
                       object::Code* function,
                       libport::Symbol msg,
                       const object::objects_type& args)
  {
    return call_apply_urbi(job, function, msg, args, 0, 0);
  }

  /*-------------------------------------.
  | Apply with arguments as ast chunks.  |
  `-------------------------------------*/

  /// !!! old apply_ast

  LIBPORT_SPEED_INLINE
  rObject call_msg(Job& job,
                   rObject target,
                   libport::Symbol message,
                   const ::ast::exps_type* arguments,
                   boost::optional< ::ast::loc> location)
  {
    // Accept to call methods on void only if void itself is holding
    // the method.
    if (target == object::void_class
        && !target->local_slot_get(message))
      runner::raise_unexpected_void_error();
    rObject routine = target->slot_get(message);
    static ::ast::exps_type*  empty_args = new ::ast::exps_type();
    if (rSlot s = routine->as<Slot>())
    {
      if (s->hasLocalSlot(SYMBOL(autoEval)) && !arguments)
        arguments = empty_args;
      runner::Job& job = ::kernel::server().getCurrentRunner();
      job.state.call_stack_get() << std::make_pair(message, location);
      FINALLY((( runner::Job&, job)),
        job.state.call_stack_get().pop_back());
      routine = s->value(target);
    }
    // Bounce on the same function with routine argument.
    return call_msg(job,
                    target,
                    routine,
                    message,
                    arguments, location);
  }

  LIBPORT_SPEED_INLINE
  rObject call_msg(Job& job,
                   object::Object* target,
                   object::Object* routine,
                   libport::Symbol message,
                   const ::ast::exps_type* input_ast_args,
                   boost::optional< ::ast::loc> loc)
  {
    aver(routine);
    aver(target);

    if (!input_ast_args)
    {
      // Behavior change between urbi2 and urbi3
      static bool report = getenv("URBI_REPORT_MISSING_PAREN");
      if (report && ( routine->as<object::Code>()
        || routine->as<object::Primitive>()))
      {
        GD_CATEGORY(Urbi.Compatibility);
        GD_FWARN("Maybe missing parens at %s", loc.get());
      }
      return routine;
    }
    // Evaluated arguments. Even if the function is lazy, it holds the
    // target.
    object::objects_type args;
    args << target;
    ::ast::exps_type ast_args =
      input_ast_args ? *input_ast_args : ::ast::exps_type();

    rObject call_message;

    const object::Code* c = routine->as<object::Code>().get();

    // Build a call message if the function uses it.
    if (c && c->ast_get()->uses_call_get())
      call_message =
        build_call_message(job, target, routine, message, ast_args);

    // Unless the function is lazy, evaluate the arguments.
    if (!c || c->ast_get()->strict())
      strict_args(job, args, ast_args);

    return call_apply(job, routine, message, args, call_message, loc);
  }

  /*----------.
  | Helpers.  |
  `----------*/

  LIBPORT_SPEED_INLINE
  void
  strict_args(Job& job,
              object::objects_type& args,
              const ::ast::exps_type& exp_args)
  {
    foreach (const ::ast::rConstExp& arg, exp_args)
    {
      rObject val = ast(job, arg.get());
      // Check if any argument is void. This will be checked again in
      // Interpreter::apply_urbi, yet raising exception here gives
      // better location (the argument and not the whole function
      // invocation).
      if (val == object::void_class)
	runner::raise_unexpected_void_error();
      aver(val, *arg);
      args << val;
    }
  }

  LIBPORT_SPEED_INLINE
  void
  lazy_args(Job& /* job */,
            object::objects_type& args,
            const object::objects_type& exp_args)
  {
    args << exp_args.front();
    foreach (object::Object* o, libport::skip_first(exp_args))
    {
      CAPTURE_GLOBAL(PseudoLazy);
      rObject lazy = PseudoLazy->clone();
      lazy->slot_set_value(SYMBOL(code), o);
      args << lazy;
    }
  }

  LIBPORT_SPEED_INLINE
  object::rObject
  build_call_message(Job& job,
                     object::Object* code,
                     libport::Symbol msg,
                     const object::objects_type& args)
  {
    CAPTURE_GLOBAL(CallMessage);
    rObject res = CallMessage->clone();

    // Set the sender to be the current self. self must always exist.
    res->slot_set_value(SYMBOL(sender), job.state.this_get().get());

    // Set the target to be the object on which the function is applied.
    res->slot_set_value(SYMBOL(target), args.front());

    // Set the code slot.
    res->slot_set_value(SYMBOL(code), code);

    // Set the name of the message call.
    res->slot_set_value(SYMBOL(message), new object::String(msg));

    res->slot_set_value(SYMBOL(args), new object::List(
                    object::objects_type(args.begin() + 1, args.end())));

    return res;
  }

  class Rebinder: public ::ast::Transformer
  {
  public:
    typedef ::ast::Transformer super_type;
    Rebinder(::ast::rRoutine routine,
             object::rCode code,
             runner::State& state)
      : idx_(0)
      , routine_(routine)
      , code_(code)
      , state_(state)
    {}

  protected:
    using super_type::visit;

    LIBPORT_SPEED_INLINE
    virtual void
    visit(::ast::Routine* r)
    {
      foreach (const ::ast::rLocalDeclaration& decl,
               *r->captured_variables_get())
        transform(decl->value_get());
      result_ = r;
    }

    LIBPORT_SPEED_INLINE
    virtual void
    visit(::ast::LocalDeclaration* decl)
    {
      if (has(decls_, decl))
      {
        result_ = decl;
        return;
      }
      decls_.insert(decl);
      // Reindex declarations.
      decl->local_index_set(idx_++);
      routine_->local_size_set(idx_);
      super_type::visit(decl);
    }

    /// FIXME: Code duplication.
    LIBPORT_SPEED_INLINE
    virtual void
    visit(::ast::LocalAssignment* assignment)
    {
      ::ast::rLocalDeclaration d = assignment->declaration_get();

      // If the variable is captured, or declared locally, we're good.
      if (has(decls_, d.get()))
      {
        super_type::visit(assignment);
        return;
      }

      object::rSlot value = state_.rget_assignment(assignment);
      code_->captures_get() << value;

      // Capture the variable
      assignment->depth_set(assignment->depth_get() + 1);
      ::ast::rLocalDeclaration nd =
        new ::ast::LocalDeclaration(d->location_get(),
                                  d->what_get(), d->value_get());
      nd->local_index_set(routine_->captured_variables_get()->size());
      *routine_->captured_variables_get() << nd;
      assignment->declaration_set(0);
      super_type::visit(assignment);
      assignment->declaration_set(nd);
    }

    LIBPORT_SPEED_INLINE
    virtual void
    visit(::ast::Local* local)
    {
      ::ast::rLocalDeclaration d = local->declaration_get();

      // If the variable is captured, or declared locally, we're good.
      if (has(decls_, d.get()))
      {
        super_type::visit(local);
        return;
      }

      // Retreive the value to capture.
      object::rSlot value = state_.rget(local);
      code_->captures_get() << value;

      // Capture the variable
      local->depth_set(local->depth_get() + 1);
      ::ast::rLocalDeclaration nd =
        new ::ast::LocalDeclaration(d->location_get(),
                                  d->what_get(), d->value_get());
      nd->local_index_set(routine_->captured_variables_get()->size());
      *routine_->captured_variables_get() << nd;
      local->declaration_set(0);
      super_type::visit(local);
      local->declaration_set(nd);
    }

  private:
    unsigned idx_;
    std::set< ::ast::LocalDeclaration*> decls_;
    ::ast::rRoutine routine_;
    object::rCode code_;
    runner::State& state_;
  };

  LIBPORT_SPEED_INLINE
  object::rObject
  build_call_message(Job& job,
                     object::Object* tgt,
                     object::Object* code,
                     libport::Symbol msg,
                     const ::ast::exps_type& args)
  {
    DECLARE_LOCATION_FILE;
    // Prepare current imports, to be stored in closure around args
    std::vector<rObject> imports;
    if (job.state.has_import_stack)
      imports = job.state.import_stack.back();
    imports.insert(imports.end(),
      job.state.import_captured.begin(), job.state.import_captured.end());
    // Build the list of lazy arguments
    object::objects_type lazy_args;
    lazy_args << tgt;
    foreach (const ::ast::rConstExp& e, args)
    {
      // Create the lazy version of arguments.
      ::ast::rExp body = //const_cast< ::ast::Exp*>(e.get());
        ::ast::new_clone(e);

      ::ast::rRoutine routine =
        new ::ast::Routine(LOCATION_HERE,
                         true, new ::ast::local_declarations_type,
                         ::ast::Factory::make_scope(LOCATION_HERE, body));

      rCode closure =
        // FIXME: something fishy about the lobby here.
        new Code(routine.get(),
                 job.state.call(),
                 job.state.lobby_get(),
                 job.state.this_get());
      closure->imports_get() = imports;
      Rebinder rebind(routine, closure, job.state);
      rebind(body.get());

      CAPTURE_GLOBAL(Lazy);
      lazy_args << Lazy->call("new", closure);
    }

    return build_call_message(job, code, msg, lazy_args);
  }

} // namespace eval

#endif // ! EVAL_CALL_HXX
