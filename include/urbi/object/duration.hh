/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_DURATION_HH
# define OBJECT_DURATION_HH

# include <urbi/object/float.hh>

namespace urbi
{
  namespace object
  {
    class Duration: public Float
    {
    /*---------------.
    | Construction.  |
    `---------------*/

    public:
      Duration(value_type seconds = 0);
      Duration(rDuration model);
      void init(const objects_type& args);

    /*-----------.
    | Printing.  |
    `-----------*/

    public:
      std::string asString() const;
      std::string asPrintable() const;

    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      value_type seconds() const;

    /*----------.
    | Details.  |
    `----------*/

    private:
      URBI_CXX_OBJECT(Duration);
    };
  }
}

#endif
