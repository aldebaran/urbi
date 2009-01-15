#include <cstdlib>

#include <libport/compiler.hh>
#include <libport/symbol.hh>

#include <kernel/userver.hh>

#include <ast/print.hh>

#include <object/lobby.hh>
#include <object/system.hh>

#include <runner/call.hh>
#include <runner/sneaker.hh>

namespace dbg
{
  using namespace runner;

  class Sneaker : public Interpreter
  {
  public:
    Sneaker(rLobby lobby,
	      sched::Scheduler& scheduler);
    ATTRIBUTE_NORETURN virtual void work();
    virtual void send_message(const std::string& tag, const std::string& msg);
  private:
  };

  static Sneaker* sneaker;                        // The sole sneaker instance

  runner::Runner&
  runner_or_sneaker_get()
  {
    if (kernel::urbiserver->scheduler_get().is_current_job(0))
    {
      passert(sneaker, sneaker);
      return *sneaker;
    }
    return ::kernel::urbiserver->getCurrentRunner();
  }

  Sneaker::Sneaker(object::rLobby lobby,
		       sched::Scheduler& scheduler)
    : Interpreter(lobby, scheduler, 0, SYMBOL(LT_sneaker_GT))
  {
    non_interruptible_set(true);
  }

  void
  Sneaker::work()
  {
    // This will never be called as...
    pabort("the sneaker is not supposed to be registered with the scheduler");
  }

  void
  Sneaker::send_message(const std::string& tag, const std::string& msg)
  {
    if (!tag.empty())
      std::cerr << tag << ": ";
    std::cerr << msg << std::endl;
  }

  void
  create_sneaker_if_needed
    (object::rLobby lobby, sched::Scheduler& scheduler)
  {
    if (!sneaker)
      sneaker = new Sneaker(lobby, scheduler);
    // Do not start the sneaker, we do not want to register it with the
    // scheduler.
  }

  void
  dump(const object::rObject& o, int depth)
  {
    assert(sneaker);
    o->dump(std::cerr, depth);
  }

  void
  pp(ast::rAst ast)
  {
    assert(sneaker);
    std::cerr << *ast;
  }

  object::rObject
  eval(const char* command)
  {
    assert(sneaker);
    return
      object::execute_parsed(parser::parse(command, __HERE__),
                             SYMBOL(eval),
			     std::string("error evaluating command `") +
			     command + "'");
  }

  void
  evalp(const char* command)
  {
    object::rObject o = eval(command);
    urbi_call(o, SYMBOL(print));
  }

  void
  ps()
  {
    assert(sneaker);
    std::cerr << "system_class: " << object::system_class.get() << std::endl;
    try
    {
      urbi_call(object::system_class, SYMBOL(ps));
    }
    catch (...)
    {
    }
  }

} // namespace dbg
