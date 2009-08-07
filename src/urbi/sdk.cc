#include <urbi/sdk.hh>
#include <kernel/userver.hh>
#include <runner/runner.hh>

namespace urbi
{
  void
  yield()
  {
    ::kernel::urbiserver->getCurrentRunner().yield();
  }

  void
  yield_until(libport::utime_t t)
  {
    ::kernel::urbiserver->getCurrentRunner().yield_until(t);
  }

  void
  yield_for(libport::utime_t t)
  {
    ::kernel::urbiserver->getCurrentRunner().yield_until(libport::utime() + t);
  }

  void
  yield_for_fd(int fd)
  {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    // Do not block
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int ret = 0;

    // FIXME: Kinda busy loop
    for (ret = select(fd + 1, &rfds, NULL, NULL, &tv);
         !ret;
         ret = select(fd + 1, &rfds, NULL, NULL, &tv))
    {
      if (ret == -1)
      {
        libport::perror("select");
        std::abort();
      }
      yield_for(128000);
      FD_SET(fd, &rfds);
    }
  }
}
