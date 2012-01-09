/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cerrno>
#include <libport/fcntl.h>
#include <fstream>
#include <libport/sys/stat.h>
#include <libport/sys/types.h>
#include <libport/format.hh>

#include <urbi/object/file.hh>
#include <object/urbi/stream.hh>
#include <object/urbi/input-stream.hh>
#include <object/urbi/output-stream.hh>
#include <urbi/object/symbols.hh>

#include <urbi/sdk.hh>
#include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {

    /*--------------.
    | Registering.  |
    `--------------*/

    URBI_CXX_OBJECT_REGISTER(Stream);
    URBI_CXX_OBJECT_REGISTER(InputStream);
    URBI_CXX_OBJECT_REGISTER(OutputStream);


    /*-----------------------------.
    | Construction / Destruction.  |
    `-----------------------------*/

    Stream::Stream(int fd, bool own)
      : fd_(fd)
      , own_(own)
    {}

    Stream::Stream(rStream model)
      : fd_(model->fd_)
      , own_(false)
    {
      aver(model);
    }

    Stream::~Stream()
    {
      // Don't play with check here (so don't simplify into calling
      // close()), since it might RAISE, which catches "this", which
      // is being destroyed => Boom.
      if (own_ && fd_ != -1)
        if (::close(fd_))
          // FIXME: RAISING is not a good thing here.
          RAISE(libport::strerror(errno));
    }

    void
    Stream::open(rFile f, int flags, mode_t mode, const char* error)
    {
      libport::path path = f->value_get()->value_get();
      fd_ = ::open(path.to_string().c_str(), flags, mode);
      if (fd_ < 0)
        FRAISE("%s: %s", error, path);
      own_ = true;
    }

    void
    Stream::check() const
    {
      if (fd_ == -1)
        RAISE("stream is closed");
    }

    void
    Stream::close()
    {
      check();
      if (::close(fd_))
        RAISE(libport::strerror(errno));
      fd_ = -1;
    }

    URBI_CXX_OBJECT_INIT(Stream)
    {
      bind(SYMBOL(close), &Stream::close);
    }
  }
}
