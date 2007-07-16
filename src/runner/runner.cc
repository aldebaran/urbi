/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include <boost/foreach.hpp>

#include "object/atom.hh"
#include "object/primitives.hh"

#include "runner/runner.hh"

namespace runner
{

  void
  Runner::operator() (const ast::CallExp& e)
  {
    // FIXME: For the time being, if there is no target, it is the
    // Connection object which is used, sort of a Lobby for IO.
    rObject tgt = 0;
    if (e.target_get())
      tgt = eval (*e.target_get());
    else
      tgt = object::connection_class;

    // Ask the target for the handler of the message.
    // It'd better be a primitive, for the time being.
    rObject prim = tgt->lookup (e.name_get ());

    // Gather the arguments, including the target.
    object::objects_type args;
    args.push_back (tgt);
    BOOST_FOREACH(ast::Exp* a, e.args_get())
      args.push_back (eval (*a));

    // Evaluate the call.  A primitive hopefully.
    current_ = prim.cast<object::Primitive>()->value_get()(args);
  }

  void
  Runner::operator() (const ast::FloatExp& e)
  {
    current_ = new object::Float (e.value_get());
  }

  void
  Runner::operator() (const ast::SemicolonExp& e)
  {
    operator() (e.lhs_get());
    operator() (e.rhs_get());
  }

} // namespace runner
