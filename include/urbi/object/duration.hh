/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_DURATION_HH
# define OBJECT_DURATION_HH

# include <boost/date_time/posix_time/posix_time.hpp>

# include <urbi/object/float.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API Duration: public Float
    {
    /*---------------.
    | Construction.  |
    `---------------*/

    public:
      Duration(value_type seconds = 0);
      Duration(const boost::posix_time::time_duration& val);
      Duration(rDuration model);
      void init();
      void init(const value_type& v);

    /*-----------.
    | Printing.  |
    `-----------*/

    public:
      virtual std::string as_string() const;
      std::string asPrintable() const;

    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      value_type seconds() const;
      boost::posix_time::time_duration boost_duration() const;

    /*----------.
    | Details.  |
    `----------*/

    private:
      URBI_CXX_OBJECT(Duration, Float);
    };
  }
}

# include <urbi/object/duration.hxx>

#endif
