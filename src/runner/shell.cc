/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <libport/finally.hh>

#include <ast/nary.hh>

#include <object/lobby.hh>
#include <object/object.hh>
#include <object/tag.hh>

#include <runner/shell.hh>

#include <sched/scheduler.hh>

namespace runner
{
  Shell::Shell(const rLobby& lobby,
	       sched::Scheduler& scheduler,
	       const libport::Symbol& name)
    : Interpreter(lobby,
		  scheduler,
		  0,
		  name)
    , executing_(false)
  {
  }

  inline
  void
  Shell::handle_oob_()
  {
    while (!oob_calls_.empty())
    {
      oob_calls_.front()();
      oob_calls_.pop_front();
    }
  }

  void
  Shell::work()
  {
    while (true)
    {
      handle_oob_();
      // Wait until we have some work to do
      if (commands_.empty())
      {
	executing_ = false;

	// The first yield will remember our possible side effect. Subsequent
	// ones will not and will pretend that we didn't have any side effect,
	// as we couldn't have influenced the system any further.
	yield_until_things_changed();

	libport::Finally finally(boost::bind(&Job::side_effect_free_set,
					     this,
					     side_effect_free_get()));
	side_effect_free_set(true);
	while (commands_.empty())
	{
          libport::Finally finally(boost::bind(&Job::side_effect_free_set,
                                               this,
                                               side_effect_free_get()));
	  handle_oob_();
	  yield_until_things_changed();
	}
	executing_ = true;
      }

      ast::rConstNary nary = commands_.front();
      commands_.pop_front();
      operator()(nary.get());
      if (non_interruptible_get())
      {
        send_message("error", "the toplevel can not be non-interruptible");
        non_interruptible_set(false);
      }
    }
  }

  void
  Shell::append_command(const ast::rConstNary& command)
  {
    commands_.push_back(command);
    scheduler_get().signal_world_change();
  }

  void
  Shell::insert_oob_call(boost::function0<void> func)
  {
    oob_calls_.push_back(func);
    scheduler_get().signal_world_change();
  }

  bool
  Shell::pending_command_get() const
  {
    return executing_ || !commands_.empty();
  }

  void
  Shell::pending_commands_clear()
  {
    commands_.clear();
  }

} // namespace runner
