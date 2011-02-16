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
#include <urbi/object/enumeration.hh>

#include <object/urbi/symbols.hh>
#include <object/urbi/logger.hh>

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
    Logger::init_helper(libport::debug::category_type name)
    {
      category_ = name;
      slot_set(SYMBOL(onEnter), proto->slot_get(SYMBOL(onEnter)));
      slot_set(SYMBOL(onLeave), proto->slot_get(SYMBOL(onLeave)));
    }

    void
    Logger::init(libport::debug::category_type name)
    {
      level_ = proto->level_;
      init_helper(name);
    }

    void
    Logger::init(libport::debug::category_type name, rObject level)
    {
      level_ = from_urbi<libport::Debug::levels::Level>(level);
      init_helper(name);
    }

#ifndef LIBPORT_DEBUG_DISABLE
# define LEVEL(Name, Type, Lev)                         \
    rObject                                             \
    Logger::Name(const std::string& msg,                \
                 const std::string& category)           \
    {                                                   \
      msg_(libport::Debug::types::Type,                 \
           libport::Debug::levels::Lev, msg, category); \
      return this;                                      \
    }                                                   \
    rObject                                             \
    Logger::Name(const std::string& msg)                \
    {                                                   \
      msg_(libport::Debug::types::Type,                 \
           libport::Debug::levels::Lev, msg);           \
      return this;                                      \
    }
#else
# define LEVEL(Name, Type, Level)               \
    rObject                                     \
    Logger::Name(const std::string&,            \
                 const std::string&)            \
    {                                           \
      return this;                              \
    }                                           \
    rObject                                     \
    Logger::Name(const std::string&)            \
    {                                           \
      return this;                              \
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
      return
        category_
        ? libport::format("Logger<%s>", *category_)
        : libport::format("Logger_%p", this);
    }

    rObject
    Logger::operator<<(const std::string& msg)
    {
      switch(level_)
      {
#define CASE(Level)                             \
        case libport::Debug::levels::Level:     \
          Level(msg);                           \
        break
        CASE(log);
        CASE(trace);
        CASE(debug);
        CASE(dump);
#undef CASE
      case libport::Debug::levels::none:
        RAISE("no log level defined");
      }

      return this;
    }

    URBI_CXX_OBJECT_REGISTER_INIT(Logger)
    {
      proto_add(Tag::proto);
      level_ = libport::Debug::levels::log;
      category_ = SYMBOL(Logger);

      bind(SYMBOL(init),
           static_cast<void (Logger::*)()>(&Logger::init));
      bind(SYMBOL(init),
           static_cast<void (Logger::*)(libport::debug::category_type)>
           (&Logger::init));
      bind(SYMBOL(init),
           static_cast<void (Logger::*)(libport::debug::category_type, rObject)>
           (&Logger::init));

#define DECLARE(Name)                                           \
      bind(SYMBOL_(Name),                                       \
           static_cast<rObject (Logger::*)(const std::string&,  \
                                           const std::string&)> \
           (&Logger::Name));                                    \
      bind(SYMBOL_(Name),                                       \
           static_cast<rObject (Logger::*)(const std::string&)> \
           (&Logger::Name))

      DECLARE(debug);
      DECLARE(dump);
      DECLARE(err);
      DECLARE(log);
      DECLARE(trace);
      DECLARE(warn);

#undef DECLARE

#define DECLARE(Name, Cxx)                      \
      bind(SYMBOL_(Name), &Logger::Cxx)

      DECLARE(asPrintable, as_printable);
#undef DECLARE

      bind(libport::Symbol( "<<" ), &Logger::operator<<);

#define DECLARE(Name)                           \
      bind(SYMBOL_(Name), &Logger::Name)

      DECLARE(onEnter);
      DECLARE(onLeave);
#undef DECLARE
    }

    URBI_ENUM_REGISTER(libport::Debug::levels::Level, Global.Logger.Levels,
                       (libport::Debug::levels::none,  None),
                       (libport::Debug::levels::log,   Log),
                       (libport::Debug::levels::trace, Trace),
                       (libport::Debug::levels::debug, Debug),
                       (libport::Debug::levels::dump,  Dump));
  }
}
