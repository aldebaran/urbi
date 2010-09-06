/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/lexical-cast.hh>

#include <object/symbols.hh>

#include <urbi/object/cxx-conversions.hh>
#include <urbi/object/file.hh>
#include <urbi/object/global.hh>
#include <urbi/object/path.hh>
#include <urbi/object/position.hh>
#include <urbi/object/string.hh>

namespace urbi
{
  namespace object
  {

    /*---------------.
    | Construction.  |
    `---------------*/

    Position::Position()
      : pos_()
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Position::Position(const value_type& pos)
      : pos_(pos)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Position::Position(libport::Symbol* f, unsigned int l, unsigned int c)
      : pos_(f, l, c)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Position::Position(rPosition model)
      : pos_(model->pos_)
    {
      CAPTURE_GLOBAL(Orderable);
      proto_add(Orderable);
      proto_add(proto);
    }

    void
    Position::init(const objects_type& args)
    {
      check_arg_count(args.size(), 0, 3);
      switch (args.size())
      {
      case 1:
        pos_ = type_check<Position>(args[0])->pos_;
        return;
      case 2:
        pos_ = value_type(0,
                          CxxConvert<unsigned>::to(args[0], 0),
                          CxxConvert<unsigned>::to(args[1], 1));
        return;
      case 3:
        libport::Symbol* f = 0;
        if (args[0] != nil_class)
          f = new libport::Symbol(from_urbi<std::string>(args[0]));
        pos_ = value_type(f,
                          CxxConvert<unsigned>::to(args[1], 1),
                          CxxConvert<unsigned>::to(args[2], 2));
        return;
      }
    }

    /*-------------.
    | Comparison.  |
    `-------------*/

    bool
    operator ==(const Position::value_type& lhs,
                const Position::value_type& rhs)
    {
      return lhs.line == rhs.line && lhs.column == rhs.column;
    }

    bool
    operator <(const Position::value_type& lhs,
               const Position::value_type& rhs)
    {
      return lhs.line < rhs.line
        || ( lhs.line == rhs.line && lhs.column < rhs.column);
    }

    /*-------------.
    | Operations.  |
    `-------------*/

    rPosition
    Position::operator - (int rhs) const
    {
      return this->operator+(-rhs);
    }

    rPosition
    Position::operator + (int rhs) const
    {
      return (new Position(pos_))->columns(rhs);
    }

    rPosition
    Position::lines(int count)
    {
      pos_.lines(count);
      return this;
    }

    rPosition
    Position::columns(int count)
    {
      // bison position does not handle cases when the count operand is lower
      // than (- column).
      int column = pos_.column;
      count = std::max(count, -column);

      pos_.columns(count);
      return this;
    }

    /*--------------.
    | Conversions.  |
    `--------------*/

    std::string
    Position::as_string() const
    {
      return string_cast(pos_);
    }

    /*-----------.
    | Accessor.  |
    `-----------*/

    rObject
    Position::file_get() const
    {
      if (!pos_.filename)
        return nil_class;
      return to_urbi<libport::path>(pos_.filename->name_get());
    }

    void
    Position::file_set(rObject o)
    {
      if (!pos_.filename)
        delete pos_.filename;
      pos_.filename = 0;
      if (o != nil_class)
        pos_.filename = new libport::Symbol(from_urbi<std::string>(o));
    }

    /*-----------------.
    | Binding system.  |
    `-----------------*/

    void
    Position::initialize(CxxObject::Binder<Position>& bind)
    {
      typedef bool (Position::*comparison_type)(rPosition rhs) const;
      bind(SYMBOL(EQ_EQ), (comparison_type) &Position::operator ==);
      bind(SYMBOL(LT), (comparison_type) &Position::operator <);
      bind(SYMBOL(MINUS), &Position::operator -);
      bind(SYMBOL(PLUS), &Position::operator +);
      bind(SYMBOL(lines), &Position::lines);
      bind(SYMBOL(columns), &Position::columns);
      bind(SYMBOL(asString), &Position::as_string);
      bind(SYMBOL(init), &Position::init);

#define DECLARE(Name)                                           \
      bind.var(SYMBOL( Name ), &Position:: Name ## _ref)

      DECLARE(line);
      DECLARE(column);

#undef DECLARE

#define DECLARE(Name)                                                   \
      bind(SYMBOL(Name), &Position::Name ##_get);                     \
      bind.proto()->property_set(SYMBOL(Name),                          \
                                 SYMBOL(updateHook),                    \
                                 make_primitive(&Position::Name ##_set))

      DECLARE(file);

#undef DECLARE
    }

    URBI_CXX_OBJECT_REGISTER(Position)
      : pos_()
    {}

  } // namespace object
} // namespace urbi
