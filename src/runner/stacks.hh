/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/stacks.hh
 ** \brief Definition of runner::Stacks.
 */

#ifndef RUNNER_STACKS_HH
# define RUNNER_STACKS_HH

# include <boost/function.hpp>
#include <boost/utility.hpp>

# include <ast/fwd.hh>
# include <urbi/object/object.hh>

namespace runner
{
  class Stacks: public boost::noncopyable
  {
  public:
    // Import types.
    typedef object::rObject rObject;
    typedef object::rSlot   rSlot;

    /// Type of a stack frame.
    typedef std::pair<rSlot*, rSlot*> frame_type;

    /// Type of the toplevel variable stack.
    typedef std::vector<rSlot> toplevel_stack_type;

    /// Type of a context.
    struct context_type
    {
      context_type(const toplevel_stack_type& s, frame_type f, unsigned d)
        : toplevel_stack(s)
        , frame(f)
        , depth(d)
      {}
      toplevel_stack_type toplevel_stack;
      frame_type frame;
      unsigned depth;
    };

  /*---------------.
  | Construction.  |
  `---------------*/

  public:
    /// Build static stacks.
    Stacks(rObject lobby);

  /*--------------------------.
  | Starting / ending calls.  |
  `--------------------------*/

  public:
    /// Signal the stacks a new execution is starting
    void execution_starts(libport::Symbol msg);

    /// Push new frames on the stacks to execute a function
    /** \param msg      Name of the function being invoked (debug purpose only)
     *  \param local    Size of the local    variable frame.
     *  \param closed   Size of the closed   variable frame.
     *  \param captured Size of the captured variable frame.
     *  \param self     Value of 'this' in the new frame.
     *  \param call     Value of 'call' in the new frame.
     */
    frame_type
    push_frame(libport::Symbol msg,
               frame_type local_frame,
               rObject self, rObject call);

    /// Helper to restore a previous frame state
    void pop_frame(libport::Symbol msg, frame_type previous_frame);


    /// Push a whole new context. Returns a token for pop_context.
    context_type push_context(rObject self);
    /// Pop the context.
    void pop_context(const context_type& previous_context);

  /*-----------------.
  | Reading values.  |
  `-----------------*/

  public:
    /// Get value from the stack.
    rObject get(ast::rConstLocal e);
    /// Get slot from the stack.
    rSlot rget(ast::rConstLocal e);
    /// Get slot from the stack.
    rSlot
    rget_assignment(ast::rConstLocalAssignment e);
    /// Get 'this'.
    rObject this_get();
    /// Get 'call'.
    rObject call();

  private:
    /// Factored helpers for both rget.
    Stacks::rSlot
    rget(libport::Symbol name, unsigned index, unsigned depth);

  /*-----------------.
  | Setting values.  |
  `-----------------*/

  public:
    /// Set 'this'.
    void this_set(rObject s);
    /// Set 'call'.
    void call_set(rObject v);
    /// Update the given value.
    void set(ast::rConstLocalAssignment e, rObject v);
    /// Define the given value.
    void def(ast::rConstLocalDeclaration e, rObject v,
             bool constant = false);
    /// Bind given argument.
    void def_arg(ast::rConstLocalDeclaration e, rObject v);
    /// Bind given captured variable.
    void def_captured(ast::rConstLocalDeclaration e, rSlot v);

    /// Switch the current 'this'.
    void this_switch(rObject s);

  private:
    /// Factored setter.
    void set(unsigned local, bool captured, rObject v);
    /// Factored definer.
    void def(unsigned local, rObject v);
    void def(unsigned local, bool captured, rSlot v);
    /// Restore switched 'this'.
    void this_switch_back(rObject v);

  /*----------.
  | Details.  |
  `----------*/

  private:
    /// The double-indirection local stack, for captured and
    /// closed-over variables
    toplevel_stack_type toplevel_stack_;
    frame_type current_frame_;
    unsigned depth_;
  };
}

# if defined LIBPORT_COMPILATION_MODE_SPEED
#  include <runner/stacks.hxx>
# endif

#endif
