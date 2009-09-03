/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

#include <boost/format.hpp>

#include <object/file.hh>
#include <object/input-stream.hh>
#include <object/symbols.hh>

#include <urbi/sdk.hh>
#include <runner/raise.hh>

namespace object
{
  /*---------------------------.
  | Construction / Destruction |
  `---------------------------*/

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
    if (own_)
      if (close(fd_))
        RAISE(libport::strerror(errno));
  }

  /*------------.
  | Data access |
  `------------*/

  int
  InputStream::_get()
  {
    if (pos_ == size_)
      if (!_getBuffer())
        return -1;
    return buffer_[pos_++];
  }

  bool
  InputStream::_getBuffer()
  {
    urbi::yield_for_fd(fd_);
    size_ = read(fd_, &buffer_, 1);
    pos_ = 0;
    return size_;
  }

  std::string
  InputStream::_getSeparator(char sep, bool incl, bool& ok)
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
      if (!_getBuffer())
        return res;
    }
  }

  /*-------------.
  | Urbi methods |
  `-------------*/

  void
  InputStream::init(rFile f)
  {
    libport::path path = f->value_get()->value_get();
    fd_ = open(path.to_string().c_str(), O_RDONLY);
    own_ = true;
    if (fd_ < 0)
    {
      boost::format fmt("Unable to open file for reading: %s");
      fd_ = 0;
      RAISE(str(fmt % path));
    }
  }

  rObject
  InputStream::get()
  {
    int res = _get();
    if (res == -1)
      return 0;
    return to_urbi(res);
  }

  rObject
  InputStream::getChar()
  {
    int res = _get();
    if (res == -1)
      return 0;
    return to_urbi(std::string(1, res));
  }

  rObject
  InputStream::getLine()
  {
    bool ok;
    std::string res = _getSeparator('\n', false, ok);
    if (ok)
      return to_urbi(res);
    return 0;
  }

  /*--------------.
  | Urbi bindings |
  `--------------*/

  rObject
  InputStream::proto_make()
  {
    return new InputStream(STDIN_FILENO, false);
  }

  void
  InputStream::initialize(object::CxxObject::Binder<object::InputStream>& bind)
  {
    bind(SYMBOL(get), &InputStream::get);
    bind(SYMBOL(getChar), &InputStream::getChar);
    bind(SYMBOL(getLine), &InputStream::getLine);
    bind(SYMBOL(init), &InputStream::init);
  }

  URBI_CXX_OBJECT_REGISTER(InputStream);
}
