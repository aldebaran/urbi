#include <object/format-info.hh>
#include <object/formatter.hh>
#include <object/symbols.hh>

namespace object
{
  Formatter::Formatter()
    : data_(new List)
  {
    proto_add(Object::proto);
  }

  Formatter::Formatter(rFormatter model)
    : data_(new List)
  {
    proto_add(model);
  }

  void
  Formatter::init(const std::string& format)
  {
    size_t cursor = 0;
    size_t end = format.size();
    std::string str("");
    while (cursor < end)
    {
      std::string buf =
        format.substr(cursor, format.find_first_of('%', cursor) - cursor);
      str += buf;
      cursor += buf.size();
      if (cursor == end)
        break;
      if (cursor < end - 1 && format[cursor + 1] == '%')
      {
        str += '%';
        cursor += 2;
        continue;
      }
      if (str.size())
      {
        data_->insertBack(to_urbi(str));
        str.erase();
      }
      rFormatInfo f = new FormatInfo();
      f->init(format.substr(cursor));
      data_->insertBack(f);
      cursor += f->pattern_get().size();
    }
    if (str.size())
      data_->insertBack(to_urbi(str));
  }

  std::string
  Formatter::operator%(const objects_type& args) const
  {
    size_t index = 0;
    size_t max = args.size();

    std::string res;
    foreach(const rObject& c, data_->value_get())
    {
      rObject str;
      if (c->is_a<FormatInfo>())
      {
        if (index < max)
          str = args[index++]->call("format", c);
        else
          RAISE("too few arguments for format");
      }
      else
        str = c;

      assert(str);
      if (!str->is_a<String>())
      {
        std::stringstream o;
        o << *str;
        RAISE(libport::format("invalid argument for format: %s", o.str()));
      }
      res += str->as<String>()->value_get();
    }
    if (index < max)
      RAISE("too many arguments for format");
    return res;
  }

  std::string
  Formatter::operator%(const rObject& arg) const
  {
    if (rList l = arg->as<List>())
      return operator%(l->value_get());
    else
      return operator%(objects_type(1, arg));
  }

#define OPERATOR_PCT(Type)                                      \
  static_cast<std::string (Formatter::*)(const Type&) const>    \
             (&Formatter::operator%)
  void
  Formatter::initialize(CxxObject::Binder<Formatter>& bind)
  {
    bind(SYMBOL(init),    &Formatter::init);
    bind(SYMBOL(data),    &Formatter::data_get);
    bind(SYMBOL(PERCENT), OPERATOR_PCT(rObject));
  }
#undef OPERATOR_PCT

  rObject
  Formatter::proto_make()
  {
    return new Formatter();
  }

  URBI_CXX_OBJECT_REGISTER(Formatter);
} // namespace object
