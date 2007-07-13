/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include <boost/foreach.hpp>
#include "runner/runner.hh"
#include "object/atom.hh"

namespace runner
{

  void
  Runner::operator() (const ast::CallExp& e)
  {
    // Gather the arguments...
    object::objects_type args;
    // ... including the target if there is one.
    if (e.target_get())
      args.push_back (eval (*e.target_get()));
    BOOST_FOREACH(ast::Exp* a, e.args_get())
      args.push_back (eval (*a));
    // Ask the target for the handler of the message.
    rObject prim = args[0]->lookup (e.name_get ());
    
    // It'd better be a primitive, for the time being.
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
