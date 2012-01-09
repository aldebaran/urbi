/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cstdlib>

#include <libport/compiler.hh>
#include <libport/symbol.hh>

#include <urbi/kernel/userver.hh>

#include <ast/print.hh>
#include <ast/exp.hh>

#include <urbi/object/symbols.hh>
#include <object/system.hh>

#include <runner/sneaker.hh>

#include <urbi/object/lobby.hh>

namespace dbg
{
  using namespace runner;

  class Sneaker : public Interpreter
  {
  public:
    Sneaker(rLobby lobby,
	      sched::Scheduler& scheduler);
    ATTRIBUTE_NORETURN virtual void work();

    virtual
    void send_message(const std::string& tag, const std::string& msg) const;
  };

  // The sole sneaker instance.
  static Sneaker* sneaker;

  runner::Runner&
  runner_or_sneaker_get()
  {
    if (kernel::scheduler().is_current_job(0))
    {
      passert(sneaker, sneaker);
      return *sneaker;
    }
    return ::kernel::runner();
  }

  Sneaker::Sneaker(object::rLobby lobby, sched::Scheduler& scheduler)
    : Interpreter(lobby, scheduler, ast::rConstAst(), SYMBOL(LT_sneaker_GT))
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
  Sneaker::send_message(const std::string& tag, const std::string& msg) const
  {
    if (!tag.empty())
      std::cerr << tag << ": ";
    std::cerr << msg << std::endl;
  }

  void
  create_sneaker_if_needed(object::rLobby lobby, sched::Scheduler& scheduler)
  {
    if (!sneaker)
      sneaker = new Sneaker(lobby, scheduler);
    // Do not start the sneaker, we do not want to register it with the
    // scheduler.
  }

  void
  dump(const object::rObject& o, int depth)
  {
    aver(sneaker);
    o->dump(std::cerr, depth);
  }

  void
  pp(ast::rAst ast)
  {
    aver(sneaker);
    std::cerr << *ast;
  }

  object::rObject
  eval(const char* command)
  {
    aver(sneaker);
    return ::urbi::object::eval(command);
  }

  void
  evalp(const char* command)
  {
    object::rObject o = eval(command);
    o->call(SYMBOL(print));
  }

  void
  ps()
  {
    aver(sneaker);
    std::cerr << "system_class: " << object::system_class.get() << std::endl;
    try
    {
      object::system_class->call(SYMBOL(ps));
    }
    catch (...)
    {
    }
  }

} // namespace dbg
