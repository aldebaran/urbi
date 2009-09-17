/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/kernel-version.hh>

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
        return (SYNCLINE_PUSH()
                "try { Channel.new(\"" + name + "\") << {\n"
                SYNCLINE_POP());
    }

    inline
    std::string
    evaluate_in_channel_close(const std::string& name, unsigned major)
    {
      if (major < 2)
        return ",";
      else
        return ("\n"
                SYNCLINE_PUSH()
                "} }\n"
                "catch (var e)\n"
                "{\n"
                "  lobby.send(\"!!! \" + e, \"" + name + "\");\n"
                "},\n"
                SYNCLINE_POP());
    }

    /*-------.
    | emit.  |
    `-------*/

    inline
    std::string
    emit(const std::string& event)
    {
      return 2 <= kernelMajor() ? event + "!" : "emit " + event;
    }


    /*---------.
    | isvoid.  |
    `---------*/

    inline
    std::string
    isvoid(const std::string& exp)
    {
      return 2 <= kernelMajor() ?
        "(" + exp + ").isVoid" : "isvoid(" + exp + ")";
    }
  }
}
