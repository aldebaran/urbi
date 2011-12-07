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
      init_socket_();
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
      : Stream()
      , pos_(0)
      , size_(0)
      , sem_(new Semaphore())
    {
      proto_add(Stream::proto);
      BIND(content);
      BIND(get);
      BIND(getChar);
      BIND(getLine);
      BIND(init);
    }

    InputStream::~InputStream()
    {
      // Unsubscribe before anything else when we're being
      // destroyed. Otherwise, when Stream::~Stream closes the socket,
      // the event will call back our onError_ method, which will
      // access our already destroyed members and make the whole thing
      // die painfully.
      if (on_error_subscription_)
        on_error_subscription_->stop();
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

      do
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
      }
      while (getBuffer_());

      return res;
    }


    void
    InputStream::init_socket_()
    {
      aver(socket_);
      socket_
        ->slot_set_value
        (SYMBOL(receive),
         new Primitive(boost::bind(&InputStream::receive_, this, _1)));

      on_error_subscription_ =
        socket_
        ->slot_get(SYMBOL(disconnected))
        ->as<Event>()
        ->onEvent(boost::bind(&InputStream::onError_, this, _1));
    }

    /*--------------.
    | Urbi methods. |
    `--------------*/

    boost::optional<std::string>
    InputStream::content()
    {
      check();
      std::string res;

      do
      {
        if (pos_ != size_)
        {
          // We have a 512 byte buffer, the data could come in several cycles.
          res.append(buffer_, pos_, size_ - pos_);
          // We read the whole buffer.
          pos_ = size_;
        }
      }
      while (getBuffer_());

      return res;
    }

    void
    InputStream::init(rFile f)
    {
      open(f, libport::Socket::READ);
      init_socket_();
    }

    rObject
    InputStream::get()
    {
      int res = get_();
      if (res == -1)
        return 0;
      return to_urbi(res);
    }

    boost::optional<std::string>
    InputStream::getChar()
    {
      int res = get_();
      if (res == -1)
        return 0;
      return std::string(1, res);
    }

    boost::optional<std::string>
    InputStream::getLine()
    {
      bool ok;
      std::string res = getSeparator_('\n', false, ok);
      if (ok)
        return res;
      return 0;
    }

    rObject
    InputStream::receive_(objects_type args)
    {
      check_arg_count(args, 1);
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
