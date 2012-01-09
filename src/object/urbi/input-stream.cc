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
#include <libport/fcntl.h>
#include <fstream>
#include <libport/sys/stat.h>
#include <libport/sys/types.h>

#include <libport/format.hh>

#include <urbi/object/file.hh>
#include <object/urbi/input-stream.hh>
#include <object/urbi/output-stream.hh>
#include <urbi/object/symbols.hh>

#include <urbi/sdk.hh>
#include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {
    /*-----------------------------.
    | Construction / Destruction.  |
    `-----------------------------*/

    InputStream::InputStream(int fd, bool own)
      : Stream(fd, own)
      , pos_(0)
      , size_(0)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    InputStream::InputStream(rInputStream model)
      : Stream(model)
      , pos_(0)
      , size_(0)
    {
      proto_add(model);
    }

    URBI_CXX_OBJECT_INIT(InputStream)
      : Stream(STDIN_FILENO, false)
      , pos_(0)
      , size_(0)
    {
      proto_add(Stream::proto);

# define DECLARE(Name, Cxx)                     \
      bind(SYMBOL_(Name), &InputStream::Cxx)

      DECLARE(get,     get);
      DECLARE(getChar, getChar);
      DECLARE(getLine, getLine);
      DECLARE(init,    init);

# undef DECLARE
    }

    InputStream::~InputStream()
    {
    }

    /*--------------.
    | Data access.  |
    `--------------*/

    int
    InputStream::get_()
    {
      check();
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
      check();
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
      open(f, O_RDONLY, 0, "cannot open file for reading");
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
  }
}
