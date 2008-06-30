# include <libport/symbol.hh>

#include <ast/print.hh>
#include <object/system-class.hh>
#include <object/urbi-exception.hh>
#include <runner/sneaker.hh>
#include <runner/interpreter.hh>

namespace dbg
{
  using namespace runner;

  class Sneaker : public Interpreter
  {
  public:
    Sneaker(rLobby lobby,
	      scheduler::Scheduler& scheduler);
    virtual void work();
    virtual void send_message(const std::string& tag, const std::string& msg);
  private:
  };

  static Sneaker* sneaker;                        // The sole sneaker instance

  Sneaker::Sneaker(object::rLobby lobby,
		       scheduler::Scheduler& scheduler)
    : Interpreter(lobby, scheduler, 0, SYMBOL(LT_sneaker_GT))
  {
    non_interruptible_set(true);
  }

  void
  Sneaker::work()
  {
    // This will never be called as the sneaker is not supposed to be
    // registered with the scheduler.
    assert(false);
  }

  void
  Sneaker::send_message(const std::string& tag, const std::string& msg)
  {
    if (!tag.empty())
      std::cerr << tag << ": ";
    std::cerr << msg;
  }

  void
  create_sneaker_if_needed
    (object::rLobby lobby, scheduler::Scheduler& scheduler)
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
    o->dump(std::cerr, *sneaker, depth);
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
      object::execute_parsed(*sneaker, parser::parse(command),
                             SYMBOL(eval),
			     object::InternalError("error evaluating command"));
  }

  void
  evalp(const char* command)
  {
    object::rObject o = eval(command);
    urbi_call(*sneaker, o, SYMBOL(print));
  }

  void
  ps()
  {
    assert(sneaker);
    std::cerr << "system_class: " << object::system_class.get() << std::endl;
    urbi_call(*sneaker, object::system_class, SYMBOL(ps));
  }

} // namespace dbg
