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

#ifndef EVAL_SEND_MESSAGE_HXX
# define EVAL_SEND_MESSAGE_HXX

# include <libport/foreach.hh>
# include <libport/format.hh>
# include <libport/debug.hh>

# include <urbi/object/fwd.hh>
# include <urbi/object/global.hh>
# include <urbi/object/lobby.hh>
# include <urbi/object/string.hh>


# include <runner/state.hh>
# include <runner/job.hh>
# include <runner/sneaker.hh>

namespace eval
{
  inline
  rObject send_message(Job& job,
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
    object::rSlot chan_slot =
      lobby->slot_locate(libport::Symbol(tag), true).second;
    rObject chan = chan_slot ? chan_slot->value() : rObject();
    if (chan && is_a(chan, lobby->slot_get(SYMBOL(Channel))))
      return
        chan->call(SYMBOL(LT_LT_),
                   new object::String(msg));
    else
      return
        lobby->call(SYMBOL(send),
                    new object::String(msg),
                    new object::String(tag));
  }

  inline
  Action  send_message(const std::string& tag,
                       const std::string& msg)
  {
    return boost::bind(&send_message, _1, tag, msg);
  }

  inline void
  show_backtrace(Job& job,
                 const runner::State::call_stack_type& bt,
                 const std::string& chan)
  {
    rforeach (const runner::State::call_type& c, bt)
      send_message(job,
                   chan,
                   libport::format("!!!    called from: %s", c));
  }

  inline void
  show_backtrace(Job& job,
                 const std::string& chan)
  {
    // Displaying a stack invokes urbiscript code, which in turn
    // changes the call stack.  Don't play this kind of games.
    runner::State::call_stack_type call_stack(job.state.call_stack_get());
    show_backtrace(job, call_stack, chan);
  }

  inline
  void show_exception(Job& job,
                      const object::UrbiException& ue,
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

  inline
  Action  verb_message(const std::string& tag, Action act)
  {
    return boost::bind(&verb_message, _1, tag, act);
  }

  inline
  rObject verb_message(Job& job, const std::string& tag, Action act)
  {
    send_message(job, tag, "enter");
    rObject res = act(job);
    send_message(job, tag, "leave");
    return res;
  }

}

#endif // !EVAL_SEND_MESSAGE_HXX
