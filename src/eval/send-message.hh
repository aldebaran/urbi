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
 ** \file eval/send-msg.hh
 ** \brief Definition of eval::send_message.
 */

#ifndef EVAL_SEND_MESSAGE_HH
# define EVAL_SEND_MESSAGE_HH

# include <runner/state.hh>
# include <eval/action.hh>

namespace eval
{
  /// Send a message to the current lobby.
  ///
  /// \param job The job in which this function is executed.
  /// \param tag The tag of the timesptamp [000000000:tag]
  /// \param msg The message to be printed.
  rObject send_message(Job& job,
                       const std::string& tag,
                       const std::string& msg);

  /// Send an error message to the current lobby.
  ///
  /// \param msg The message to be printed. "!!! " will be prepended.
  /// \param job The job in which this function is executed.
  rObject send_error(const std::string& msg, Job& job = ::kernel::runner());

  Action  send_message(const std::string& tag,
                       const std::string& msg);

  // FIXME: Should be moved inside its own file ?

  void show_backtrace(Job& job,
                      const runner::State::call_stack_type& bt,
                      const std::string& chan);

  /// Send the current backtrace through the connection.
  ///
  /// \param job The job in which this function is executed.
  /// \param chan The channel to print through.
  void show_backtrace(Job& job,
                      const std::string& chan);

  void show_exception(const object::UrbiException& ue,
                      Job& job = ::kernel::runner(),
                      const std::string& tag = "error");


  Action  verb_message(const std::string& tag, Action act);

  rObject verb_message(Job& job, const std::string& tag, Action act);

}

# include <eval/send-message.hxx>

#endif // !EVAL_SEND_MESSAGE_HH
