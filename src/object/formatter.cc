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
  Formatter::init(std::string format)
  {
    size_t cursor = 0;
    size_t end = format.size();
    std::string str("");
    while (cursor < end)
    {
      std::string buf = format.substr(cursor, format.find_first_of('%', cursor) - cursor);
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

  void
  Formatter::initialize(CxxObject::Binder<Formatter>& bind)
  {
    bind(SYMBOL(init), &Formatter::init);
    bind(SYMBOL(data), &Formatter::data_get);
  }

  rObject
  Formatter::proto_make()
  {
    return new Formatter();
  }

  URBI_CXX_OBJECT_REGISTER(Formatter);
} // namespace object
