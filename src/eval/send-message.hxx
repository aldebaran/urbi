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
 ** \file eval/send-msg.hxx
 ** \brief Definition of eval::send_message.
 */

#ifndef EVAL_SEND_MESSAGE_HXX
# define EVAL_SEND_MESSAGE_HXX

# include <urbi/object/fwd.hh>

# include <runner/state.hh>
# include <runner/job.hh>
# include <runner/sneaker.hh>

namespace eval
{

  inline
  rObject
  send_error(const std::string& msg, Job& job)
  {
    return send_message(job, "error", "!!! " + msg);
  }

  inline
  Action
  send_message(const std::string& tag,
               const std::string& msg)
  {
    return boost::bind(&send_message, _1, tag, msg);
  }

  inline
  void
  show_backtrace(Job& job,
                 const std::string& chan)
  {
    // Displaying a stack invokes urbiscript code, which in turn
    // changes the call stack.  Don't play this kind of games.
    runner::State::call_stack_type call_stack(job.state.call_stack_get());
    show_backtrace(job, call_stack, chan);
  }

  inline
  Action
  verb_message(const std::string& tag, Action act)
  {
    return boost::bind(&verb_message, _1, tag, act);
  }

  inline
  rObject
  verb_message(Job& job, const std::string& tag, Action act)
  {
    send_message(job, tag, "enter");
    rObject res = act(job);
    send_message(job, tag, "leave");
    return res;
  }

}

#endif // !EVAL_SEND_MESSAGE_HXX
