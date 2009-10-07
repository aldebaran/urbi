/*
 * Copyright (C) 2009, Gostai S.A.S.
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

#include <object/file.hh>
#include <object/output-stream.hh>
#include <object/symbols.hh>

#include <runner/raise.hh>

namespace object
{
  OutputStream::OutputStream(int fd, bool own)
    : fd_(fd)
    , own_(own)
  {
    proto_add(proto ? proto : Object::proto);
  }

  OutputStream::OutputStream(rOutputStream model)
    : fd_(model->fd_)
    , own_(false)
  {
    proto_add(proto);
  }

  OutputStream::~OutputStream()
  {
    if (own_ && fd_ != -1)
      if (::close(fd_))
        RAISE(libport::strerror(errno));
  }

  void
  OutputStream::checkFD_() const
  {
    if (fd_ == -1)
      RAISE("stream is closed");
  }

  /*---------------.
  | Urbi methods.  |
  `---------------*/

  void OutputStream::init(rFile f)
  {
    libport::path path = f->value_get()->value_get();
    fd_ = open(path.to_string().c_str(),
               O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);

    if (fd_ < 0)
      FRAISE("cannot open file for writing: %s", path);
    own_ = true;
  }

  rOutputStream OutputStream::putByte(unsigned char c)
  {
    checkFD_();
    // FIXME: bufferize
    size_t size = write(fd_, &c, 1);
    assert_eq(size, 1u);
    (void)size;
    return this;
  }

  void OutputStream::flush()
  {
    checkFD_();
    // FIXME: nothing since not bufferized for now
  }

  rOutputStream
  OutputStream::put(rObject o)
  {
    checkFD_();
    std::string str = o->call(SYMBOL(asString))->as<String>()->value_get();
    size_t str_size = str.size();
    size_t size = write(fd_, str.c_str(), str_size);
    assert_eq(size, str_size);
    (void)size;
    return this;
  }

  void
  OutputStream::close()
  {
    checkFD_();
    if (::close(fd_))
      RAISE(libport::strerror(errno));
    fd_ = -1;
  }

  /*----------------.
  | Urbi bindings.  |
  `----------------*/

  rObject
  OutputStream::proto_make()
  {
    return new OutputStream(STDOUT_FILENO, false);
  }

  void
  OutputStream::initialize(CxxObject::Binder<OutputStream>& bind)
  {
    bind(SYMBOL(LT_LT), &OutputStream::put    );
    bind(SYMBOL(close), &OutputStream::close  );
    bind(SYMBOL(flush), &OutputStream::flush  );
    bind(SYMBOL(init),  &OutputStream::init   );
    bind(SYMBOL(put),   &OutputStream::putByte);
  }

  URBI_CXX_OBJECT_REGISTER(OutputStream);
}
