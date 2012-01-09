/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/interpreter.cc
 ** \brief Implementation of runner::Interpreter.
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <libport/config.h>
#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/format.hh>
#include <libport/lexical-cast.hh>
#include <libport/symbol.hh>

#include <ast/call.hh>
#include <ast/exp.hh>

#include <urbi/kernel/uconnection.hh>

#include <object/profile.hh>
#include <urbi/object/symbols.hh>
#include <object/system.hh>
#include <urbi/object/urbi-exception.hh>

#include <parser/uparser.hh>

#include <runner/interpreter.hh>
#include <urbi/runner/raise.hh>

#include <sched/exception.hh>

#include <urbi/object/cxx-conversions.hh>
#include <urbi/object/global.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/location.hh>
#include <urbi/object/job.hh>

// If not in speed mode, compile visit methods here.
#ifndef LIBPORT_COMPILATION_MODE_SPEED
# include <runner/interpreter-visit.hxx>
#else
GD_CATEGORY(Urbi);
#endif

namespace runner
{
  using libport::Finally;

  // This function takes an expression and attempts to decompose it
  // into a list of identifiers. The resulting chain is stored in
  // reverse order, that is the most specific tag first.
  typedef std::vector<libport::Symbol> tag_chain_type;
  static
  tag_chain_type
  decompose_tag_chain(ast::rConstExp e)
  {
    tag_chain_type res;
    while (!e->implicit())
    {
      ast::rConstCall c = e.unsafe_cast<const ast::Call>();
      if (!c || c->arguments_get())
	runner::raise_urbi(SYMBOL(ImplicitTagComponent));
      res << c->name_get();
      e = c->target_get();
    }
    return res;
  }


  /*--------------.
  | Interpreter.  |
  `--------------*/

  Interpreter::Interpreter(rLobby lobby,
                           sched::Scheduler& sched,
                           ast::rConstAst ast,
                           const std::string& name)
    : Runner(lobby, sched, name)
    , profile_(0)
    , profile_checkpoint_(0)
    , profile_function_current_(0)
    , profile_function_call_depth_(0)
    , ast_(ast)
    , code_(0)
    , result_(0)
    , stacks_(lobby)
    , innermost_node_(0)
    , current_exception_(0)
  {
    init();
    apply_tag(lobby->tag_get());
  }

  Interpreter::Interpreter(rLobby lobby,
                           sched::Scheduler& sched,
                           rObject code,
                           const std::string& name,
                           rObject self,
                           const objects_type& args)
    : Runner(lobby, sched, name)
    , profile_(0)
    , profile_checkpoint_(0)
    , profile_function_current_(0)
    , profile_function_call_depth_(0)
    , ast_(0)
    , code_(code)
    , this_(self)
    , args_(args)
    , result_(0)
    , stacks_(lobby)
    , innermost_node_(0)
    , current_exception_(0)
  {
    init();
    apply_tag(lobby->tag_get());
  }

  Interpreter::Interpreter(const Interpreter& model, rObject code,
			   const std::string& name,
                           const objects_type& args)
    : Runner(model, name)
    , profile_(0)
    , profile_checkpoint_(0)
    , profile_function_current_(0)
    , profile_function_call_depth_(0)
    , ast_(0)
    , code_(code)
    , args_(args)
    , result_(0)
    , call_stack_(model.call_stack_)
    , stacks_(model.lobby_)
    , innermost_node_(0)
    , current_exception_(0)
  {
    tag_stack_set(model.tag_stack_get_all());
    init();
  }

  Interpreter::Interpreter(const Interpreter& model,
			   ast::rConstAst ast,
			   const std::string& name)
    : Runner(model, name)
    , profile_(0)
    , profile_checkpoint_(0)
    , profile_function_current_(0)
    , profile_function_call_depth_(0)
    , ast_(ast)
    , code_(0)
    , result_(0)
    , call_stack_(model.call_stack_)
    , stacks_(model.lobby_)
    , innermost_node_(0)
    , current_exception_(0)
  {
    tag_stack_set(model.tag_stack_get_all());
    init();
  }


  Interpreter::Interpreter(rLobby lobby,
                           sched::Scheduler& sched,
                           boost::function0<void> job,
                           rObject self,
                           const std::string& name)
    : Runner(lobby, sched, name)
    , profile_(0)
    , profile_checkpoint_(0)
    , profile_function_current_(0)
    , profile_function_call_depth_(0)
    , ast_(0)
    , code_(0)
    , job_(job)
    , result_(0)
    , stacks_(self)
    , innermost_node_(0)
    , current_exception_(0)
  {
    GD_FINFO_TRACE("Spawn new interpreter \"%s\".", name);
    init();
    apply_tag(lobby->tag_get());
  }

