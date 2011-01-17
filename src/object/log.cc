/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <runner/interpreter.hh>
#include <object/log.hh>

namespace urbi
{
  namespace object
  {
    Log::Log()
      : Tag()
    {
      proto_add(proto);
    }

    Log::Log(rLog model)
      : Tag()
    {
      proto_add(model);
    }

    Log::~Log()
    {}

#define LEVEL(Name, Type, Level)                                \
    rObject                                                     \
    Log::Name(const std::string& category,                      \
              const std::string& msg)                           \
    {                                                           \
      msg_(libport::Debug::types::Type,                         \
           libport::Debug::levels::Level, category, msg);       \
      return this;                                              \
    }                                                           \

    LEVEL(log, info, log);
    LEVEL(trace, info, trace);
    LEVEL(debug, info, debug);
    LEVEL(dump, info, dump);
    LEVEL(err, error, log);
    LEVEL(warn, warn, log);
#undef LEVEL
    void
    Log::msg_(libport::Debug::types::Type type,
              libport::Debug::levels::Level level,
              const std::string& category,
              const std::string& msg)
    {
      static ::libport::debug::category_type c =
        libport::debug::add_category(libport::debug::category_type(category));

      std::string function = "<urbi-toplevel>";
      std::string file = "<urbi-stdin>";
      int line = 0;

      const ast::Ast* ast = ::kernel::interpreter().innermost_node();
      if (ast)
      {
        if (libport::Symbol* f = ast->location_get().begin.filename)
          file = f->name_get();
        line = ast->location_get().begin.line;
      }

      const runner::Interpreter::call_stack_type& bt =
        ::kernel::interpreter().call_stack_get();
      if (bt.size() > 1)
        function = bt[bt.size() - 2].first.name_get();

      if (GD_DEBUGGER &&
          GD_DEBUGGER->enabled(level, c))

        GD_DEBUGGER->debug(msg,
                           type, c, function,
                           // FIXME: use full location when GD handles it
                           file, line);
    }

    void
    Log::onEnter()
    {
      libport::debugger_data().indent++;
    }

    void
    Log::onLeave()
    {
      libport::debugger_data().indent--;
    }

    URBI_CXX_OBJECT_INIT(Log)
    {
      proto_add(Tag::proto);
      bind(SYMBOL(debug),   &Log::debug);
      bind(SYMBOL(dump),    &Log::dump);
      bind(SYMBOL(err),     &Log::err);
      bind(SYMBOL(log),     &Log::log);
      bind(SYMBOL(onEnter), &Log::onEnter);
      bind(SYMBOL(onLeave), &Log::onLeave);
      bind(SYMBOL(trace),   &Log::trace);
      bind(SYMBOL(warn),    &Log::warn);
    }
  }
}
