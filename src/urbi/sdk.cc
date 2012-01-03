/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cerrno>
#include <libport/read-stdin.hh>
#include <libport/exception.hh>

#include <urbi/sdk.hh>
#include <urbi/kernel/userver.hh>
#include <object/system.hh>
#include <runner/runner.hh>

namespace urbi
{
  object::rObject
  run(const std::string& code)
  {
    return object::eval(code);
  }

  object::rObject
  run_bg(const std::string& code)
  {
    return object::eval("detach({" + code + "});");
  }

  void
  yield()
  {
    ::kernel::runner().yield();
  }

  void
  yield_until(libport::utime_t t)
  {
    ::kernel::runner().yield_until(t);
  }

  void
  yield_for(libport::utime_t t)
  {
    ::kernel::runner().yield_for(t);
  }

  void
  yield_for_fd(int fd)
  {
    aver_le(0, fd);
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    // Do not block
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    // FIXME: Kinda busy loop
    int ret = 0;
    for (ret = select(fd + 1, &rfds, NULL, NULL, &tv);
         !ret;
         ret = select(fd + 1, &rfds, NULL, NULL, &tv))
    {
      yield_for(128000);
      FD_SET(fd, &rfds);
    }
    if (ret == -1)
      errnoabort("select");
  }

  std::string
  yield_for_read(int fd)
  {
    std::string res;
    try
    {
      // FIXME: Kinda busy loop.
      while (true)
      {
        res = libport::read_fd(fd);
        if (!res.empty())
          break;
        yield_for(128000);
      }
    }
    catch (const libport::Exception& e)
    {
    }
    return res;
  }
}
