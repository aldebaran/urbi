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

    /* embeded in main lib: moved to register.cc
    URBI_CXX_OBJECT_REGISTER(Stream);
    URBI_CXX_OBJECT_REGISTER(InputStream);
    URBI_CXX_OBJECT_REGISTER(OutputStream);
    */


    /*-----------------------------.
    | Construction / Destruction.  |
    `-----------------------------*/

    Stream::Stream()
    {
    }

    Stream::Stream(int fd, bool own)
    {
      open_(fd, own);
    }

    Stream::Stream(rStream model)
    {
      if (model->socket_)
        FRAISE("cannot clone opened Streams");
    }

    Stream::~Stream()
    {
      // When destructed, close the file descriptor owned by this class if
      // this is not already done by the user.
      if (!closed())
        close();
    }

    void
    Stream::new_socket_()
    {
      if (socket_ && socket_->isConnected())
        socket_->close();
      socket_ = new Socket();
      socket_->init();
      socket_->setAutoRead(false);
    }

    void
    Stream::open(rFile f, libport::Socket::OpenMode mode, int extraFlags,
                 int createMode)
    {
      new_socket_();
      libport::path path = f->value_get()->value_get();
      socket_->open_file(path.to_string(), mode, extraFlags, createMode);
    }

    void
    Stream::open_(int fd, bool own)
    {
#if defined F_GETFL
      // Check that we have a valid fd.
      if (fcntl(fd, F_GETFL) == -1)
        FRAISE("cannot create stream: %s", libport::strerror(errno));
#endif

      int fd2 = fd;
      if (!own)
      {
        fd2 = dup(fd);
        if (fd2 == -1)
          FRAISE("cannot create stream: %s", libport::strerror(errno));
      }
      new_socket_();
      socket_->setNativeFD(fd2);
    }

    void
    Stream::check() const
    {
      if (closed())
        RAISE("stream is closed");
    }

    bool
    Stream::closed() const
    {
      return !socket_ || !socket_->isConnected();
    }

    void
    Stream::close()
    {
      check();
      socket_->close();
      socket_ = 0;
    }

    URBI_CXX_OBJECT_INIT(Stream)
    {
      BIND(close);
      BINDG(closed);
    }
  }
}
