/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef RUNNER_STACKS_HXX
# define RUNNER_STACKS_HXX

# include <libport/bind.hh>
# include <libport/config.h>
# include <libport/compilation.hh>

# include <ast/local-assignment.hh>
# include <ast/local-declaration.hh>
# include <ast/local.hh>
# include <urbi/object/symbols.hh>
# include <runner/stacks.hh>
# include <urbi/object/slot.hh>
# include <urbi/object/slot.hxx>

namespace runner
{
  using object::Slot;
  using object::rSlot;

  LIBPORT_SPEED_ALWAYS_INLINE
  Stacks::Stacks(rObject lobby)
    : toplevel_stack_()
    , current_frame_(0, 0)
    , depth_(0)
  {
    toplevel_stack_ << new Slot(lobby) << new Slot;
    current_frame_.first = &toplevel_stack_[0];
    current_frame_.second = 0;
  }


  LIBPORT_SPEED_ALWAYS_INLINE
  Stacks::context_type
  Stacks::push_context(rObject self)
  {
    context_type res(toplevel_stack_, current_frame_, depth_);
    toplevel_stack_.clear();
    toplevel_stack_ << new Slot(self) << new Slot;
    current_frame_.first = &toplevel_stack_[0];
    current_frame_.second = 0;
    depth_ = 0;
    return res;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::pop_context(const context_type& previous_context)
  {
    toplevel_stack_ = previous_context.toplevel_stack;
    current_frame_ = previous_context.frame;
    depth_ = previous_context.depth;
    if (depth_ == 0)
      current_frame_.first = &toplevel_stack_[0];
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  Stacks::frame_type
  Stacks::push_frame(libport::Symbol,
                     frame_type frame,
                     rObject self, rObject call)
  {
    // Switch frame.
    frame_type prev = current_frame_;
    current_frame_ = frame;

    // Bind 'this' and 'call'.
    current_frame_.first[0] = new Slot(self);
    current_frame_.first[1] = new Slot(call);
    ++depth_;

    // Return previous frame.
    return prev;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::this_set(rObject s)
  {
    *current_frame_.first[0] = s;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::call_set(rObject v)
  {
    *current_frame_.first[1] = v;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  Stacks::rObject
  Stacks::this_get()
  {
    return *current_frame_.first[0];
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  Stacks::rObject
  Stacks::call()
  {
    return *current_frame_.first[1];
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::pop_frame(libport::Symbol, frame_type prev)
  {
    --depth_;
    current_frame_ = prev;
    if (!depth_)
      current_frame_.first = &toplevel_stack_[0];
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::set(ast::rConstLocalAssignment e, rObject v)
  {
    set(e->local_index_get(), e->depth_get(), v);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::set(unsigned local, bool captured, rObject v)
  {
    if (captured)
      *current_frame_.second[local] = v;
    else
      *current_frame_.first[local + 2] = v;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::def(ast::rConstLocalDeclaration e, rObject v, bool constant)
  {
    // The toplevel's stack grows on demand.
    if (depth_ == 0)
    {
      const size_t size = e->local_index_get() + 2;
      // FIXME: We may have to grow the stacks by more than one
      // because of a binder limitation. See FIXME in Binder::bind.
      if (size >= toplevel_stack_.size())
      {
        for (unsigned i = toplevel_stack_.size(); i <= size; ++i)
          toplevel_stack_ << rSlot();
      }
      current_frame_.first = &toplevel_stack_[0];
    }

    rSlot slot = new Slot(v);
    slot->constant_set(constant);
    def(e->local_index_get() + 2, false, slot);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::def_arg(ast::rConstLocalDeclaration e, rObject v)
  {
    def(e->local_index_get() + 2, v);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::def_captured(ast::rConstLocalDeclaration e, rSlot v)
  {
    def(e->local_index_get(), true, v);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::def(unsigned local, bool captured, rSlot v)
  {
    aver(v);
    if (captured)
      current_frame_.second[local] = v;
    else
      current_frame_.first[local] = v;
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::def(unsigned local, rObject v)
  {
    def(local, false, new Slot(v));
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  Stacks::rSlot
  Stacks::rget(libport::Symbol, unsigned index, unsigned depth)
  {
    if (depth)
      return current_frame_.second[index];
    else
      return current_frame_.first[index + 2];
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  Stacks::rSlot
  Stacks::rget(ast::rConstLocal e)
  {
    return rget(e->name_get(), e->local_index_get(), e->depth_get());
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  Stacks::rSlot
  Stacks::rget_assignment(ast::rConstLocalAssignment e)
  {
    return rget(e->what_get(), e->local_index_get(), e->depth_get());
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  Stacks::rObject Stacks::get(ast::rConstLocal e)
  {
    return *rget(e);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::this_switch(rObject v)
  {
    this_set(v);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::this_switch_back(rObject v)
  {
    this_set(v);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  void
  Stacks::execution_starts(libport::Symbol)
  {
  }
}

#endif
