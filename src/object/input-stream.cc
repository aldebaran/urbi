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
#include <libport/fcntl.h>
#include <fstream>
#include <libport/sys/stat.h>
#include <libport/sys/types.h>

#include <libport/format.hh>

#include <object/file.hh>
#include <object/input-stream.hh>
#include <object/symbols.hh>

#include <urbi/sdk.hh>
#include <runner/raise.hh>

namespace object
{
  /*-----------------------------.
  | Construction / Destruction.  |
  `-----------------------------*/

  InputStream::InputStream(int fd, bool own)
    : fd_(fd)
    , own_(own)
    , pos_(0)
    , size_(0)
  {
    proto_add(proto ? proto : Object::proto);
  }

  InputStream::InputStream(rInputStream model)
    : fd_(model->fd_)
    , own_(false)
    , pos_(0)
    , size_(0)
  {
    assert(model);
    proto_add(model);
  }

  InputStream::~InputStream()
  {
    if (own_ && close(fd_))
      RAISE(libport::strerror(errno));
  }

  /*--------------.
  | Data access.  |
  `--------------*/

  int
  InputStream::get_()
  {
    if (pos_ == size_ && !getBuffer_())
      return -1;
    return buffer_[pos_++];
  }

  bool
  InputStream::getBuffer_()
  {
    assert_eq(pos_, size_);
    buffer_ = urbi::yield_for_read(fd_);
    size_ = buffer_.size();
    pos_ = 0;
    return size_;
  }

  std::string
  InputStream::getSeparator_(char sep, bool incl, bool& ok)
  {
    ok = false;
    std::string res;

    while (true)
    {
      for (; pos_ < size_; ++pos_)
      {
        ok = true;
        if (buffer_[pos_] == sep)
        {
          if (incl)
            res += sep;
          pos_++;
          return res;
        }
        else
          res += buffer_[pos_];
      }
      if (!getBuffer_())
        return res;
    }
  }

  /*--------------.
  | Urbi methods. |
  `--------------*/

  void
  InputStream::init(rFile f)
  {
    libport::path path = f->value_get()->value_get();
    fd_ = open(path.to_string().c_str(), O_RDONLY);
    if (fd_ < 0)
      FRAISE("cannot open file for reading: %s", path);
    own_ = true;
  }

  rObject
  InputStream::get()
  {
    int res = get_();
    if (res == -1)
      return 0;
    return to_urbi(res);
  }

  rObject
  InputStream::getChar()
  {
    int res = get_();
    if (res == -1)
      return 0;
    return to_urbi(std::string(1, res));
  }

  rObject
  InputStream::getLine()
  {
    bool ok;
    std::string res = getSeparator_('\n', false, ok);
    if (ok)
      return to_urbi(res);
    return 0;
  }

  /*----------------.
  | Urbi bindings.  |
  `----------------*/

  rObject
  InputStream::proto_make()
  {
    return new InputStream(STDIN_FILENO, false);
  }

  void
  InputStream::initialize(CxxObject::Binder<InputStream>& bind)
  {
    bind(SYMBOL(get), &InputStream::get);
    bind(SYMBOL(getChar), &InputStream::getChar);
    bind(SYMBOL(getLine), &InputStream::getLine);
    bind(SYMBOL(init), &InputStream::init);
  }

  URBI_CXX_OBJECT_REGISTER(InputStream);
}
