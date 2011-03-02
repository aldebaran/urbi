/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <runner/urbi-stack.hh>
#include <runner/urbi-job.hh>
#include <object/logger.hh>

namespace urbi
{
  namespace object
  {
    Logger::Logger()
      : Tag()
    {
      proto_add(proto);
    }

    Logger::Logger(rLogger model)
      : Tag()
    {
      proto_add(model);
    }

    Logger::~Logger()
    {}

#ifdef LIBPORT_DEBUG_DISABLE
# define LEVEL(Name, Type, Level) \
   rObject                                                      \
    Logger::Name(const std::string& msg,                        \
                 const std::string& category)                   \
   {                                                            \
     std::cerr << category  << " : " << msg << std::endl;       \
   }
#else
# define LEVEL(Name, Type, Level)                                \
    rObject                                                     \
    Logger::Name(const std::string& msg,                        \
                 const std::string& category)                   \
    {                                                           \
      msg_(libport::Debug::types::Type,                         \
           libport::Debug::levels::Level, category, msg);       \
      return this;                                              \
    }
#endif

    LEVEL(log,   info,  log);
    LEVEL(trace, info,  trace);
    LEVEL(debug, info,  debug);
    LEVEL(dump,  info,  dump);
    LEVEL(err,   error, log);
    LEVEL(warn,  warn,  log);
#undef LEVEL

#ifndef LIBPORT_DEBUG_DISABLE
    void
    Logger::msg_(libport::Debug::types::Type type,
              libport::Debug::levels::Level level,
              const std::string& category,
              const std::string& msg)
    {
      ::libport::debug::category_type c =
        libport::debug::add_category(libport::debug::category_type(category));

      std::string function = "<urbi-toplevel>";
      std::string file = "<urbi-stdin>";
      int line = 0;

      const ast::Ast* ast =
        ::kernel::interpreter().state.innermost_node_get();
      if (ast)
      {
        if (libport::Symbol* f = ast->location_get().begin.filename)
          file = f->name_get();
        line = ast->location_get().begin.line;
      }

      const runner::UrbiStack::call_stack_type& bt =
        ::kernel::interpreter().state.call_stack_get();
      if (bt.size() > 1)
        function = bt[bt.size() - 2].first.name_get();

      if (GD_DEBUGGER && GD_DEBUGGER->enabled(level, c))
        // FIXME: use full location when GD handles it
        GD_DEBUGGER->debug(msg, type, c, function, file, line);
    }
#endif

    void
    Logger::onEnter()
    {
#ifndef LIBPORT_DEBUG_DISABLE
      libport::debugger_data().indent++;
#endif
    }

    void
    Logger::onLeave()
    {
#ifndef LIBPORT_DEBUG_DISABLE
      libport::debugger_data().indent--;
#endif
    }

    URBI_CXX_OBJECT_INIT(Logger)
    {
      proto_add(Tag::proto);
      bind(SYMBOL(debug),   &Logger::debug);
      bind(SYMBOL(dump),    &Logger::dump);
      bind(SYMBOL(err),     &Logger::err);
      bind(SYMBOL(log),     &Logger::log);
      bind(SYMBOL(onEnter), &Logger::onEnter);
      bind(SYMBOL(onLeave), &Logger::onLeave);
      bind(SYMBOL(trace),   &Logger::trace);
      bind(SYMBOL(warn),    &Logger::warn);
    }
  }
}
