#include <libport/finally.hh>

#include <object/object.hh>
#include <object/tag-class.hh>
#include <runner/shell.hh>
#include <scheduler/scheduler.hh>

namespace runner
{
  Shell::Shell(rLobby lobby,
	       scheduler::Scheduler& scheduler,
	       const libport::Symbol& name)
    : Interpreter(lobby,
		  scheduler,
		  0,
		  name)
    , executing_(false)
  {
    rObject connection_tag = lobby_->slot_locate(SYMBOL(connectionTag));
    if (connection_tag)
      push_tag(extract_tag(connection_tag->slot_get(SYMBOL(connectionTag))));
  }

  void
  Shell::work()
  {
    while (true)
    {
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
	  yield_until_things_changed();

	executing_ = true;
      }

      ast::rConstNary nary = commands_.front();
      commands_.pop_front();
      eval(nary);
    }
  }

  void
  Shell::append_command(ast::rConstNary command)
  {
    commands_.push_back(command);
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
