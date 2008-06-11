/**
 ** \file runner/stacks.hh
 ** \brief Definition of runner::Stacks.
 */

#ifndef RUNNER_STACKS_HH
# define RUNNER_STACKS_HH

# include <ast/fwd.hh>
# include <object/object.hh>

 // FIXME: action

namespace runner
{
  class Stacks
  {
    public:
      // Import types
      typedef object:: rObject  rObject;
      typedef object::rrObject rrObject;

      /// Build static stacks
      Stacks(rObject lobby);

      /// Set values in the stacks
      void set(ast::rConstAssignment e, rObject v);
      void def(ast::rConstDeclaration e, rObject v);
      void def_arg(ast::rConstDeclaration e, rObject v);
      void def_captured(ast::rConstDeclaration e, rrObject v);

      /// Get values from the stacks
      rObject get(ast::rConstLocal e);
      rrObject rget(ast::rConstLocal e);

      /// Get and set 'this' and 'call'
      void self_set(rObject v);
      void call_set(rObject v);
      rObject self();
      rObject call();
      boost::function0<void> switch_self(rObject v);

      /// Signal the stacks a new execution is starting
      void execution_starts(const libport::Symbol& msg);

      /// Push new frames on the stacks to execute a function
      /** \param msg      Name of the function being invoked (debug purpose only)
       *  \param local    Size of the local    variable frame.
       *  \param closed   Size of the closed   variable frame.
       *  \param captured Size of the captured variable frame.
       *  \param self     Value of 'this' in the new frame.
       *  \param call     Value of 'call' in the new frame.
       *  \return The action to restore the previous frame state.
       */
      boost::function0<void>
      push_frame(const libport::Symbol& msg,
                 unsigned local, unsigned closed, unsigned captured,
                 rObject self, rObject call);

    private:

      /// Factored setter
      void set(unsigned local, bool closed, bool captured, rObject v);
      /// Factored definer
      void def(unsigned local, rObject v);
      void def(unsigned local, bool captured, rrObject v);

      /// Helper to restore a previous frame state
      void pop_frame(const libport::Symbol& msg,
                     unsigned local, unsigned closed, unsigned captured);
      /// Helper to restore switched 'this'
      void switch_self_back(rObject v);

      /// The local stack
      typedef std::vector<rObject> local_stack_type;
      local_stack_type local_stack_;
      /// The related frame pointer
      unsigned local_pointer_;

      /// The double-indirection local stack, for captured and
      /// closed-over variables
      typedef std::vector<rrObject> rlocal_stack_type;
      rlocal_stack_type rlocal_stack_;
      /// The closed variables frame pointer
      unsigned closed_pointer_;
      /// The captured variables frame pointer
      unsigned captured_pointer_;
  };
}

#endif