  Interpreter::~Interpreter()
  {
  }

  void
  Interpreter::init()
  {
    // Push a dummy scope tag, in case we do have an "at" at the
    // toplevel.
    create_scope_tag();
  }

  void
  Interpreter::show_exception(const object::UrbiException& ue,
                              const std::string& tag) const
  {
    CAPTURE_GLOBAL(Exception);

    // FIXME: should bounce in all case to Exception.'$show'.
    if (is_a(ue.value_get(), Exception))
      ue.value_get()->call(SYMBOL(DOLLAR_show));
    else
    {
      send_message("error",
                   libport::format("!!! %s", *ue.value_get()));
      show_backtrace(ue.backtrace_get(), tag);
    }
  }

  void
  Interpreter::work()
  {
    try
    {
      aver(ast_ || code_ || job_);
      check_for_pending_exception();
      if (ast_)
	result_ = operator()(ast_.get());
      else if (code_)
      {
        args_.push_front(this_ ? this_ : rObject(lobby_));
	result_ = apply(code_, libport::Symbol::make_empty(), args_);
      }
      else
        job_();
    }
    catch (object::UrbiException& exn)
    {
      // If this runner has a parent, let the exception go through
      // so that it will be handled by Job::run().
      if (child_job())
	throw;
      else
      // This is a detached runner, show the error.
      {
        // Yielding inside a catch is forbidden.
        Finally finally(boost::bind(&Runner::non_interruptible_set,
                                    this, non_interruptible_get()));
        non_interruptible_set(true);
        show_exception(exn);
      }
    }
  }

  void
  Interpreter::scheduling_error(const std::string& msg)
  {
    // We may have a situation here. If the stack space is running
    // near exhaustion, we cannot reasonably hope that we will get
    // enough stack space to build an exception, which potentially
    // requires a non-negligible amount of calls. For this reason, we
    // create another job whose job is to build the exception (in a
    // freshly allocated stack) and propagate it to us as we are its
    // parent.
    //
    // Yet, if the user asked for non-interruptible mode, we do not
    // want to fire a new Runner...

    // The user requested non-interruptible mode for a reason, honor
    // it and try to throw in this job.
    if (non_interruptible_get())
      raise_scheduling_error(msg);

    CAPTURE_GLOBAL2(Exception, Scheduling);
    object::objects_type args;
    args << object::to_urbi(msg);
    sched::rJob child =
      new Interpreter(*this,
                      Scheduling->slot_get(SYMBOL(throwNew)),
                      SYMBOL(Scheduling),
                      args);
    sched::Job::Collector collector(this);
    register_child(child, collector);
    child->start_job();

    try
    {
      yield_until_terminated(*child);
    }
    catch (const sched::ChildException& ce)
    {
      try
      {
	ce.rethrow_child_exception();
      }
      catch (const object::UrbiException& ue)
      {
	raise(ue.value_get(), false);
      }
    }
  }

  object::rObject
  Interpreter::eval_tag(ast::rConstExp e)
  {
    try
    {
      // Try to evaluate e as a normal expression.
      return operator()(e.get());
    }
    catch (object::UrbiException&)
    {
      // We got a lookup error. It means that we have to automatically
      // create the tag. In this case, we only accept k1 style tags,
      // i.e. chains of identifiers, excluding function calls.
      // The reason to do that is:
      //   - we do not want to mix k1 non-declared syntax with k2
      //     clean syntax for tags
      //   - we have no way to know whether the lookup error arrived
      //     in a function call or during the direct resolution of
      //     the name.

      // `Tag.tags' represents the top level tag.
      CAPTURE_GLOBAL2(Tag, tags);
      rObject parent = tags;
      rObject where = stacks_.this_get();
      tag_chain_type chain = decompose_tag_chain(e);
      rforeach (libport::Symbol elt, chain)
      {
	// Check whether the concerned level in the chain already
	// exists.
	if (rObject owner = where->slot_locate(elt).first)
        {
          GD_FINFO_DUMP("Component %s exists.", elt);
	  where = owner->local_slot_get(elt)->value();
	  if (object::Tag* parent_ = dynamic_cast<object::Tag*>(where.get()))
	  {
            GD_INFO_DUMP("It is a tag, so use it as the new parent.");
	    parent = parent_;
	  }
        }
	else
	{
	  // We have to create a new tag, which will be attached
	  // to the upper level (hierarchical tags, implicitly
	  // rooted by Tags).
	  where = parent->call(SYMBOL(new), new object::String(elt));
	  parent->slot_set(elt, where);
	  parent = where;
	}
      }

      return where;
    }
  }

