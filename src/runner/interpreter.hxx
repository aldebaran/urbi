/**
 ** \file runner/interpreter.hxx
 ** \brief Inline implementation of runner::Interpreter.
 */

#ifndef RUNNER_INTERPRETER_HXX
# define RUNNER_INTERPRETER_HXX

# include "object/atom.hh"
# include "object/global-class.hh"
# include "object/alien.hh"
# include "binder/binder.hh"

namespace runner
{

  inline
  Interpreter::Interpreter (rLobby lobby,
			    rObject locals,
			    scheduler::Scheduler& sched,
			    const ast::Ast* ast,
			    bool free_ast_after_use)
    : Interpreter::super_type(),
      Runner(lobby, sched),
      ast_(ast),
      free_ast_after_use_(free_ast_after_use),
      code_(0),
      current_(0),
      locals_(locals)
  {
    init();
  }

  inline
  Interpreter::Interpreter(const Interpreter& model, rObject code)
    : Interpreter::super_type(),
      Runner(model),
      ast_(0),
      free_ast_after_use_(false),
      code_(code),
      current_(0),
      locals_(model.locals_)
  {
    init();
  }

  inline
  Interpreter::Interpreter(const Interpreter& model,
			   const ast::Ast* ast,
			   bool free_ast_after_use)
    : Interpreter::super_type (),
      Runner(model),
      ast_(ast),
      free_ast_after_use_(free_ast_after_use),
      code_(0),
      current_(0),
      locals_(model.locals_)
  {
  }

  inline
  Interpreter::~Interpreter ()
  {
  }

  inline
  void Interpreter::init()
  {
    if (!locals_)
      locals_ = object::Object::make_method_scope(lobby_);
    // If the lobby has a slot connectionTag, push it
    rObject connection_tag = lobby_->slot_locate(SYMBOL(connectionTag));
    if (connection_tag)
      push_tag(extract_tag(connection_tag->slot_get(SYMBOL(connectionTag))));
  }

  inline
  const object::rObject&
  Interpreter::locals_get () const
  {
    return locals_;
  }

  inline
  Interpreter::rObject
  Interpreter::eval (const ast::Ast& e)
  {
    ECHO("Eval: " << &e << " {{{" << e << "}}}");
    operator()(e);
    ECHO("Eval: " << &e << " = " << current_);
    return current_;
  }

} // namespace runner

#endif // RUNNER_INTERPRETER_HXX
