/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/enumeration.hh>

#include <urbi/object/symbols.hh>
#include <object/urbi/logger.hh>

namespace urbi
{
  namespace object
  {
    Logger::Logger()
      : Tag()
      , category_(SYMBOL(Logger))
      , type_(types::info)
      , level_(levels::log)
    {
      proto_add(proto);
    }

    Logger::Logger(rLogger model)
      : Tag()
      , category_(model->category_)
      , type_(model->type_)
      , level_(model->level_)
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

    void
    Logger::msg_(libport::Debug::types::Type type,
                 libport::Debug::levels::Level level,
                 const std::string& msg,
                 boost::optional<std::string> category)
    {
      LIBPORT_USE(type, level, msg, category);
#if ! defined LIBPORT_DEBUG_DISABLE
      if (! category && ! category_)
        FRAISE("no category defined");

      libport::debug::category_type c =
        category ? libport::debug::category_type(*category) : *category_;

      libport::debug::add_category(c);

      if (GD_DEBUGGER && GD_DEBUGGER->enabled(level, c))
      {
        ast::loc loc = ::kernel::current_location();
        std::string file = (loc.begin.filename
                            ? loc.begin.filename->name_get()
                            : "<stdin>");

        // FIXME: use full location when GD handles it
        GD_DEBUGGER->debug(msg, type, c,
                           ::kernel::current_function_name(),
                           file, loc.begin.line);
      }
#endif
    }

# define LEVEL(Name, Type, Lev)                         \
    Logger*                                             \
    Logger::Name(const std::string& msg,                \
                 const std::string& category)           \
    {                                                   \
      msg_(types::Type, levels::Lev, msg, category);    \
      return this;                                      \
    }                                                   \
                                                        \
    Logger*                                             \
    Logger::Name(const std::string& msg)                \
    {                                                   \
      msg_(types::Type, levels::Lev, msg);              \
      return this;                                      \
    }                                                   \
                                                        \
    Logger*                                             \
    Logger::Name()                                      \
    {                                                   \
      Logger* res(this);                                \
      res->type_ = types::Type;                         \
      res->level_ = levels::Lev;                        \
      return res;                                       \
    }

    LEVEL(log,   info,  log);
    LEVEL(trace, info,  trace);
    LEVEL(debug, info,  debug);
    LEVEL(dump,  info,  dump);
    LEVEL(err,   error, log);
    LEVEL(warn,  warn,  log);
#undef LEVEL

    void
    Logger::onEnter()
    {
      GD_INDENTATION_INC();
    }

    void
    Logger::onLeave()
    {
      GD_INDENTATION_DEC();
    }

    std::string
    Logger::as_printable() const
    {
      return
        category_
        ? libport::format("Logger<%s>", *category_)
        : libport::format("Logger_%p", this);
    }

    Logger*
    Logger::operator<<(rObject o)
    {
      msg_(type_, level_, o->as_string());
      return this;
    }

    URBI_CXX_OBJECT_REGISTER_INIT(Logger)
      : Tag()
      , category_(SYMBOL(Logger))
      , type_(types::info)
      , level_(levels::log)
    {
      proto_add(Tag::proto);

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
           static_cast<Logger* (Logger::*)(const std::string&,  \
                                           const std::string&)> \
           (&Logger::Name));                                    \
      bind(SYMBOL_(Name),                                       \
           static_cast<Logger* (Logger::*)(const std::string&)> \
           (&Logger::Name));                                    \
      bind(SYMBOL_(Name),                                       \
           static_cast<Logger* (Logger::*)()>                   \
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

    URBI_ENUM_REGISTER(Logger::levels::Level, Global.Logger.Levels,
                       (Logger::levels::none,  None),
                       (Logger::levels::log,   Log),
                       (Logger::levels::trace, Trace),
                       (Logger::levels::debug, Debug),
                       (Logger::levels::dump,  Dump));
  }
}
