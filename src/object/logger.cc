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

    void
    Logger::init()
    {
      if (proto->category_)
        init(*proto->category_);
    }

    void
    Logger::init(libport::debug::category_type name)
    {
      category_ = name;
      this->slot_set(SYMBOL(onEnter), proto->slot_get(SYMBOL(onEnter)));
      this->slot_set(SYMBOL(onLeave), proto->slot_get(SYMBOL(onLeave)));
    }

#ifdef LIBPORT_DEBUG_DISABLE
# define LEVEL(Name, Type, Level)                                      \
    rObject                                                            \
    Logger::Name(const std::string& msg,                               \
                 const std::string& category)                          \
    {                                                                  \
      msg_(libport::Debug::types::Type,                                \
           libport::Debug::levels::Level, msg, category);              \
      return this;                                                     \
    }                                                                  \
    rObject                                                            \
    Logger::Name(const std::string& msg)                               \
    {                                                                  \
      msg_(libport::Debug::types::Type,                                \
           libport::Debug::levels::Level, msg);                        \
      return this;                                                     \
    }
#else
# define LEVEL(Name, Type, Level)                                      \
    rObject                                                            \
    Logger::Name(const std::string& msg,                               \
                 const std::string& category)                          \
    {                                                                  \
      std::cerr << category  << " : " << msg << std::endl;             \
      return this;                                                     \
    }                                                                  \
    rObject                                                            \
    Logger::Name(const std::string& msg)                               \
    {                                                                  \
      if (! category_)                                                 \
        FRAISE("no category defined");                                 \
      std::cerr << *category_  << " : " << msg << std::endl;           \
      return this;                                                     \
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
                 const std::string& msg,
                 boost::optional<std::string> category)
    {
      if (! category && ! category_)
        FRAISE("no category defined");

      libport::debug::category_type c =
        category ? libport::debug::category_type(*category) : *category_;

      libport::debug::add_category(c);

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

    std::string
    Logger::as_printable() const
    {
      if (category_)
        return libport::format("Logger<%s>", *category_);
      return libport::format("Logger_%p", this);
    }


    URBI_CXX_OBJECT_INIT(Logger)
    {
      proto_add(Tag::proto);

      bind(SYMBOL(init),
           static_cast<void (Logger::*)()>(&Logger::init));
      bind(SYMBOL(init),
           static_cast<void (Logger::*)(libport::debug::category_type)>
           (&Logger::init));

#define DECLARE(Name)                                                   \
      bind(SYMBOL(Name),                                                \
           static_cast<rObject (Logger::*)(const std::string&,          \
                                           const std::string&)>         \
           (&Logger::Name));                                            \
      bind(SYMBOL(Name),                                                \
           static_cast<rObject (Logger::*)(const std::string&)>         \
           (&Logger::Name))

      DECLARE(debug);
      DECLARE(dump);
      DECLARE(err);
      DECLARE(log);
      DECLARE(trace);
      DECLARE(warn);

#undef DECLARE

#define DECLARE(Name, Cxx)                      \
      bind(SYMBOL(Name), &Logger::Cxx)

      DECLARE(asPrintable, as_printable);

#undef DECLARE


#define DECLARE(Name)                                                   \
      bind(SYMBOL(Name), &Logger::Name)

      DECLARE(onEnter);
      DECLARE(onLeave);
#undef DECLARE
    }
  }
}
