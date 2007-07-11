/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

#include "runner/runner.hh"
#include "object/atom.hh"

namespace runner
{

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
