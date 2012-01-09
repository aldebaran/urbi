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
      : Stream(STDOUT_FILENO, false)
    {
      proto_add(Stream::proto);
      // Spurious spaces to avoid static check on Symbol uses.
      bind(libport::Symbol( "<<" ), &OutputStream::put);

# define DECLARE(Name, Cxx)                     \
      bind(SYMBOL_(Name), &OutputStream::Cxx)

      DECLARE(flush, flush);
      DECLARE(init,  init);
      DECLARE(put,   putByte);

# undef DECLARE
    }

    OutputStream::~OutputStream()
    {
    }

    /*---------------.
    | Urbi methods.  |
    `---------------*/

    void OutputStream::init(rFile f)
    {
      open(f, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU,
           "cannot open file for writing");
    }

    rOutputStream OutputStream::putByte(unsigned char c)
    {
      check();
      // FIXME: bufferize
      size_t size = write(fd_, &c, 1);
      assert_eq(size, 1u);
      LIBPORT_USE(size);
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
      size_t str_size = str.size();
      size_t size = write(fd_, str.c_str(), str_size);
      assert_eq(size, str_size);
      LIBPORT_USE(size);
      return this;
    }
  }
}
