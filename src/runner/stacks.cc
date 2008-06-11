#include <boost/bind.hpp>

// Uncomment this to get exhaustive execution debug output
// #define ENABLE_STACK_DEBUG_TRACES

#include <ast/assignment.hh>
#include <ast/declaration.hh>
#include <ast/local.hh>
#include <runner/stacks.hh>
#include <runner/stack-debug.hh>


namespace runner
{
  Stacks::Stacks(rObject lobby)
    : local_pointer_(0)
    , closed_pointer_(0)
    , captured_pointer_(0)
  {
    // push toplevel's 'this' and 'call'
    local_stack_.push_back(lobby);
    local_stack_.push_back(0);

    STACK_ECHO("STACKS SPAWNED");
  }

  boost::function0<void>
  Stacks::push_frame(const libport::Symbol& msg,
                     unsigned local, unsigned closed, unsigned captured,
                     rObject self, rObject call)
  {
    STACK_ECHO("Call " << msg << libport::incindent);
    STACK_ECHO("Handle stacks " << libport::incindent);

    // Reserve room for 'this' and 'call'
    local += 2;

    STACK_ECHO("Frame sizes: " << libport::incindent);
    STACK_ECHO("Local    : " << local);
    STACK_ECHO("Closed   : " << closed);
    STACK_ECHO("Captured : " << captured);
    STACK_NECHO(libport::decindent);

    boost::function0<void> res =
      boost::bind(&Stacks::pop_frame, this, msg, local_pointer_,
                  closed_pointer_, captured_pointer_);

    // Adjust frame pointers
    local_pointer_ = local_stack_.size();
    captured_pointer_ = rlocal_stack_.size();
    closed_pointer_ = captured_pointer_ + captured;

    STACK_ECHO("New frame pointers: " << libport::incindent);
    STACK_ECHO("Local    : " << local_pointer_);
    STACK_ECHO("Closed   : " << closed_pointer_);
    STACK_ECHO("Captured : " << captured_pointer_);
    STACK_NECHO(libport::decindent);

    // Grow stacks
    local_stack_.resize(local_pointer_ + local);
    unsigned size = captured_pointer_ + captured + closed;
    rlocal_stack_.resize(size, 0);
    for (unsigned i = captured_pointer_; i < size; ++i)
      rlocal_stack_[i] = new rObject();

    // Bind 'this' and 'call'
    self_set(self);
    call_set(call);

    return res;
  }

  void
  Stacks::self_set(rObject v)
  {
    STACK_ECHO("Set 'this' @[" << local_pointer_ << "] = " << v.get());
    local_stack_[local_pointer_] = v;
  }

  void
  Stacks::call_set(rObject v)
  {
    STACK_ECHO("Set 'call' @[" << local_pointer_ + 1 << "] = " << v.get());
    local_stack_[local_pointer_ + 1] = v;
  }

  Stacks::rObject
  Stacks::self()
  {
    STACK_ECHO("Read 'this' @[" << local_pointer_ << "] = "
               << local_stack_[local_pointer_].get());
    return local_stack_[local_pointer_];
  }

  Stacks::rObject
  Stacks::call()
  {
    STACK_ECHO("Read 'call' @[" << local_pointer_ + 1 << "] = "
               << local_stack_[local_pointer_ + 1].get());
    return local_stack_[local_pointer_ + 1];
  }

  void
  Stacks::pop_frame(const libport::Symbol& STACK_IF_DEBUG(msg),
                    unsigned local, unsigned closed, unsigned captured)
  {
    STACK_NECHO(libport::decindent);
    STACK_ECHO("Return from " << msg);
    STACK_NECHO(libport::decindent);

    local_stack_.resize(local_pointer_, 0);
    rlocal_stack_.resize(captured_pointer_, 0);

    local_pointer_ = local;
    closed_pointer_ = closed;
    captured_pointer_ = captured;
  }

  void
  Stacks::set(ast::rConstAssignment e, rObject v)
  {
    STACK_OPEN();
    STACK_NECHO("Setting " << e->what_get()
                << " (#" << e->local_index_get() << " ");
    set(e->local_index_get(), e->closed_get(), e->depth_get(), v);
  }

  void
  Stacks::set(unsigned local, bool closed, bool captured, rObject v)
  {

#define DBG                                   \
    STACK_NECHO(") @[" << idx << "] = " << v.get() << std::endl)
    if (closed)
      if (captured)
      {
        STACK_NECHO("captured");
        unsigned idx = captured_pointer_ + local;
        DBG;
        *rlocal_stack_[idx] = v;
      }
      else
      {
        STACK_NECHO("closed");
        unsigned idx = closed_pointer_ + local;
        DBG;
        *rlocal_stack_[idx] = v;
      }
    else
    {
      STACK_NECHO("local");
      unsigned idx = local_pointer_ + local + 2;
      DBG;
      local_stack_[idx] = v;
    }
#undef DBG

  }

