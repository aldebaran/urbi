/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_EVAL_FWD_HH
# define URBI_EVAL_FWD_HH

# include <urbi/object/fwd.hh>

namespace eval
{
  object::rObject
  call(runner::Job& job,
       object::rObject function,
       const object::objects_type& args = object::objects_type());

  object::rObject
  call_apply(runner::Job& job,
             object::rObject target,
             object::rObject routine,
             libport::Symbol message,
             const object::objects_type& args = object::objects_type());
}

#endif
