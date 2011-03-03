/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef RUNNER_STATE_HXX
# define RUNNER_STATE_HXX

# include <libport/bind.hh>
# include <libport/config.h>
# include <libport/compilation.hh>

# include <ast/local-assignment.hh>
# include <ast/local-declaration.hh>
# include <ast/local.hh>
# include <object/symbols.hh>
# include <runner/stacks.hh>
# include <urbi/object/slot.hh>
# include <urbi/object/slot.hxx>

namespace runner
{

  /// Handle tags.
  /// \{

  LIBPORT_SPEED_INLINE void
  State::update_priority_cache(sched::prio_type prio)
  {
    // FIXME: Previously, this code was refering to
    // scheduler_get().real_time_behavior_get() to avoid the computation of
    // the priority, but with a cache, there is no need to check for it
    // because the execution time is bounded and small.  But this is still
    // not real-time.  To make it real-time, the job priority should be
    // stored with the corresponding tag.  Thus the priority would be
    // computed incrementally.
    if (prio >= priority_cache_)
      priority_cache_valid_ = false;
    if (tag_stack_.empty())
      priority_cache_ = priority();
  }

  LIBPORT_SPEED_INLINE void
  State::apply_tag(const object::rTag& tag, libport::Finally* finally)
  {
    tag_stack_push(tag);
    if (finally)
      *finally << boost::bind(&State::tag_stack_pop, this);
  }

  LIBPORT_SPEED_ALWAYS_INLINE void
  State::tag_stack_clear()
  {
    tag_stack_.clear();
  }

  LIBPORT_SPEED_ALWAYS_INLINE const State::tag_stack_type&
  State::tag_stack_get_all() const
  {
    return tag_stack_;
  }

  LIBPORT_SPEED_ALWAYS_INLINE void
  State::tag_stack_set(const tag_stack_type& tag_stack)
  {
    tag_stack_ = tag_stack;
  }

  LIBPORT_SPEED_ALWAYS_INLINE size_t
  State::tag_stack_size() const
  {
    return tag_stack_.size();
  }

  LIBPORT_SPEED_INLINE void
  State::tag_stack_push(const object::rTag& tag)
  {
    tag_stack_.push_back(tag);
    update_priority_cache(tag->value_get()->prio_get());
  }

  LIBPORT_SPEED_INLINE void
  State::tag_stack_pop()
  {
    const object::rTag& tag = tag_stack_.back();
    sched::prio_type prio = tag->value_get()->prio_get();
    tag_stack_.pop_back();
    update_priority_cache(prio);
  }

  /// \}


  /// Scope tag
  /// \{

  // Scope tag are likely to be removed by static analysis.

  LIBPORT_SPEED_INLINE
  const sched::rTag&
  State::scope_tag(sched::Scheduler& sched)
  {
    sched::rTag& tag = scope_tags_.back();
    if (!tag)
    {
      // Create the tag on demand. It must have the lowest possible priority
      // to avoid influencing the scheduling algorithm.
      tag = new sched::Tag(SYMBOL(scopeTag));
      tag->prio_set(sched, sched::UPRIO_NONE);
    }
    return tag;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  State::create_scope_tag()
  {
    // Push a scopeTag into existence, but do not allocate it until it is
    // used with the scope_tag function.
    scope_tags_.push_back(0);
  }

  LIBPORT_SPEED_INLINE
  void
  State::cleanup_scope_tag(sched::Scheduler& sched)
  {
    const sched::rTag& tag = scope_tags_.back();
    if (tag)
      tag->stop(sched, object::void_class);
    scope_tags_.pop_back();
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  const sched::rTag&
  State::scope_tag_get() const
  {
    return scope_tags_.back();
  }

  /// \}


  /// Call Stack
  /// \{

  LIBPORT_SPEED_ALWAYS_INLINE
  const State::call_stack_type&
  State::call_stack_get() const
  {
    return call_stack_;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  State::call_stack_type&
  State::call_stack_get()
  {
    return call_stack_;
  }

  LIBPORT_SPEED_INLINE
  libport::Symbol
  State::innermost_call_get() const
  {
    if (call_stack_.empty())
      return SYMBOL(LT_empty_GT);
    else
      return call_stack_.back().first;
  }

  /// \}


  /// Frame stack
  /// \{

  // This is a duplication of the interface of Stacks, but this is
  // temporary and should be modified as soon as the State can be
  // separated in multiple independent and/or inter-dependent components
  // to handle the state of a job.

  // This wrapper may cause a source of performance lost due to ref-counted
  // arguments on which the inlining keep the original semantic of
  // incrementing multiple times the counter.

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::execution_starts(libport::Symbol msg)
  {
    stacks_.execution_starts(msg);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  State::var_frame_type
  State::push_frame(libport::Symbol msg,
                        var_frame_type local_frame,
                        rObject self, rObject call)
  {
    return stacks_.push_frame(msg, local_frame, self, call);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::pop_frame(libport::Symbol msg,
                            var_frame_type previous_frame)
  {
    stacks_.pop_frame(msg, previous_frame);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  State::var_context_type
  State::push_context(rObject self)
  {
    return stacks_.push_context(self);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::pop_context(const var_context_type& previous_context)
  {
    stacks_.pop_context(previous_context);
  }

  /*-----------------.
  | Reading values.  |
  `-----------------*/

  LIBPORT_SPEED_ALWAYS_INLINE
  State::rObject
  State::get(ast::rConstLocal e)
  {
    return stacks_.get(e);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  State::rSlot
  State::rget(ast::rConstLocal e)
  {
    return stacks_.rget(e);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  State::rSlot
  State::rget_assignment(ast::rConstLocalAssignment e)
  {
    return stacks_.rget_assignment(e);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  State::rObject
  State::this_get()
  {
    return stacks_.this_get();
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  State::rObject
  State::call()
  {
    return stacks_.call();
  }

  /*-----------------.
  | Setting values.  |
  `-----------------*/

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::this_set(rObject s)
  {
    stacks_.this_set(s);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::call_set(rObject v)
  {
    stacks_.call_set(v);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::set(ast::rConstLocalAssignment e, rObject v)
  {
    stacks_.set(e, v);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::def(ast::rConstLocalDeclaration e, rObject v,
                      bool constant)
  {
    stacks_.def(e, v, constant);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::def_arg(ast::rConstLocalDeclaration e, rObject v)
  {
    stacks_.def_arg(e, v);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::def_captured(ast::rConstLocalDeclaration e, rSlot v)
  {
    stacks_.def_captured(e, v);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::this_switch(rObject s)
  {
    stacks_.this_switch(s);
  }

  /// \}

  /// Lobby
  /// \{

  LIBPORT_SPEED_ALWAYS_INLINE
  State::Lobby* State::lobby_get()
  {
    return lobby_.get();
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::lobby_set(Lobby* lobby)
  {
    lobby_ = lobby;
  }

  /// \}

  /// \ name Last location
  /// \{

  LIBPORT_SPEED_ALWAYS_INLINE
  void State::innermost_node_set(const ast::Ast* n)
  {
    innermost_node_ = n;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  const ast::Ast* State::innermost_node_get() const
  {
    return innermost_node_;
  }

  /// \}


} // namespace runner

#endif // ! RUNNER_STATE_HXX
