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
      : data_(0)
    {
      BIND(init);
      BINDG(data, data_get);
      BIND(PERCENT, operator%, std::string, (const rObject&) const);
    }

    void
    Formatter::init(const std::string& format)
    {
      size_t cursor = 0;
      size_t end = format.size();
      std::string str;
      // Consistency check: all the FormatInfos must be positional, or
      // non-positional.  Keep a pointer to the later FormatInfo in
      // order to compare to it.
      FormatInfo* prev = 0;
      while (cursor < end)
      {
        // Fetch non-format string prefix.
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
        // Fetch format string.
        rFormatInfo f = new FormatInfo();
        f->init_(format.substr(cursor), false);
        if (prev
            && ((prev->rank_get() == 0 && f->rank_get() != 0)
                || (f->rank_get() == 0 && prev->rank_get() != 0)))
          FRAISE("format: cannot mix positional and "
                 "non-positional arguments: %s vs. %s",
                 *prev, *f);
        data_->insertBack(f);
        cursor += f->pattern_get().size();
        prev = f;
      }
      if (!str.empty())
        data_->insertBack(to_urbi(str));
    }

    std::string
    Formatter::operator%(const objects_type& args) const
    {
      size_t index = 0;
      size_t max = args.size();
      // Final argument used, to check that we used them all.
      size_t final = 0;
      std::string res;
      foreach (const rObject& c, data_->value_get())
      {
        rObject str;
        if (rFormatInfo f = c->as<FormatInfo>())
        {
          size_t i = f->rank_get() ? f->rank_get() - 1 : index++;
          if (i < max)
            str = args[i]->call("format", c);
          else
            RAISE("format: too few arguments");
          final = std::max(final, i);
        }
        else
          str = c;

        aver(str);
        if (!str->as<String>())
        {
          // Need this because operator<< is not const.
          std::stringstream o;
          o << *str;
          FRAISE("format: invalid argument: %s", o.str());
        }
        res += str->as<String>()->value_get();
      }
      if (max && final < max - 1)
        RAISE("format: too many arguments");
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
