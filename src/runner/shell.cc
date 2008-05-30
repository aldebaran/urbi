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
		  object::Object::make_method_scope(lobby),
		  scheduler,
		  0,
		  name)
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
      while (commands_.empty())
      {
	bool side_effect_free_save = side_effect_free_get();
	side_effect_free_set(true);
	yield_until_things_changed();
	side_effect_free_set(side_effect_free_save);
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

} // namespace runner
