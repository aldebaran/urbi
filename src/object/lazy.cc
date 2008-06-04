#include "ast/function.hh"
#include "ast/scope.hh"
#include "global-class.hh"
#include "runner/interpreter.hh"
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
  mkLazy(runner::Runner& r, ast::rConstExp e)
  {
    // Strangely, this temporary variable is required. Calling the
    // ast::Function ctor inline in the make_code(...) call invokes
    // the ast copy ctor. Please post an explanation if you
    // understand the problem.
    ast::rClosure ast =
      new ast::Closure(e->location_get(), new ast::declarations_type(),
                       new ast::Scope(e->location_get(), const_cast<ast::Exp*>(e.get())));
    rObject function = dynamic_cast<runner::Interpreter*>(&r)->make_code(ast);
    return mkLazy(r, function);
  }
}
