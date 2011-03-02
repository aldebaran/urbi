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
 ** \file runner/eval/call.hh
 ** \brief Definition of eval::call.
 */

#ifndef EVAL_CALL_HH
# define EVAL_CALL_HH

# include <eval/action.hh>

# include <ast/fwd.hh>
# include <ast/loc.hh>
# include <ast/exps-type.hh>

namespace eval
{

  Action  call(object::rObject function,
               const object::objects_type& args = object::objects_type());

  rObject call(UrbiJob& job,
               object::rObject function,
               const object::objects_type& args = object::objects_type());

  /*
  Action  call(rObject this_,
               libport::Symbol msg,
               rObject code,
               const object::objects_type& args,
               boost::optional< ::ast::loc> loc);

  rObject call(UrbiJob& job,
               rObject target,
               libport::Symbol msg,
               rObject code,
               const object::objects_type& args,
               boost::optional< ::ast::loc> loc);
  */

  /*-----------------------------------------------------------------.
  | Apply with evaluated arguments, and potentially a call message.  |
  `-----------------------------------------------------------------*/

  Action  call_apply(object::rObject target,
                     object::rObject routine,
                     libport::Symbol message,
                     const object::objects_type& args = object::objects_type());

  rObject call_apply(UrbiJob& job,
                     object::rObject target,
                     object::rObject routine,
                     libport::Symbol message,
                     const object::objects_type& args = object::objects_type());

  Action  call_apply(object::rObject function,
                     libport::Symbol msg,
                     const object::objects_type& args,
                     object::rObject call_message = 0);

  rObject call_apply(UrbiJob& job,
                     object::rObject function,
                     libport::Symbol msg,
                     const object::objects_type& args,
                     object::rObject call_message = 0);

  rObject call_apply(UrbiJob& job,
                     object::Object* function,
                     libport::Symbol msg,
                     const object::objects_type& args,
                     object::Object* call_message,
                     boost::optional< ::ast::loc> loc);


  /*--------------------------.
  | Apply with a call message |
  `--------------------------*/

  rObject call_msg(UrbiJob& job,
                   object::Object* function,
                   libport::Symbol msg,
                   object::Object* call_message);

  rObject call_msg(UrbiJob& job,
                   object::Object* function,
                   libport::Symbol msg,
                   object::Object* call_message,
                   boost::optional< ::ast::loc> loc);

  /*-----------------------------------------------.
  | Apply an urbi function (i.e., not a primitive) |
  `-----------------------------------------------*/

  rObject call_apply_urbi(UrbiJob& job,
                          object::Code* function,
                          libport::Symbol msg,
                          const object::objects_type& args,
                          object::Object* call_message_);

  rObject call_funargs(UrbiJob& job,
                       object::Code* function,
                       libport::Symbol msg,
                       const object::objects_type& args);

  /*-------------------------------------.
  | Apply with arguments as ast chunks.  |
  `-------------------------------------*/

  rObject call_msg(UrbiJob& job,
                   rObject target,
                   libport::Symbol message,
                   const ::ast::exps_type* arguments,
                   boost::optional< ::ast::loc> loc);

  rObject call_msg(UrbiJob& job,
                   object::Object* target,
                   object::Object* routine,
                   libport::Symbol message,
                   const ::ast::exps_type* input_ast_args,
                   boost::optional< ::ast::loc> loc);

} // namespace eval

# if defined LIBPORT_COMPILATION_MODE_SPEED
#  include <eval/call.hxx>
# endif

#endif // ! EVAL_CALL_HH
