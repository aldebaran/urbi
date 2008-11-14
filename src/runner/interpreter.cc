/**
 ** \file runner/interpreter.cc
 ** \brief Implementation of runner::Interpreter.
 */

// #define ENABLE_DEBUG_TRACES

#include <libport/compiler.hh>
#include <libport/finally.hh>

#include <algorithm>
#include <deque>

#include <boost/range/iterator_range.hpp>

#include <libport/foreach.hh>
#include <libport/symbol.hh>

#include <kernel/uconnection.hh>

#include <object/cxx-conversions.hh>
#include <object/global.hh>
#include <object/lobby.hh>
#include <object/task.hh>

#include <runner/call.hh>
#include <runner/interpreter.hh>
#include <runner/raise.hh>

#include <parser/uparser.hh>

#include <scheduler/exception.hh>

namespace runner
{
  using libport::Finally;

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
	runner::raise_urbi(SYMBOL(ImplicitTagComponentError));
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
    tag_stack_.push_back
      (lobby->slot_get(SYMBOL(connectionTag))->as<object::Tag>());
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
    , tag_stack_(model.tag_stack_)
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
    , tag_stack_(model.tag_stack_)
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
    // present directly or indirectly.
    rObject connection_tag = lobby_->slot_locate(SYMBOL(connectionTag));
    if (connection_tag)
    {
      scheduler::rTag tag =
	extract_tag(connection_tag->slot_get(SYMBOL(connectionTag)));
      if (!has_tag(*tag))
	apply_tag(tag, 0);
    }
    // Push a dummy scope tag, in case we do have an "at" at the
    // toplevel.
    create_scope_tag();
  }

  void
  Interpreter::show_exception_ (object::UrbiException& ue)
  {
    rObject str = urbi_call(*this, ue.value_get(), SYMBOL(asString));
    std::ostringstream o;
    o << "!!! " << str->as<object::String>()->value_get();
    send_message("error", o.str ());
    show_backtrace(ue.backtrace_get(), "error");
  }

  void
  Interpreter::work ()
  {
    try
    {
      assert (ast_ || code_);
      check_for_pending_exception();
      if (ast_)
	result_ = operator()(ast_.get());
      else
	result_ = apply(lobby_, code_, libport::Symbol::make_empty(), args_);
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
        // Yielding inside a catch is forbidden
        Finally finally (boost::bind(&Runner::non_interruptible_set,
                                     this, non_interruptible_get()));
        non_interruptible_set(true);
        show_exception_(exn);
      }
    }
  }

  void
  Interpreter::scheduling_error(std::string msg)
  {
    libport::Finally finally;
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
    scheduler::rJob child =
      new Interpreter(*this,
		      SchedulingError->slot_get(SYMBOL(throwNew)),
		      SYMBOL(SchedulingError),
		      args);
    register_child(child, finally);
    child->start_job();

    try
    {
      // Clear the non-interruptible flag so that we do not
      // run into an error while waiting for our child.
      finally << boost::bind(&Job::non_interruptible_set, this,
			     non_interruptible_get());
      non_interruptible_set(false);
      yield_until_terminated(*child);
    }
    catch (const scheduler::ChildException& ce)
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
      //     the name

      // Tag represents the top level tag
      CAPTURE_GLOBAL(Tags);
      const rObject& toplevel = Tags;
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
	  // We have to create a new tag, which will be attached
	  // to the upper level (hierarchical tags, implicitly
	  // rooted by Tags).
	  where = object::urbi_call
            (*this, parent, SYMBOL(new), new object::String(elt));
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
      if (c.second)
        o << c.second.get();
      res.push_back(std::make_pair(c.first.name_get(), o.str()));
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
      std::stringstream o;
      o << innermost_node_->location_get();
      exn->slot_update(*this, SYMBOL(location),
                       new object::String(o.str()));
      exn->slot_update(*this, SYMBOL(backtrace),
                       as_task()->as<object::Task>()->backtrace());
    }
    call_stack_type bt = call_stack_get();
    if (skip_last && !bt.empty())
      bt.pop_back();
    throw object::UrbiException(exn, bt);
  }

} // namespace runner

#ifndef LIBPORT_SPEED // If not in speed mode, compile visit method here
# include <runner/interpreter-visit.hxx>
#endif
