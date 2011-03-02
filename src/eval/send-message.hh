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
 ** \file runner/eval/send-msg.hh
 ** \brief Definition of eval::send_message.
 */

#ifndef EVAL_SEND_MESSAGE_HH
# define EVAL_SEND_MESSAGE_HH

# include <runner/urbi-stack.hh>
# include <eval/action.hh>

namespace eval
{
  /// Send a message to the current lobby.
  ///
  /// \param job The job in which this function is executed.
  /// \param tag The tag of the timesptamp [000000000:tag]
  /// \param msg The message to be printed.
  rObject send_message(UrbiJob& job,
                       const std::string& tag,
                       const std::string& msg);

  Action  send_message(const std::string& tag,
                       const std::string& msg);

  // FIXME: Should be moved inside its own file ?

  void show_backtrace(UrbiJob& job,
                      const runner::UrbiStack::call_stack_type& bt,
                      const std::string& chan);

  /// Send the current backtrace through the connection.
  ///
  /// \param job The job in which this function is executed.
  /// \param chan The channel to print through.
  void show_backtrace(UrbiJob& job,
                      const std::string& chan);

  void show_exception(UrbiJob& job,
                      const object::UrbiException& ue,
                      const std::string& tag = "error");


  Action  verb_message(const std::string& tag, Action act);

  rObject verb_message(UrbiJob& job, const std::string& tag, Action act);

}

# include <eval/send-message.hxx>

#endif // !EVAL_SEND_MESSAGE_HH
