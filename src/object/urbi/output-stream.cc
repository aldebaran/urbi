/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cerrno>
#include <libport/sys/types.h>
#include <libport/sys/stat.h>
#include <libport/fcntl.h>

#include <fstream>

#include <libport/format.hh>

#include <urbi/object/file.hh>
#include <object/urbi/input-stream.hh>
#include <object/urbi/output-stream.hh>
#include <urbi/object/symbols.hh>

#include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {
    OutputStream::OutputStream(int fd, bool own)
      : Stream(fd, own)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    OutputStream::OutputStream(rOutputStream model)
      : Stream(model)
    {
      proto_add(proto);
    }

    URBI_CXX_OBJECT_INIT(OutputStream)
      : Stream()
    {
      proto_add(Stream::proto);
      // Spurious spaces to avoid static check on Symbol uses.
      bind(libport::Symbol( "<<" ), &OutputStream::put);

      BIND(flush);
      BIND(init);
      BIND(put, putByte);
    }

    OutputStream::~OutputStream()
    {
    }

    /*---------------.
    | Urbi methods.  |
    `---------------*/

    void OutputStream::init(rFile f)
    {
      open(f, libport::Socket::USE_FLAGS, O_WRONLY | O_APPEND | O_CREAT,
           S_IRWXU);
    }

    rOutputStream OutputStream::putByte(unsigned char c)
    {
      check();
      // FIXME: bufferize
      socket_->write(std::string((char*)&c, 1));
      return this;
    }

    void OutputStream::flush()
    {
      check();
      // FIXME: nothing since not bufferized for now
    }

    rOutputStream
    OutputStream::put(rObject o)
    {
      check();
      std::string str = o->as_string();
      socket_->write(str);
      return this;
    }
  }
}
