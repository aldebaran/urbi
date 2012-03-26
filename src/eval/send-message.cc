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
 ** \file eval/send-msg.cc
 ** \brief Definition of eval::send_message.
 */

#include <runner/job.hh>

#include <eval/send-message.hh>
#include <libport/format.hh>
#include <libport/debug.hh>

#include <urbi/object/fwd.hh>
#include <urbi/object/global.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/string.hh>

#include <runner/state.hh>
#include <runner/job.hh>
#include <runner/sneaker.hh>

namespace eval
{

  rObject
  send_message(Job& job,
               const std::string& tag,
               const std::string& msg)
  {
    if (dbg::is_sneaker(job)) // unlikely
    {
      if (!tag.empty())
        std::cerr << tag << ": ";
      std::cerr << msg << std::endl;
      return object::void_class;
    }

    // If there is a Channel object with name 'tag', use it.
    object::Lobby* lobby = job.state.lobby_get();
    rObject chan = lobby->slot_get_value(libport::Symbol(tag), false);
    CAPTURE_LANG(lang);
    if (chan && is_a(chan, lang->slot_get_value(SYMBOL(Channel))))
      return
        chan->call(SYMBOL(LT_LT),
                   new object::String(msg));
    else
      return
        lobby->call(SYMBOL(send),
                    new object::String(msg),
                    new object::String(tag));
  }

  void
  show_backtrace(Job& job,
                 const runner::State::call_stack_type& bt,
                 const std::string& chan)
  {
    rforeach (const runner::State::call_type& c, bt)
      send_message(job,
                   chan,
                   libport::format("!!!    called from: %s", c));
  }

  void
  show_exception(const object::UrbiException& ue,
                 Job& job,
                 const std::string& tag)
  {
    CAPTURE_GLOBAL(Exception);

    // FIXME: should bounce in all case to Exception.'$show'.
    if (is_a(ue.value_get(), Exception))
      ue.value_get()->call(SYMBOL(DOLLAR_show));
    else
    {
      send_message(job, tag,
                   libport::format("!!! %s", *ue.value_get()));
      show_backtrace(job, ue.backtrace_get(), tag);
    }
  }

}
