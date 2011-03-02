/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/eval/exec.hxx
 ** \brief Definition of eval::exec.
 */

#ifndef EVAL_EXEC_HXX
# define EVAL_EXEC_HXX

# include <eval/action.hh>
# include <urbi/object/executable.hh>

namespace eval
{

  inline
  Action  exec(object::rExecutable e,
               const object::objects_type& args)
  {
    typedef rObject (*fun_t)(UrbiJob& job,
                             object::rExecutable e,
                             const object::objects_type& args);
    return boost::bind((fun_t) &exec, _1, e, args);
  }

  inline
  rObject exec(UrbiJob& job,
               object::rExecutable e,
               const object::objects_type& args)
  {
    job.state.this_set(e);
    return (*e)(args);
  }

  inline
  Action  exec(boost::function0<void> e,
               rObject self)
  {
    typedef rObject (*fun_t)(UrbiJob& job,
                             boost::function0<void> e,
                             rObject self);
    return boost::bind((fun_t) &exec, _1, e, self);
  }

  inline
  rObject exec(UrbiJob& job,
               boost::function0<void> e,
               rObject self)
  {
    job.state.this_set(self);
    e();
    return object::void_class;
  }


} // namespace eval

#endif // ! EVAL_EXEC_HXX
