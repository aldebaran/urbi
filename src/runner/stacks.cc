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
#include <runner/stack-debug.hh>


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
    local_stack_.push_back(new Slot(lobby));
    local_stack_.push_back(new Slot());
    STACK_ECHO("STACKS SPAWNED");
  }

  void
  Stacks::push_frame(const libport::Symbol&,
                     unsigned local, unsigned captured,
                     rObject self, rObject call)
  {
    STACK_ECHO("Call " << msg << libport::incindent);
    STACK_ECHO("Handle stacks " << libport::incindent);

    // Reserve room for 'this' and 'call'.
    local += 2;

    STACK_ECHO("Frame sizes: " << libport::incindent);
    STACK_ECHO("Local    : " << local);
    STACK_ECHO("Captured : " << captured);
    STACK_NECHO(libport::decindent);

    // Adjust frame pointers.
    local_pointer_ = local_stack_.size();
    captured_pointer_ = captured_stack_.size();

    STACK_ECHO("New frame pointers: " << libport::incindent);
    STACK_ECHO("Local    : " << local_pointer_);
    STACK_ECHO("Captured : " << captured_pointer_);
    STACK_NECHO(libport::decindent);

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
    STACK_ECHO("Set 'this' @[" << local_pointer_ << "] = " << s.get());
    local_stack_[local_pointer_] = new Slot(s);
  }

  void
  Stacks::call_set(rObject v)
  {
    STACK_ECHO("Set 'call' @[" << local_pointer_ + 1 << "] = " << v.get());
    local_stack_[local_pointer_ + 1] = new Slot(v);
  }

  Stacks::rObject
  Stacks::this_get()
  {
    STACK_ECHO("Read 'this' @[" << local_pointer_ << "] = "
               << local_stack_[local_pointer_].get());
    return *local_stack_[local_pointer_];
  }

  Stacks::rObject
  Stacks::call()
  {
    STACK_ECHO("Read 'call' @[" << local_pointer_ + 1 << "] = "
               << local_stack_[local_pointer_ + 1].get());
    return *local_stack_[local_pointer_ + 1];
  }

  void
  Stacks::pop_frame(const libport::Symbol& STACK_IF_DEBUG(msg),
                    unsigned local, unsigned captured)
  {
    STACK_NECHO(libport::decindent);
    STACK_ECHO("Return from " << msg);
    STACK_NECHO(libport::decindent);

    local_stack_.resize(local_pointer_, rSlot());
    captured_stack_.resize(captured_pointer_, rSlot());

    local_pointer_ = local;
    captured_pointer_ = captured;
  }

  void
  Stacks::set(ast::rConstLocalAssignment e, rObject v)
  {
    STACK_OPEN();
    STACK_NECHO("Setting " << e->what_get()
                << " (#" << e->local_index_get() << " ");
    set(e->local_index_get(), e->depth_get(), v);
  }

  void
  Stacks::set(unsigned local, bool captured, rObject v)
  {

#define DBG                                   \
    STACK_NECHO(") @[" << idx << "] = " << v.get() << std::endl)
    if (captured)
    {
      STACK_NECHO("captured");
      unsigned idx = captured_pointer_ + local;
      DBG;
      *captured_stack_[idx] = v;
    }
    else
    {
      STACK_NECHO("local");
      unsigned idx = local_pointer_ + local + 2;
      DBG;
      *local_stack_[idx] = v;
    }
#undef DBG

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
          STACK_ECHO("Growing toplevel local stack");
          rSlot slot = new Slot();
          slot->constant_set(constant);
          local_stack_.push_back(slot);
        }
      }
    }

    STACK_OPEN();
    STACK_NECHO("Defining " << e->what_get()
                << " (#" << e->local_index_get() << " ");
    rSlot slot = new Slot(v);
    slot->constant_set(constant);
    def(e->local_index_get() + 2, false, slot);
  }

  void
  Stacks::def_arg(ast::rConstLocalDeclaration e, rObject v)
  {
    STACK_OPEN();
    STACK_NECHO("Bind argument " << e->what_get()
                << " (#" << e->local_index_get() << " ");
    def(e->local_index_get(), v);
  }

  void
  Stacks::def_captured(ast::rConstLocalDeclaration e, rSlot v)
  {
    STACK_OPEN();
    STACK_NECHO("Bind capture " << e->what_get()
                << " (#" << e->local_index_get() << " ");
    def(e->local_index_get(), true, v);
  }

  void
  Stacks::def(unsigned local, bool captured, rSlot v)
  {
    aver(v);
#define DBG                                                     \
    STACK_NECHO(") @[" << idx << "] = " << " @" << v.get() << std::endl)
    if (captured)
    {
      STACK_NECHO("captured");
      unsigned idx = captured_pointer_ + local;
      DBG;
      captured_stack_[idx] = v;
    }
    else
    {
      STACK_NECHO("local");
      unsigned idx = local_pointer_ + local;
      DBG;
      local_stack_[idx] = v;
    }
#undef DBG
  }

  void
  Stacks::def(unsigned local, rObject v)
  {
    unsigned idx = local_pointer_ + local + 2;
    STACK_NECHO("local) @[" << idx << "] = " << v.get() << std::endl);
    local_stack_[idx] = new Slot(v);
  }

  Stacks::rSlot
  Stacks::rget(libport::Symbol name, unsigned index, unsigned depth)
  {
    (void)name;
    rSlot res;

    STACK_OPEN();
    STACK_NECHO("Get variable " << name
                << " (#" << index << " ");
#define DBG                                     \
    STACK_NECHO(") @[" << idx << "] = ")
    if (depth)
    {
      STACK_NECHO("captured");
      unsigned idx = captured_pointer_ + index;
      DBG;
      res = captured_stack_[idx];
    }
    else
    {
      STACK_NECHO("local");
      unsigned idx = local_pointer_ + 2 + index;
      DBG;
      res = local_stack_[idx];
    }
    STACK_NECHO(res->value().get() << " @" << res.get() << std::endl);
    return res;
#undef DBG
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
    STACK_ECHO("Switching 'this':" << libport::incindent);
    this_set(v);
    STACK_NECHO(libport::decindent);
  }

  void
  Stacks::this_switch_back(rObject v)
  {
    STACK_ECHO("Switching back 'this':" << libport::incindent);
    this_set(v);
    STACK_NECHO(libport::decindent);
  }

  void
  Stacks::execution_starts(const libport::Symbol& STACK_IF_DEBUG(msg))
  {
    STACK_NECHO(libport::decindent);
    STACK_ECHO("Execute " << msg << libport::incindent);
  }
}
