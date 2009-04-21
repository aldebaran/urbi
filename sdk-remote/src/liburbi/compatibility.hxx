#include <urbi/uobject.hh>

namespace urbi
{
  namespace compatibility
  {

    /*----------.
    | channel.  |
    `----------*/

    inline
    std::string
    channel_construct(const std::string& name)
    {
      if (kernelMajor() < 2)
        return "";
      else
        // Really create the channel for this tag, as the user is
        // probably using this tag in the code.
        return ("if (!hasSlot(\"" + name + "\"))\n"
                "{\n"
                "  var this." + name + " = Channel.new(\"" + name + "\")|\n"
                "  var this.__created_chan__ = true |\n"
                "}|\n");
    }

    inline
    std::string
    channel_destroy(const std::string& name)
    {
      if (kernelMajor() < 2)
        return "";
      else
        return ("if (hasSlot(\"__created_chan__\"))\n"
                "{\n"
                "  removeSlot(\"__created_chan__\")|\n"
                "  removeSlot(\"" + name + "\")|\n"
                "};\n");
    }



    /*----------------------.
    | evaluate_in_channel.  |
    `----------------------*/

    inline
    std::string
    evaluate_in_channel_open(const std::string& tag)
    {
      if (kernelMajor() < 2)
        return tag + " << ";
      else
        return
          channel_construct(tag) +
          "try {" + tag + " << ";
    }

    inline
    std::string
    evaluate_in_channel_close(const std::string& name)
    {
      if (kernelMajor() < 2)
        return "";
      else
        return ("}\n"
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
