/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/kernel-version.hh>
#include <urbi/uabstractclient.hh>

namespace urbi
{
  namespace compatibility
  {


    /*----------------------.
    | evaluate_in_channel.  |
    `----------------------*/

    inline
    std::string
    evaluate_in_channel_open(const std::string& name, unsigned major)
    {
      if (major < 2)
        return name + " << ";
      else
        return SYNCLINE_WRAP("try { Channel.new(\"%s\") << {", name);
    }

    inline
    std::string
    evaluate_in_channel_close(const std::string& name, unsigned major)
    {
      if (major < 2)
        return ",";
      else
        return ("\n"
                + SYNCLINE_WRAP(
                              "} }\n"
                              "catch (var e)\n"
                              "{\n"
                              "  lobby.send(\"!!! \" + e, \"%s\");\n"
                              "},", name));
    }

    /*-------.
    | emit.  |
    `-------*/

    inline
    std::string
    emit(const std::string& event, unsigned major)
    {
      return 2 <= major ? event + "!" : "emit " + event;
    }


    /*---------.
    | isvoid.  |
    `---------*/

    inline
    std::string
    isvoid(const std::string& exp, unsigned major)
    {
      return 2 <= major ?
        "(" + exp + ").isVoid" : "isvoid(" + exp + ")";
    }


    inline
    std::string
    stop(const std::string& tag, unsigned major)
    {
      return 2 <= major ?
        tag + ".stop" : "stop " + tag;
    }
  }
}
