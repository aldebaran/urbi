/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_LOGGER_HH
# define OBJECT_LOGGER_HH

# include <boost/optional.hpp>

# include <libport/debug.hh>
# include <libport/symbol.hh>

# include <urbi/object/tag.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Logger: public Tag
    {
    public:
      Logger();
      Logger(std::string category);
      Logger(rLogger model);
      virtual ~Logger();

      void init();
      void init(libport::debug::category_type name);
      std::string as_printable() const;

    /*-------------------.
    | Logger functions.  |
    `-------------------*/
    public:
#define LEVEL(Level)                                                     \
      rObject Level(const std::string& msg, const std::string& category); \
      rObject Level(const std::string& msg)

      LEVEL(log);
      LEVEL(trace);
      LEVEL(debug);
      LEVEL(dump);
      LEVEL(warn);
      LEVEL(err);
#undef LEVEL

    /*--------------.
    | Indentation.  |
    `--------------*/
    public:
      void onEnter();
      void onLeave();

    /*----------.
    | Details.  |
    `----------*/
    private:
#ifndef LIBPORT_DEBUG_DISABLE
      void msg_(libport::Debug::types::Type type,
                libport::Debug::levels::Level level,
                const std::string& msg,
                boost::optional<std::string> category);
      boost::optional<libport::debug::category_type> category_;
      URBI_CXX_OBJECT(Logger, Tag);
    };
  }
}

#endif
