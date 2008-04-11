#ifndef OBJECT_LAZY_HH
# define OBJECT_LAZY_HH

namespace object
{
  inline
  rObject
  mkLazy(runner::Runner& r, rObject content)
  {
    rObject res = object::global_class->slot_get(SYMBOL(Lazy))->clone();
    res->slot_update(r, SYMBOL(code), content);
    return res;
  }

  inline
  rObject
  mkLazy(runner::Runner& r, ast::Exp* e)
  {
    // Strangly, this temporary variable is required. Calling the
    // ast::Function ctor inline in the make_code(...) call invokes
    // the ast copy ctor. Please post an explanation if you
    // understand the problem.
    ast::Function ast(e->location_get(), new ast::symbols_type(),
		      new ast::Scope(e->location_get(), 0, ast::clone(*e)));
    rObject function = r.make_code(ast);
    urbi_call(r, function, SYMBOL(makeClosure));
    return mkLazy(r, function);
  }
}

#endif
