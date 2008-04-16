#ifndef OBJECT_LAZY_HH
# define OBJECT_LAZY_HH

# include "ast/fwd.hh"
# include "object.hh"
# include "runner/runner.hh"

namespace object
{
  /// Build a lazy value with the given value
  rObject
  mkLazy(runner::Runner& r, rObject content);

  /// Build a lazy value that evaluates to the given expression
  rObject
  mkLazy(runner::Runner& r, ast::Exp* e);
}

#endif
