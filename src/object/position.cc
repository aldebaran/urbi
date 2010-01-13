/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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
      proto_add(proto ? proto : Object::proto);
    }

    Position::Position(value_type pos)
      : pos_(pos)
    {
      proto_add(proto ? proto : Object::proto);
    }


    Position::Position(libport::Symbol* f, unsigned int l, unsigned int c)
      : pos_(f, l, c)
    {
      proto_add(proto ? proto : Object::proto);
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
      case 0:
        pos_ = value_type();
        return;
      case 1:
        pos_ = type_check<Position>(args[0])->pos_;
        return;
      case 2:
        pos_ = value_type(0,
                          CxxConvert<unsigned>::to(args[0], 0),
                          CxxConvert<unsigned>::to(args[1], 1));
        return;
      case 3:
        libport::Symbol* f =
          new libport::Symbol(from_urbi<std::string>(args[0]));
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
    Position::operator ==(rPosition rhs) const
    {
      return pos_.line == rhs->pos_.line && pos_.column == rhs->pos_.column;
    }

    bool
    Position::operator <(rPosition rhs) const
    {
      return pos_.line < rhs->pos_.line
        || ( pos_.line == rhs->pos_.line && pos_.column < rhs->pos_.column);
    }

    /*-------------.
    | Operations.  |
    `-------------*/

    rPosition
    Position::operator - (const rFloat rhs) const
    {
      return new Position(pos_ - rhs);
    }

    rPosition
    Position::operator + (const rFloat rhs) const
    {
      return new Position(pos_ + rhs);
    }

    void
    Position::lines(const rFloat count)
    {
      pos_.lines(count);
    }

    void
    Position::columns(const rFloat count)
    {
      pos_.lines(count);
    }

    /*--------------.
    | Conversions.  |
    `--------------*/

    std::string
    Position::as_string() const
    {
      std::ostringstream o;
      o << pos_;
      return o.str();
    }

    /*-----------.
    | Accessor.  |
    `-----------*/

    rObject
    Position::file_get() const
    {
      if (!pos_.filename)
        return nil_class;
      return to_urbi(*pos_.filename);
    };

    void
    Position::file_set(rObject o)
    {
      if (!pos_.filename)
        delete pos_.filename;
      pos_.filename = 0;
      if (o != nil_class)
        pos_.filename =
          new libport::Symbol(from_urbi<std::string>(o));
    };

    /*-----------------.
    | Binding system.  |
    `-----------------*/

    void
    Position::initialize(CxxObject::Binder<Position>& bind)
    {
      bind(SYMBOL(EQ_EQ), &Position::operator ==);
      bind(SYMBOL(LT),
           (bool (Position::*)(rPosition rhs) const) &Position::operator <);
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
