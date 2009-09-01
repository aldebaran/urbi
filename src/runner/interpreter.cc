/**
 ** \file runner/interpreter.cc
 ** \brief Implementation of runner::Interpreter.
 */

// #define ENABLE_DEBUG_TRACES

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <libport/compiler.hh>
#include <libport/config.h>
#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/format.hh>
#include <libport/lexical-cast.hh>
#include <libport/symbol.hh>

#include <ast/call.hh>
#include <ast/exp.hh>

#include <kernel/uconnection.hh>

#include <object/cxx-conversions.hh>
#include <object/global.hh>
#include <object/lobby.hh>
#include <object/symbols.hh>
#include <object/task.hh>
#include <object/urbi-exception.hh>

#include <runner/interpreter.hh>
#include <runner/raise.hh>

#include <parser/uparser.hh>

#include <sched/exception.hh>

namespace runner
{
  using libport::Finally;

  // This function takes an expression and attempts to decompose it
  // into a list of identifiers. The resulting chain is stored in
  // reverse order, that is the most specific tag first.
  typedef std::vector<libport::Symbol> tag_chain_type;
  static
  tag_chain_type
  decompose_tag_chain (ast::rConstExp e)
  {
    tag_chain_type res;
    while (!e->implicit())
    {
      ast::rConstCall c = e.unsafe_cast<const ast::Call>();
      if (!c || c->arguments_get())
	runner::raise_urbi(SYMBOL(ImplicitTagComponentError));
      res.push_back (c->name_get());
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
                           const libport::Symbol& name)
    : Runner(lobby, sched, name)
    , ast_(ast)
    , code_(0)
    , result_(0)
    , stacks_(lobby)
  {
    init();
    apply_tag(lobby->slot_get(SYMBOL(connectionTag))->as<object::Tag>(), 0);
  }

  Interpreter::Interpreter(const Interpreter& model, rObject code,
			   const libport::Symbol& name,
                           const objects_type& args)
    : Runner(model, name)
    , ast_(0)
    , code_(code)
    , args_(args)
    , result_(0)
    , call_stack_(model.call_stack_)
    , stacks_(model.lobby_)
  {
    tag_stack_set(model.tag_stack_get());
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
    tag_stack_set(model.tag_stack_get());
    init();
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
  Interpreter::show_exception_(object::UrbiException& ue)
  {
    send_message("error",
                 libport::format("!!! %s",
                                 (ue.value_get()
                                  ->call(SYMBOL(asString))
                                  ->as<object::String>()
                                  ->value_get())));
    show_backtrace(ue.backtrace_get(), "error");
  }

  void
  Interpreter::work()
  {
    try
    {
      assert(ast_ || code_);
      check_for_pending_exception();
      if (ast_)
	result_ = operator()(ast_.get());
      else
      {
        libport::push_front(args_, lobby_);
	result_ = apply(code_, libport::Symbol::make_empty(), args_);
      }
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
        show_exception_(exn);
      }
    }
  }

  void
  Interpreter::scheduling_error(const std::string& msg)
  {
    libport::Finally finally;
    sched::Job::ChildrenCollecter children(this, 1);
    // We may have a situation here. If the stack space is running
    // near exhaustion, we cannot reasonably hope that we will get
    // enough stack space to build an exception, which potentially
    // requires a non-negligible amount of calls. For this reason, we
    // create another job whose task is to build the exception (in a
    // freshly allocated stack) and propagate it to us as we are its
    // parent.
    CAPTURE_GLOBAL(SchedulingError);
    object::objects_type args;
    args.push_back(object::to_urbi(msg));
    sched::rJob child =
      new Interpreter(*this,
		      SchedulingError->slot_get(SYMBOL(throwNew)),
		      SYMBOL(SchedulingError),
		      args);
    register_child(child, children);
    child->start_job();

    try
    {
      // Clear the non-interruptible flag so that we do not
      // run into an error while waiting for our child.
      bool non_interruptible = non_interruptible_get();
      Job* job = this;
      FINALLY(((Job*, job))((bool, non_interruptible)),
              job->non_interruptible_set(non_interruptible));
      non_interruptible_set(false);
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
      ECHO("Implicit tag: " << *e);
      // We got a lookup error. It means that we have to automatically
      // create the tag. In this case, we only accept k1 style tags,
      // i.e. chains of identifiers, excluding function calls.
      // The reason to do that is:
      //   - we do not want to mix k1 non-declared syntax with k2
      //     clean syntax for tags
      //   - we have no way to know whether the lookup error arrived
      //     in a function call or during the direct resolution of
      //     the name.

      // `Tags' represents the top level tag.
      CAPTURE_GLOBAL(Tags);
      rObject parent = Tags;
      rObject where = stacks_.self();
      tag_chain_type chain = decompose_tag_chain(e);
      rforeach (const libport::Symbol& elt, chain)
      {
	// Check whether the concerned level in the chain already
	// exists.
	if (rObject owner = where->slot_locate(elt).first)
        {
          ECHO("Component " << elt << " exists.");
	  where = owner->local_slot_get(elt)->value();
	  if (object::Tag* parent_ = dynamic_cast<object::Tag*>(where.get()))
	  {
            ECHO("It is a tag, so use it as the new parent.");
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
                              const std::string& chan)
  {
    rforeach (const call_type& c, bt)
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
      if (c.second)
        o << c.second.get();
      res.push_back(frame_type(c.first.name_get(), o.str()));
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
    if (is_a(exn, Exception))
    {
      assert(innermost_node_);
      exn->slot_update
        (SYMBOL(location),
         new object::String(string_cast(innermost_node_->location_get())));
      exn->slot_update
        (SYMBOL(backtrace),
         as_task()->as<object::Task>()->backtrace());
    }
    call_stack_type bt = call_stack_get();
    if (skip_last && !bt.empty())
      bt.pop_back();
    throw object::UrbiException(exn, bt);
  }

} // namespace runner

// If not in speed mode, compile visit methods here.
#ifndef LIBPORT_COMPILATION_MODE_SPEED
# include <runner/interpreter-visit.hxx>
#endif
