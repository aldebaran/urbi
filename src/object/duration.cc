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
      : Float(seconds)
    {
      proto_add(proto);
    }

    Duration::Duration(rDuration model)
      : Float(model->value_get())
    {
      proto_add(proto);
    }

    /*-----------.
    | Printing.  |
    `-----------*/

    std::string
    Duration::asString() const
    {
      return string_cast(value_get()) + "s";
    }

    std::string
    Duration::asPrintable() const
    {
      return libport::format("Duration(%s)", asString());
    }

    /*--------------.
    | Conversions.  |
    `--------------*/

    Duration::value_type
    Duration::seconds() const
    {
      return value_get();
    }

    URBI_CXX_OBJECT_REGISTER(Duration)
      : Float(0)
    {
      proto_add(Float::proto);

      bind(SYMBOL(asPrintable), &Duration::asPrintable);
      bind(SYMBOL(asString), &Duration::asString);
      bind(SYMBOL(seconds), &Duration::seconds);
    }
  }
}
