/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <stdexcept>
#include <libport/debug.hh>

#include <urbi/details.hh>

GD_CATEGORY(Urbi.Details);

#define FRAISE(Format, ...)                                     \
  throw std::runtime_error(libport::format(Format, __VA_ARGS__))

namespace urbi
{
  namespace uobjects
  {

    StringPair
    uname_split(const std::string& name)
    {
      size_t p = name.find_last_of(".");
      if (p == name.npos)
      {
        GD_FWARN("invalid argument to split_name: %s", name);
        return StringPair(name, "");
      }
      std::string oname = name.substr(0, p);
      std::string slot = name.substr(p + 1, name.npos);
      return StringPair(oname, slot);
    }

    StringPair
    uname_xsplit(const std::string& name, const std::string& ctx)
    {
      StringPair res = uname_split(name);
      if (res.second.empty())
        FRAISE("%s%sinvalid argument: %s",
               ctx, ctx.empty() ? "" : ": ", name);
      return res;
    }

  }
}
