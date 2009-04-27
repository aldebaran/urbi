#include <urbi/uobject.hh>

namespace urbi
{
  namespace compatibility
  {


    /*----------------------.
    | evaluate_in_channel.  |
    `----------------------*/

    inline
    std::string
    evaluate_in_channel_open(const std::string& name)
    {
      if (kernelMajor() < 2)
        return name + " << ";
      else
        return "try { Channel.new(\"" + name + "\") << { ";
    }

    inline
    std::string
    evaluate_in_channel_close(const std::string& name)
    {
      if (kernelMajor() < 2)
        return "";
      else
        return ("} }\n"
                "catch (var e)\n"
                "{\n"
                "  lobby.send(\"!!! \" + e, \"" + name + "\");\n"
                "};\n");
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
