/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_POSITION_HH
# define OBJECT_POSITION_HH

# include <parser/position.hh>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {

    class Position: public CxxObject
    {

    /*---------------.
    | Construction.  |
    `---------------*/

    public:
      typedef ::yy::position value_type;
      Position();
      Position(const value_type& pos);
      Position(libport::Symbol* f, unsigned int l, unsigned int c);
      Position(rPosition model);
      void init(const objects_type& args);

    /*--------------.
    | Comparisons.  |
    `--------------*/

    public:
      using Object::operator <;
      bool operator ==(rPosition rhs) const;
      bool operator <(rPosition rhs) const;

    /*-------------.
    | Operations.  |
    `-------------*/

    public:
      rPosition operator - (int rhs) const;
      rPosition operator + (int rhs) const;

      rPosition lines(int c);
      rPosition columns(int c);

    /*--------------.
    | Conversions.  |
    `--------------*/

    public:
      virtual std::string as_string() const;
      value_type& value_get();

    /*-----------.
    | Accessor.  |
    `-----------*/

    private:
      rObject file_get() const;
      void file_set(rObject o);

      unsigned int* line_ref();
      unsigned int* column_ref();

    /*----------.
    | Details.  |
    `----------*/

    private:
      value_type pos_;

      URBI_CXX_OBJECT_(Position);
    };

    bool
    operator ==(const Position::value_type& lhs,
                const Position::value_type& rhs);

    bool
    operator <(const Position::value_type& lhs,
               const Position::value_type& rhs);


  } // namespace object
} // namespace urbi

#include <urbi/object/position.hxx>

#endif
