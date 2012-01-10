/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/eval/action.hh
 ** \brief Definition of eval::Action.
 */

#ifndef EVAL_ACTION_HH
# define EVAL_ACTION_HH

# include <libport/bind.hh>
# include <boost/function.hpp>

# include <urbi/object/fwd.hh>
# include <urbi/runner/fwd.hh>

namespace eval
{
  using object::rObject;
  using runner::Job;

  typedef boost::function1<rObject, Job&> Action;

  // Attempt to avoid boost function and to use classes.
# if 0
  struct Action
  {
    virtual rObject work(Job* job) = 0;
  };

  struct Call : public Action
  {
    /// Create a new class to hold arguments for the execution.
    static
    Action* build(rObject code, const objects_type& args)
    { return new Call(code, args); }

    virtual rObject work(Job* job) { return exec(job); }

    // Implement this method if you don't need extra persistent context
    // during the traversal of the tree.
    rObject exec(Job* job)
    { return exec(job, code_, args_); }

    // Implement this method if you need to keep context during the
    // traversal of the tree.
    static inline
    rObject exec(Job* job,
                 rObject code, const objects_type& args)
    {
      std::auto_ptr<Call> v(reinterpret_cast<Call*>(build(code, args)));
      return v->exec(job);
    }

    // Hold the value before the evaluation of this action.
  private:
    Call(rObject code, const objects_type& args)
      : code_(code), args_(args)
    {}

    rObject code_;
    const objects_type& args_;
  };
# endif

} // namespace eval

#endif // ! EVAL_ACTION_HH
