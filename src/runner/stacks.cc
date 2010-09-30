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
#include <urbi/object/slot.hh>
#include <urbi/object/slot.hxx>
#include <runner/stacks.hh>

namespace runner
{
  typedef Stacks::action_type action_type;
  using object::Slot;
  using object::rSlot;

  Stacks::Stacks(rObject lobby)
    : local_pointer_(0)
    , captured_pointer_(0)
  {
    // Push toplevel's 'this' and 'call'.
    local_stack_ << new Slot(lobby) << new Slot();
  }

  void
  Stacks::push_frame(libport::Symbol,
                     unsigned local, unsigned captured,
                     rObject self, rObject call)
  {
    // Reserve room for 'this' and 'call'.
    local += 2;

    // Adjust frame pointers.
    local_pointer_ = local_stack_.size();
    captured_pointer_ = captured_stack_.size();

    // Grow stacks.
    local_stack_.resize(local_pointer_ + local);
    unsigned size = captured_pointer_ + captured;
    captured_stack_.resize(size, rSlot());
    for (unsigned i = captured_pointer_; i < size; ++i)
      captured_stack_[i] = new Slot();

    // Bind 'this' and 'call'.
    this_set(self);
    call_set(call);
  }

  void
  Stacks::this_set(rObject s)
  {
    local_stack_[local_pointer_] = new Slot(s);
  }

  void
  Stacks::call_set(rObject v)
  {
    local_stack_[local_pointer_ + 1] = new Slot(v);
  }

  Stacks::rObject
  Stacks::this_get()
  {
    return *local_stack_[local_pointer_];
  }

  Stacks::rObject
  Stacks::call()
  {
    return *local_stack_[local_pointer_ + 1];
  }

  void
  Stacks::pop_frame(libport::Symbol,
                    unsigned local, unsigned captured)
  {
    local_stack_.resize(local_pointer_, rSlot());
    captured_stack_.resize(captured_pointer_, rSlot());

    local_pointer_ = local;
    captured_pointer_ = captured;
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
    {
      unsigned idx = captured_pointer_ + local;
      *captured_stack_[idx] = v;
    }
    else
    {
      unsigned idx = local_pointer_ + local + 2;
      *local_stack_[idx] = v;
    }
  }

  void
  Stacks::def(ast::rConstLocalDeclaration e, rObject v, bool constant)
  {
    // The toplevel's stack grows on demand.
    if (local_pointer_ == 0)
    {
      // FIXME: We may have to grow the stacks by more than one
      // because of a binder limitation. See FIXME in Binder::bind.
      if (e->local_index_get() + 2 >= local_stack_.size())
      {
        for (unsigned i = local_stack_.size();
             i <= e->local_index_get() + 2; ++i)
        {
          rSlot slot = new Slot();
          slot->constant_set(constant);
          local_stack_ << slot;
        }
      }
    }

    rSlot slot = new Slot(v);
    slot->constant_set(constant);
    def(e->local_index_get() + 2, false, slot);
  }

  void
  Stacks::def_arg(ast::rConstLocalDeclaration e, rObject v)
  {
    def(e->local_index_get(), v);
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
    {
      unsigned idx = captured_pointer_ + local;
      captured_stack_[idx] = v;
    }
    else
    {
      unsigned idx = local_pointer_ + local;
      local_stack_[idx] = v;
    }
  }

  void
  Stacks::def(unsigned local, rObject v)
  {
    unsigned idx = local_pointer_ + local + 2;
    local_stack_[idx] = new Slot(v);
  }

  Stacks::rSlot
  Stacks::rget(libport::Symbol name, unsigned index, unsigned depth)
  {
    (void)name;
    rSlot res;

    if (depth)
    {
      unsigned idx = captured_pointer_ + index;
      res = captured_stack_[idx];
    }
    else
    {
      unsigned idx = local_pointer_ + 2 + index;
      res = local_stack_[idx];
    }
    return res;
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
