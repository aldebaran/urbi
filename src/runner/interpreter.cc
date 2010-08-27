/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
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

// #define ENABLE_DEBUG_TRACES

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <libport/config.h>
#include <libport/echo.hh>
#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/format.hh>
#include <libport/lexical-cast.hh>
#include <libport/symbol.hh>

#include <ast/call.hh>
#include <ast/exp.hh>

#include <kernel/uconnection.hh>

#include <object/symbols.hh>
#include <object/urbi-exception.hh>
#include <object/system.hh>

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
                           libport::Symbol name)
    : Runner(lobby, sched, name)
    , ast_(ast)
    , code_(0)
    , result_(0)
    , stacks_(lobby)
  {
    init();
    apply_tag(lobby->tag_get());
  }

  Interpreter::Interpreter(rLobby lobby,
                           sched::Scheduler& sched,
                           rObject code,
                           libport::Symbol name,
                           rObject self,
                           const objects_type& args)
    : Runner(lobby, sched, name)
    , ast_(0)
    , code_(code)
    , this_(self)
    , args_(args)
    , result_(0)
    , stacks_(lobby)
  {
    init();
    apply_tag(lobby->tag_get());
  }

  Interpreter::Interpreter(const Interpreter& model, rObject code,
			   libport::Symbol name,
                           const objects_type& args)
    : Runner(model, name)
    , ast_(0)
    , code_(code)
    , args_(args)
    , result_(0)
    , call_stack_(model.call_stack_)
    , stacks_(model.lobby_)
  {
    tag_stack_set(model.tag_stack_get_all());
    init();
  }

  Interpreter::Interpreter(const Interpreter& model,
			   ast::rConstAst ast,
			   libport::Symbol name)
    : Runner(model, name)
    , ast_(ast)
    , code_(0)
    , result_(0)
    , call_stack_(model.call_stack_)
    , stacks_(model.lobby_)
  {
    tag_stack_set(model.tag_stack_get_all());
    init();
  }


  Interpreter::Interpreter(rLobby lobby,
                           sched::Scheduler& sched,
                           boost::function0<void> job,
                           rObject self,
                           libport::Symbol name)
    : Runner(lobby, sched, name)
    , ast_(0)
    , code_(0)
    , job_(job)
    , result_(0)
    , stacks_(self)
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
        libport::push_front(args_, this_ ? this_ : rObject(lobby_));
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
      LIBPORT_DEBUG("Implicit tag: " << *e);
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
          LIBPORT_DEBUG("Component " << elt << " exists.");
	  where = owner->local_slot_get(elt)->value();
	  if (object::Tag* parent_ = dynamic_cast<object::Tag*>(where.get()))
	  {
            LIBPORT_DEBUG("It is a tag, so use it as the new parent.");
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
    show_backtrace(call_stack_, chan);
  }

  Interpreter::backtrace_type
  Interpreter::backtrace_get() const
  {
    CAPTURE_GLOBAL(StackFrame);
    backtrace_type res;
    /* We need to create StackFrame objects while iterating, which will modify
     * the call stack, so make a copy.
     */
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
    CAPTURE_GLOBAL(Exception);

    // innermost_node_ can be empty if the interpreter has not interpreted
    // any urbiscript.  E.g., slot_set can raise an exception only from the
    // C++ side.  It would be better to produce a C++ backtrace instead.
    if (is_a(exn, Exception) && innermost_node_)
    {
      boost::optional<ast::loc> l = innermost_node_->location_get();
      exn->slot_update(SYMBOL(DOLLAR_location), object::to_urbi(l));
      exn->slot_update(SYMBOL(DOLLAR_backtrace),
                       as_job()->as<object::Job>()->backtrace());
    }
    call_stack_type bt = call_stack_get();
    if (skip_last && !bt.empty())
      bt.pop_back();
    throw object::UrbiException(exn, bt);
  }

} // namespace runner