  void
  Stacks::def(ast::rConstDeclaration e, rObject v)
  {
    // The toplevel's stack grows on demand.
    if (local_pointer_ == 0)
    {
      // FIXME: We may have to grow the stacks by more than one
      // because of a binder limitation. See FIXME in Binder::bind.
      if (e->closed_get() && e->local_index_get() >= rlocal_stack_.size())
      {
        STACK_ECHO("Growing toplevel closed stack");
        for (unsigned i = rlocal_stack_.size();
             i <= e->local_index_get(); ++i)
          rlocal_stack_.push_back(new rObject());
      }
      else if (e->local_index_get() + 2 >= local_stack_.size())
      {
        STACK_ECHO("Growing toplevel local stack");
        for (unsigned i = local_stack_.size();
             i <= e->local_index_get() + 2; ++i)
          local_stack_.push_back(rObject());
      }
    }

    STACK_OPEN();
    STACK_NECHO("Defining " << e->what_get()
                << " (#" << e->local_index_get() << " ");
    if (e->closed_get())
      def(e->local_index_get(), false, rrObject(new rObject(v)));
    else
      def(e->local_index_get(), v);
  }

  void
  Stacks::def_arg(ast::rConstDeclaration e, rObject v)
  {
    STACK_OPEN();
    STACK_NECHO("Bind argument " << e->what_get()
                << " (#" << e->local_index_get() << " ");
    if (e->closed_get())
      def(e->local_index_get(), false, rrObject(new rObject(v)));
    else
      def(e->local_index_get(), v);
  }

  void
  Stacks::def_captured(ast::rConstDeclaration e, rrObject v)
  {
    STACK_OPEN();
    STACK_NECHO("Bind capture " << e->what_get()
                << " (#" << e->local_index_get() << " ");
    def(e->local_index_get(), true, v);
  }

  void
  Stacks::def(unsigned local, bool captured, rrObject v)
  {
#define DBG                                   \
    STACK_NECHO(") @[" << idx << "] = " << v.get() << std::endl)
    if (captured)
    {
      STACK_NECHO("captured");
      unsigned idx = captured_pointer_ + local;
      DBG;
      rlocal_stack_[idx] = v;
    }
    else
    {
      STACK_NECHO("closed");
      unsigned idx = closed_pointer_ + local;
      DBG;
      rlocal_stack_[idx] = v;
    }
#undef DBG
  }

  void
  Stacks::def(unsigned local, rObject v)
  {
    unsigned idx = local_pointer_ + local + 2;
    STACK_NECHO("local) @[" << idx << "] = " << v.get() << std::endl);
    local_stack_[idx] = v;
  }

  Stacks::rrObject Stacks::rget(ast::rConstLocal e)
  {
    rrObject res;

    assert(e->closed_get());
    STACK_OPEN();
    STACK_NECHO("Capture variable " << e->name_get()
                << " (#" << e->local_index_get() << " ");
#define DBG                                     \
    STACK_NECHO(") @[" << idx << "] = ")
    if (e->depth_get())
    {
      STACK_NECHO("captured");
      unsigned idx = captured_pointer_ + e->local_index_get();
      DBG;
      res = rlocal_stack_[idx];
    }
    else
    {
      STACK_NECHO("closed");
      unsigned idx = closed_pointer_ + e->local_index_get();
      DBG;
      res = rlocal_stack_[idx];
    }
    STACK_NECHO(res->get() << " @" << res.get() << std::endl);
    return res;
#undef DBG
  }

  Stacks::rObject Stacks::get(ast::rConstLocal e)
  {
    rObject value;
    STACK_OPEN();
    STACK_NECHO("Read variable " << e->name_get()
                << " (#" << e->local_index_get() << " ");

#define DBG                                                             \
    STACK_NECHO(") @[" << idx << "] = ")

    if (e->closed_get())
      if (e->depth_get())
      {
        unsigned idx = captured_pointer_ + e->local_index_get();
        STACK_NECHO("captured");
        DBG;
        value = *rlocal_stack_[idx];
      }
      else
      {
        unsigned idx = closed_pointer_ + e->local_index_get();
        STACK_NECHO("closed");
        DBG;
        value = *rlocal_stack_[idx];
      }
    else
    {
      assert(!e->depth_get());
      unsigned idx = local_pointer_ + e->local_index_get() + 2;
      STACK_NECHO("local");
      DBG;
      value = local_stack_[idx];
    }

#undef DBG

    STACK_NECHO(value.get() << std::endl);
    return value;
  }

  boost::function0<void>
  Stacks::switch_self(rObject v)
  {
    STACK_ECHO("Switching 'this':" << libport::incindent);
    boost::function0<void> res =
      boost::bind(&Stacks::switch_self_back, this,
                  local_stack_[local_pointer_]);
    self_set(v);
    STACK_NECHO(libport::decindent);
    return res;
  }

  void
  Stacks::switch_self_back(rObject v)
  {
    STACK_ECHO("Switching back 'this':" << libport::incindent);
    self_set(v);
    STACK_NECHO(libport::decindent);
  }

  void
  Stacks::execution_starts(const libport::Symbol& STACK_IF_DEBUG(msg))
  {
      STACK_NECHO(libport::decindent);
      STACK_ECHO("Execute " << msg << libport::incindent);
  }
}
