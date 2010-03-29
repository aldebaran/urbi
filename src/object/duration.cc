/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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

    Duration::Duration(value_type seconds)
      : Float(seconds)
    {
      proto_add(proto);
    }

    Duration::Duration(rDuration model)
      : Float(model->value_get())
    {
      proto_add(proto);
    }

    void
    Duration::init(const objects_type& args)
    {
      check_arg_count(args.size(), 0, 1);
      value_get() = args.empty() ? 0 : from_urbi<value_type>(args[0]);
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
      bind(SYMBOL(init), &Duration::init);
      bind(SYMBOL(seconds), &Duration::seconds);
    }
  }
}
