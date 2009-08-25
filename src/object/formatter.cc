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
  Formatter::format_list(const objects_type& args) const
  {
    std::string res("");
    const objects_type& l = args[0]->as<List>()->value_get();
    size_t index = 0;
    size_t max = l.size();

    foreach(const rObject& c, data_->value_get())
    {
      if (c->is_a<FormatInfo>())
      {
        if (index >= max)
          RAISE("too few argument for format");
        else
          res += l[index++]->call("format", c)->as<String>()->value_get();
      }
      else
        res += c->as<String>()->value_get();
    }
    if (index < max)
      RAISE("too many arguments for format");
    return res;
  }

  std::string
  Formatter::format_non_list(const rObject& arg) const
  {
    libport::ReservedVector<rObject, 8> args(1, arg);

    return format_list(args);
  }

  OVERLOAD_TYPE(overload_format, 1, 0,
                any-type, &Formatter::format_non_list,
                List, &Formatter::format_list);

  void
  Formatter::initialize(CxxObject::Binder<Formatter>& bind)
  {
    bind(SYMBOL(init), &Formatter::init);
    bind(SYMBOL(data), &Formatter::data_get);
    bind(SYMBOL(PERCENT), overload_format);
  }

  rObject
  Formatter::proto_make()
  {
    return new Formatter();
  }

  URBI_CXX_OBJECT_REGISTER(Formatter);
} // namespace object
