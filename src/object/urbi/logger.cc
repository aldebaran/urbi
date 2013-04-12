/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <object/urbi/logger.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/dictionary.hh>
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
      init_helper(SYMBOL(Logger));
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
    Logger::init_helper(category_type name)
    {
      category_ = name;
      // Make it exist, at least for sake of "categories()".
      libport::debug::add_category(name);
      slot_set_value(SYMBOL(onEnter), proto->slot_get_value(SYMBOL(onEnter)));
      slot_set_value(SYMBOL(onLeave), proto->slot_get_value(SYMBOL(onLeave)));
    }

    void
    Logger::init(category_type name)
    {
      level_ = proto->level_;
      init_helper(name);
    }

    void
    Logger::init(category_type name, rObject level)
    {
      level_ = from_urbi<levels::Level>(level);
      init_helper(name);
    }

    /*-------------.
    | Categories.  |
    `-------------*/

    rDictionary
    Logger::categories() const
    {
      rDictionary res = new Dictionary;
      foreach (const libport::debug::categories_type::value_type &p,
               libport::debug::categories())
        res->set(to_urbi(p.first), to_urbi(p.second));
      return res;
    }

    void
    Logger::enable(const std::string& pattern)
    {
      libport::debug::enable_category(libport::Symbol(pattern));
    }

    void
    Logger::disable(const std::string& pattern)
    {
      libport::debug::disable_category(libport::Symbol(pattern));
    }

    void
    Logger::set(const std::string& specs)
    {
      libport::debug::set_categories_state(specs, libport::debug::AUTO);
    }


    /*---------.
    | Levels.  |
    `---------*/

    Logger::levels::Level
    Logger::level_get() const
    {
      return GD_CURRENT_LEVEL();
    }

    void
    Logger::level_set(levels::Level level) const
    {
      GD_FILTER(level);
    }

    /*-----------.
    | Messages.  |
    `-----------*/

    void
    Logger::msg_(types::Type type,
                 levels::Level level,
                 const std::string& msg,
                 boost::optional<std::string> category)
    {
      LIBPORT_USE(type, level, msg, category);
#if ! defined LIBPORT_DEBUG_DISABLE
      if (! category && ! category_)
        FRAISE("no category defined");

      category_type c = category ? category_type(*category) : *category_;

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

    URBI_CXX_OBJECT_INIT(Logger)
      : Tag()
      , category_(SYMBOL(Logger))
      , type_(types::info)
      , level_(levels::log)
    {
      proto_add(Tag::proto);

      BIND(asPrintable, as_printable);
      BINDG(categories);
      BIND(disable);
      BIND(enable);
      BIND(init, init, void, ());
      BIND(init, init, void, (category_type));
      BIND(init, init, void, (category_type, rObject));
      BIND(onEnter);
      BIND(onLeave);
      BIND(set);
      bind(libport::Symbol( "<<" ), &Logger::operator<<);
      bind(SYMBOL(level), &Logger::level_get, &Logger::level_set);

#define DECLARE(Name)                                                   \
      BIND(Name, Name, Logger*, (const std::string&, const std::string&)); \
      BIND(Name, Name, Logger*, (const std::string&));                  \
      BIND(Name, Name, Logger*, ())

      DECLARE(debug);
      DECLARE(dump);
      DECLARE(err);
      DECLARE(log);
      DECLARE(trace);
      DECLARE(warn);
#undef DECLARE
    }

  }
}
