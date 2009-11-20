#include <libport/lexical-cast.hh>

#include <object/symbols.hh>
#include <urbi/object/duration.hh>

namespace urbi
{
  namespace object
  {
    /*---------------.
    | Construction.  |
    `---------------*/

    Duration::Duration(time_t seconds)
      : _seconds(seconds)
    {
      proto_add(proto);
    }

    Duration::Duration(rDuration model)
      : _seconds(model->_seconds)
    {}

    /*-----------.
    | Printing.  |
    `-----------*/

    std::string
    Duration::asString() const
    {
      return string_cast(_seconds) + "s";
    }

    std::string
    Duration::asPrintable() const
    {
      return libport::format("Duration(%s)", asString());
    }

    /*--------------.
    | Conversions.  |
    `--------------*/

    time_t
    Duration::seconds() const
    {
      return _seconds;
    }

    URBI_CXX_OBJECT_REGISTER(Duration)
      : _seconds(0)
    {
      bind(SYMBOL(asPrintable), &Duration::asPrintable);
      bind(SYMBOL(asString), &Duration::asString);
    }
  }
}
