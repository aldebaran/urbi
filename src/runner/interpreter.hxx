/**
 ** \file runner/interpreter.hxx
 ** \brief Inline implementation of runner::Interpreter.
 */

#ifndef RUNNER_INTERPRETER_HXX
# define RUNNER_INTERPRETER_HXX

#include <object/tag.hh>

namespace runner
{

  inline
  Interpreter::rObject
  Interpreter::result_get()
  {
    return result_;
  }

  inline
  libport::Symbol
  Interpreter::innermost_call_get() const
  {
    assert(!call_stack_.empty());
    return call_stack_.back().first;
  }

  inline void
  Interpreter::tag_stack_clear()
  {
    tag_stack_.clear();
  }

  inline const tag_stack_type&
  Interpreter::tag_stack_get() const
  {
    return tag_stack_;
  }

  inline void
  Interpreter::tag_stack_set(const tag_stack_type& tag_stack)
  {
    tag_stack_ = tag_stack;
  }

  /*----------------.
  | Regular visit.  |
  `----------------*/

  inline object::rObject
  Interpreter::operator() (const ast::Ast* e)
  {
    libport::Finally finally(scoped_set(innermost_node_, e));
    return e->eval(*this);
  }

} // namespace runner

#endif // RUNNER_INTERPRETER_HXX
