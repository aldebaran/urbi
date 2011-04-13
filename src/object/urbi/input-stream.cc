/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
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

#include <urbi/object/event.hh>
#include <urbi/object/file.hh>
#include <object/urbi/input-stream.hh>
#include <object/urbi/output-stream.hh>
#include <object/urbi/symbols.hh>

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
      , sem_(new Semaphore())
    {
      proto_add(proto ? rObject(proto) : Object::proto);
      socket_->slot_set(SYMBOL(receive),
        new Primitive(boost::bind(&InputStream::receive_, this, _1)));
      socket_->slot_get(SYMBOL(disconnected))->as<Event>()
      ->onEvent(boost::bind(&InputStream::onError_, this, _1));
    }

    InputStream::InputStream(rInputStream model)
      : Stream(model)
      , pos_(0)
      , size_(0)
      , sem_(new Semaphore())
    {
      proto_add(model);
    }

    URBI_CXX_OBJECT_INIT(InputStream)
#if defined WIN32
      : Stream()
#else
      : Stream(STDIN_FILENO, false)
#endif
      , pos_(0)
      , size_(0)
      , sem_(new Semaphore())
    {
      proto_add(Stream::proto);
      BIND(get);
      BIND(getChar);
      BIND(getLine);
      BIND(init);
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
      check();
      assert_eq(pos_, size_);
      buffer_.clear();
      socket_->readOnce();
      waiting_ = true;
      sem_->acquire();
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
      open(f, libport::Socket::READ);
      socket_->slot_set(SYMBOL(receive),
                 new Primitive(boost::bind(&InputStream::receive_, this, _1)));
      socket_->slot_get(SYMBOL(disconnected))->as<Event>()
        ->onEvent(boost::bind(&InputStream::onError_, this, _1));
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

    rObject
    InputStream::receive_(objects_type args)
    {
      if (args.size() != 2)
        runner::raise_arity_error(args.size(), 1);
      buffer_ += args[1]->as<String>()->value_get();
      if (waiting_)
        sem_->release();
      return void_class;
    }

    rObject
    InputStream::onError_(objects_type)
    {
      if (waiting_)
        sem_->release();
      return void_class;
    }
  }
}
