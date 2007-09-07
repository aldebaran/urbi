/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */
//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <boost/foreach.hpp>

#include "kernel/uconnection.hh"
#include "object/atom.hh"
#include "object/urbi-exception.hh"
#include "runner/runner.hh"

namespace runner
{

  void
  Runner::emit_result (rObject result)
  {
    context_->value_get ().new_result (result);
  }

  void
  Runner::operator() (const ast::AssignExp& e)
  {
    assert (e.lhs_get().args_get().size () == 1);
    rObject tgt = target(e.lhs_get().args_get().front());
    libport::Symbol s = e.lhs_get().name_get();
    tgt->slot_set (s, eval(e.rhs_get()));
  }

  void
  Runner::operator() (const ast::AndExp&)
  {
  }

  void
  Runner::operator() (const ast::CallExp& e)
  {
    // Iterate over arguments, with a special case for the target.
    ast::exps_type::const_iterator
      i = e.args_get().begin(),
      i_end = e.args_get().end();
    rObject tgt = target(*i);

    // Ask the target for the handler of the message.
    rObject val = tgt->lookup (e.name_get ());

    // Gather the arguments, including the target.
    object::objects_type args;
    args.push_back (tgt);
    for (++i; i != i_end; ++i)
      args.push_back (eval (**i));

    // We may have to run a primitive, or some code.
    switch (val->kind_get ())
    {
      case object::Object::kind_primitive:
        current_ = val.cast<object::Primitive>()->value_get()(args);
	break;
      case object::Object::kind_code:
	current_ = eval (val.cast<object::Code>()->value_get());
	break;
      default:
	current_ = val;
	break;
    }
  }


  void
  Runner::operator() (const ast::FloatExp& e)
  {
    current_ = new object::Float (e.value_get());
  }


  void
  Runner::operator() (const ast::ListExp& e)
  {
    // list values
    std::list<object::rObject> values;
    // Evaluate every expression in the list
    // FIXME: parallelized?
    BOOST_FOREACH (const ast::Exp* v, e.value_get())
    {
      operator() (*v);
      values.push_back(current_);
    }
    current_ = new object::List (values);
  }


  void
  Runner::operator() (const ast::Function& e)
  {
    // FIXME: Arguments.
    current_ = new object::Code (*e.body_get());
  }


  void
  Runner::operator() (const ast::NegOpExp& e)
  {
    rObject val = eval (e.operand_get ());
    assert (val->kind_get() == object::Object::kind_float);
    current_ =
      new object::Float (-1 * val.cast<object::Float>()->value_get ());
  }


  void
  Runner::operator() (const ast::SemicolonExp& e)
  {
    try
    {
      operator() (e.lhs_get());
      ECHO ("sending result of lhs");
      if (current_.get ())
        emit_result (current_);
    }
    catch (object::UrbiException& ue)
    {
      UConnection& c = context_.cast<object::Context>()->value_get();
      c.sendc (ue.what (), "error");
      c.endline ();
    }

    current_.reset ();
    assert (current_.get () == 0);
    operator() (e.rhs_get());
    ECHO ("sending result of rhs");
    if (current_.get ())
      emit_result (current_);
  }


  void
  Runner::operator() (const ast::StringExp& e)
  {
    current_ = new object::String(e.value_get());
  }

} // namespace runner
