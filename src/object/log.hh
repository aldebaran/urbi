/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_LOG_HH
# define OBJECT_LOG_HH

# include <libport/debug.hh>

# include <urbi/object/tag.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Log: public Tag
    {
    public:
      Log();
      Log(rLog model);
      virtual ~Log();

    /*----------------.
    | Log functions.  |
    `----------------*/
    public:
      rObject log(const std::string& category, const std::string& msg);
      rObject trace(const std::string& category, const std::string& msg);
      rObject debug(const std::string& category, const std::string& msg);
      rObject dump(const std::string& category, const std::string& msg);
      rObject warn(const std::string& category, const std::string& msg);
      rObject err(const std::string& category, const std::string& msg);

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
      void msg_(libport::Debug::types::Type type,
                libport::Debug::levels::Level level,
                const std::string& category,
                const std::string& msg);
      URBI_CXX_OBJECT(Log, Tag);
    };
  }
}

#endif
