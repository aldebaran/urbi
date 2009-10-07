/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_DATE_HH
# define OBJECT_DATE_HH

# include <libport/ctime>

# include <object/cxx-object.hh>

namespace object
{
  class Date: public CxxObject
  {

  /*---------------.
  | Construction.  |
  `---------------*/

  public:
    typedef time_t value_type;
    Date();
    Date(rDate model);
    Date(time_t time);
    void init(const objects_type& args);

  /*--------------.
  | Comparisons.  |
  `--------------*/

  public:
    using Object::operator <;
    bool operator ==(rDate rhs) const;
    bool operator <(rDate rhs) const;

  /*--------------.
  | Conversions.  |
  `--------------*/

  public:
    std::string as_string() const;

  /*----------.
  | Details.  |
  `----------*/

  private:
    time_t time_;

    URBI_CXX_OBJECT(Date);
  };
}

#endif
