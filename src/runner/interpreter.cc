/**
 ** \file runner/interpreter.cc
 ** \brief Implementation of runner::Interpreter.
 */

// #define ENABLE_DEBUG_TRACES

#include <libport/compiler.hh>

#include <algorithm>
#include <deque>

#include <boost/range/iterator_range.hpp>

#include <libport/foreach.hh>
#include <libport/symbol.hh>

#include <kernel/exception.hh>
#include <kernel/uconnection.hh>

#include <object/tag-class.hh>

#include <runner/interpreter.hh>

#include <parser/uparser.hh>

namespace runner
{
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
      if (ast_)
	result_ = operator()(ast_.get());
      else
	result_ = apply(lobby_, code_, SYMBOL(task), args_);
    }
    catch(object::UrbiException& ue)
    {
      show_error_(ue);
      throw;
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

#ifndef LIBPORT_SPEED // If not in speed mode, compile visit method here
# include <runner/interpreter-visit.hxx>
#endif