  void
  Interpreter::show_backtrace(const call_stack_type& bt,
                              const std::string& chan) const
  {
    rforeach (const call_type& c, bt)
      send_message(chan,
                   libport::format("!!!    called from: %s", c));
  }

  void
  Interpreter::show_backtrace(const std::string& chan) const
  {
    // Displaying a stack invokes urbiscript code, which in turn
    // changes the call stack.  Don't play this kind of games.
    call_stack_type stack = call_stack_;
    show_backtrace(stack, chan);
  }

  Interpreter::backtrace_type
  Interpreter::backtrace_get() const
  {
    CAPTURE_GLOBAL(StackFrame);
    backtrace_type res;
    // We need to create StackFrame objects while iterating, which
    // will modify the call stack, so make a copy.
    call_stack_type copy = call_stack_;
    foreach (call_type c, copy)
    {
      rObject loc = object::nil_class;
      if (c.second)
        loc = new object::Location(c.second.get());
      rObject frame =
        StackFrame->call("new", new object::String(c.first.name_get()), loc);
      res << frame_type(frame);
    }
    return res;
  }

  object::call_stack_type
  Interpreter::call_stack_get() const
  {
    return call_stack_;
  }

  void
  Interpreter::raise(rObject exn, bool skip_last)
  {
    raise(exn, skip_last, boost::optional<ast::loc>());
    pabort("Unreachable");
  }

  void
  Interpreter::raise(rObject exn, bool skip_last,
                     const boost::optional<ast::loc>& loc)
  {
    CAPTURE_GLOBAL(Exception);

    // innermost_node_ can be empty if the interpreter has not interpreted
    // any urbiscript.  E.g., slot_set can raise an exception only from the
    // C++ side.  It would be better to produce a C++ backtrace instead.
    if (is_a(exn, Exception) && innermost_node_)
    {
      boost::optional<ast::loc> l = loc ? loc : innermost_node_->location_get();
      exn->slot_update(SYMBOL(DOLLAR_location), object::to_urbi(l));
      exn->slot_update(SYMBOL(DOLLAR_backtrace),
                       as_job()->as<object::Job>()->backtrace());
    }
    call_stack_type bt = call_stack_get();
    if (skip_last && !bt.empty())
      bt.pop_back();
    throw object::UrbiException(exn, bt);
  }

  object::rObject
  Interpreter::eval(const ast::Ast* e, rObject self)
  {
    Stacks::context_type ctx = stacks_.push_context(self);
    rObject res;
    {
      FINALLY(
        ((Stacks&, stacks_))
        ((const Stacks::context_type&, ctx)),
        stacks_.pop_context(ctx));
      res = operator()(e);
    }
    return res;
  }

  /*------------.
  | Profiling.  |
  `------------*/

  void
  Interpreter::profile_start(Profile* profile, libport::Symbol name,
                             Object* current, bool count)
  {
    assert(!profile_);
    assert(profile);
    profile_ = profile;
    profile_checkpoint_ = libport::utime();
    profile_function_current_ = current;
    if (!profile_->functions_profile_[current])
    {
      profile_->functions_profile_[current] = new FunctionProfile;
      profile_->functions_profile_[current]->name_ = name;
    }
    if (count)
      ++profile_->functions_profile_[current]->calls_;
  }

  void
  Interpreter::profile_stop()
  {
    assert(profile_);
    profile_->step(profile_checkpoint_, profile_function_current_);
    profile_ = 0;
  }

  bool
  Interpreter::is_profiling() const
  {
    return profile_;
  }

  void
  Interpreter::hook_preempted() const
  {
    if (profile_)
    {
      profile_->step(profile_checkpoint_, profile_function_current_);
      ++profile_->yields_;
    }
  }

  void
  Interpreter::hook_resumed() const
  {
    if (profile_)
    {
      libport::utime_t now = libport::utime();
      profile_->wall_clock_time_ += now - profile_checkpoint_;
      profile_checkpoint_ = now;
    }
  }

} // namespace runner
