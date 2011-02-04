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

# include <libport/debug.hh>

# include <urbi/object/tag.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Logger: public Tag
    {
    public:
      Logger();
      Logger(rLogger model);
      virtual ~Logger();

    /*-------------------.
    | Logger functions.  |
    `-------------------*/
    public:
#define LEVEL(Level)                                                    \
      rObject Level(const std::string& msg, const std::string& category)

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
                const std::string& category,
                const std::string& msg);
#endif
      URBI_CXX_OBJECT(Logger, Tag);
    };
  }
}

#endif
