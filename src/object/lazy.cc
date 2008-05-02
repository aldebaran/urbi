#include "ast/function.hh"
#include "ast/new-clone.hh"
#include "ast/scope.hh"

#include "global-class.hh"
#include "lazy.hh"

namespace object
{
  rObject
  mkLazy(runner::Runner& r, rObject content)
  {
    rObject res = object::global_class->slot_get(SYMBOL(Lazy))->clone();
    res->slot_update(r, SYMBOL(code), content);
    return res;
  }

  rObject
  mkLazy(runner::Runner& r, const ast::Exp& e)
  {
    // Strangely, this temporary variable is required. Calling the
    // ast::Function ctor inline in the make_code(...) call invokes
    // the ast copy ctor. Please post an explanation if you
    // understand the problem.
    ast::Function ast(e.location_get(), new ast::symbols_type(),
		      new ast::Scope(e.location_get(), 0, new_clone(e)));
    rObject function = r.make_code(ast);
    urbi_call(r, function, SYMBOL(makeClosure));
    return mkLazy(r, function);
  }
}
