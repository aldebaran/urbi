/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <sstream>
#include <object/format-info.hh>
#include <object/formatter.hh>
#include <urbi/object/symbols.hh>

namespace urbi
{
  namespace object
  {

    /*-----------------.
    | Implementation.  |
    `-----------------*/

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

    URBI_CXX_OBJECT_INIT(Formatter)
    {
# define DECLARE(Urbi, Cxx)               \
      bind(SYMBOL_(Urbi), &Formatter::Cxx)

      DECLARE(init, init);
      DECLARE(data, data_get);

# undef DECLARE

# define OPERATOR_PCT(Type)                                       \
      static_cast<std::string (Formatter::*)(const Type&) const>  \
      (&Formatter::operator%)                                     \

      bind(SYMBOL(PERCENT), OPERATOR_PCT(rObject));

# undef OPERATOR_PCT
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
        if (!str.empty())
        {
          data_->insertBack(to_urbi(str));
          str.erase();
        }
        rFormatInfo f = new FormatInfo();
        f->init(format.substr(cursor));
        data_->insertBack(f);
        cursor += f->pattern_get().size();
      }
      if (!str.empty())
        data_->insertBack(to_urbi(str));
    }

    std::string
    Formatter::operator%(const objects_type& args) const
    {
      size_t index = 0;
      size_t max = args.size();

      std::string res;
      foreach (const rObject& c, data_->value_get())
      {
        rObject str;
        if (c->as<FormatInfo>())
        {
          if (index < max)
            str = args[index++]->call("format", c);
          else
            RAISE("too few arguments for format");
        }
        else
          str = c;

        aver(str);
        if (!str->as<String>())
        {
          // Need this because operator<< is not const.
          std::stringstream o;
          o << *str;
          FRAISE("invalid argument for format: %s", o.str());
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
      {
        objects_type args;
        args << arg;
        return operator%(args);
      }
    }
  } // namespace object
}
