/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include <boost/foreach.hpp>

#include "object/atom.hh"

#include "runner/runner.hh"

namespace runner
{

  void
  Runner::operator() (const ast::AssignExp& e)
  {
    assert (e.lhs_get().args_get().size () == 1);
    rObject tgt = target(e.lhs_get().args_get().front());
    libport::Symbol s = e.lhs_get().name_get();
    tgt->slot_set (s, eval(e.rhs_get()));
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

    // We may have to run a primitive.
    if (val->kind_get () == object::Object::kind_primitive)
      current_ = val.cast<object::Primitive>()->value_get()(args);
    else
      current_ = val;
  }

  void
  Runner::operator() (const ast::FloatExp& e)
  {
    current_ = new object::Float (e.value_get());
  }


  void
  Runner::operator() (const ast::Function& e)
  {
    // FIXME: Arguments.
    current_ = new object::Code (e.body_get());
  }



  void
  Runner::operator() (const ast::SemicolonExp& e)
  {
    operator() (e.lhs_get());
    operator() (e.rhs_get());
  }

} // namespace runner
