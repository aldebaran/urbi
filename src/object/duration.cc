/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/lexical-cast.hh>

#include <urbi/object/symbols.hh>
#include <urbi/object/duration.hh>

namespace urbi
{
  namespace object
  {
    // Compute with doubles.
#define MILLION 1000000.0

    /*---------------.
    | Construction.  |
    `---------------*/

    Duration::Duration(value_type seconds)
      : Float(seconds)
    {
      proto_add(proto);
    }

    Duration::Duration(const boost::posix_time::time_duration& val)
      : Float(val.total_microseconds() / MILLION)
    {
      proto_add(proto);
    }

    Duration::Duration(rDuration model)
      : Float(model->value_get())
    {
      proto_add(proto);
    }

    URBI_CXX_OBJECT_INIT(Duration)
      : Float(0)
    {
      proto_add(Float::proto);

      bind(SYMBOL(init),
           static_cast<void (Duration::*)()>(&Duration::init));
      bind(SYMBOL(init),
           static_cast<void (Duration::*)(const Duration::value_type&)>
             (&Duration::init));

#define DECLARE(Name, Cxx)               \
      bind(SYMBOL_(Name), &Duration::Cxx)

      DECLARE(asPrintable, asPrintable);
      DECLARE(asString,    as_string);
      DECLARE(seconds,     seconds);

#undef DECLARE
    }

    void
    Duration::init()
    {
      value_get() = 0;
    }

    void
    Duration::init(const value_type& v)
    {
      value_get() = v;
    }

    /*-----------.
    | Printing.  |
    `-----------*/

    std::string
    Duration::as_string() const
    {
      return string_cast(value_get()) + "s";
    }

    std::string
    Duration::asPrintable() const
    {
      return libport::format("Duration(%s)", *this);
    }

    /*--------------.
    | Conversions.  |
    `--------------*/

    Duration::value_type
    Duration::seconds() const
    {
      return value_get();
    }

    boost::posix_time::time_duration
    Duration::boost_duration() const
    {
      return
        CxxConvert<boost::posix_time::time_duration>
        ::to(const_cast<Duration*>(this));
    }

  }
}
