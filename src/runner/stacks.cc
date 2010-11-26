/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/bind.hh>

// Uncomment this to get exhaustive execution debug output
// #define ENABLE_STACK_DEBUG_TRACES

#include <ast/local-assignment.hh>
#include <ast/local-declaration.hh>
#include <ast/local.hh>
#include <object/symbols.hh>
#include <runner/stacks.hh>
#include <urbi/object/slot.hh>
#include <urbi/object/slot.hxx>

namespace runner
{
  typedef Stacks::action_type action_type;
  using object::Slot;
  using object::rSlot;

  Stacks::Stacks(rObject lobby)
    : toplevel_stack_()
    , current_frame_(0, 0)
    , depth_(0)
  {
    toplevel_stack_ << new Slot(lobby) << new Slot;
    current_frame_.first = &toplevel_stack_[0];
    current_frame_.second = 0;
  }


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

  void
  Stacks::pop_context(const context_type& previous_context)
  {
    toplevel_stack_ = previous_context.toplevel_stack;
    current_frame_ = previous_context.frame;
    depth_ = previous_context.depth;
    if (depth_ == 0)
      current_frame_.first = &toplevel_stack_[0];
  }

  Stacks::frame_type
  Stacks::push_frame(libport::Symbol,
                     frame_type frame,
                     rObject self, rObject call)
  {
    // Switch frame.
    frame_type prev = current_frame_;
    current_frame_ = frame;

    // Bind 'this' and 'call'.
    this_set(self);
    call_set(call);
    ++depth_;

    // Return previous frame.
    return prev;
  }

  void
  Stacks::this_set(rObject s)
  {
    // FIXME: new slot?
    current_frame_.first[0] = new Slot(s);
  }

  void
  Stacks::call_set(rObject v)
  {
    // FIXME: new slot?
    current_frame_.first[1] = new Slot(v);
  }

  Stacks::rObject
  Stacks::this_get()
  {
    return *current_frame_.first[0];
  }

  Stacks::rObject
  Stacks::call()
  {
    return *current_frame_.first[1];
  }

  void
  Stacks::pop_frame(libport::Symbol, frame_type prev)
  {
    --depth_;
    current_frame_ = prev;
    if (!depth_)
      current_frame_.first = &toplevel_stack_[0];
  }

  void
  Stacks::set(ast::rConstLocalAssignment e, rObject v)
  {
    set(e->local_index_get(), e->depth_get(), v);
  }

  void
  Stacks::set(unsigned local, bool captured, rObject v)
  {
    if (captured)
      *current_frame_.second[local] = v;
    else
      *current_frame_.first[local + 2] = v;
  }

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
        {
          rSlot slot = new Slot();
          slot->constant_set(constant);
          toplevel_stack_ << slot;
        }
      }
      current_frame_.first = &toplevel_stack_[0];
    }

    rSlot slot = new Slot(v);
    slot->constant_set(constant);
    def(e->local_index_get() + 2, false, slot);
  }

  void
  Stacks::def_arg(ast::rConstLocalDeclaration e, rObject v)
  {
    def(e->local_index_get() + 2, v);
  }

  void
  Stacks::def_captured(ast::rConstLocalDeclaration e, rSlot v)
  {
    def(e->local_index_get(), true, v);
  }

  void
  Stacks::def(unsigned local, bool captured, rSlot v)
  {
    aver(v);
    if (captured)
      current_frame_.second[local] = v;
    else
      current_frame_.first[local] = v;
  }

  void
  Stacks::def(unsigned local, rObject v)
  {
    current_frame_.first[local] = new Slot(v);
  }

  Stacks::rSlot
  Stacks::rget(libport::Symbol, unsigned index, unsigned depth)
  {
    if (depth)
      return current_frame_.second[index];
    else
      return current_frame_.first[index + 2];
  }

  Stacks::rSlot
  Stacks::rget(ast::rConstLocal e)
  {
    return rget(e->name_get(), e->local_index_get(), e->depth_get());
  }

  Stacks::rSlot
  Stacks::rget_assignment(ast::rConstLocalAssignment e)
  {
    return rget(e->what_get(), e->local_index_get(), e->depth_get());
  }

  Stacks::rObject Stacks::get(ast::rConstLocal e)
  {
    return *rget(e);
  }

  void
  Stacks::this_switch(rObject v)
  {
    this_set(v);
  }

  void
  Stacks::this_switch_back(rObject v)
  {
    this_set(v);
  }

  void
  Stacks::execution_starts(libport::Symbol)
  {
  }
}
