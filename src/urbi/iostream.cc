/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cstdio>
#include <libport/cstdlib>
#include <libport/debug.hh>
#include <libport/escape.hh>

#include <urbi/kernel/userver.hh>
#include <urbi/sdk.hh>
#include <urbi/iostream.hh>

GD_CATEGORY(Urbi.StreamBuffer);

namespace urbi
{
  static const size_t chunk_size = LIBPORT_BUFSIZ;

  StreamBuffer::Buffer::Buffer()
    : buffer(reinterpret_cast<char*>(malloc(chunk_size)))
    , used(0)
    , size(chunk_size)
  {
    aver(buffer);
  }

  StreamBuffer::StreamBuffer()
    : buffer1_()
    , buffer2_()
    , buffer_read_(&buffer1_)
    , buffer_write_(&buffer2_)
    , frozen_(0)
    , close_(false)
  {
    GD_INFO_TRACE("new StreamBuffer.");
    setg(buffer_read_->buffer, buffer_read_->buffer, buffer_read_->buffer);
  }

  int
  StreamBuffer::underflow()
  {
    GD_PUSH_TRACE("requesting new data.");
    if (buffer_write_->used == 0)
    {
      if (close_)
      {
        GD_INFO_DEBUG("closed, return EOF.");
        return EOF;
      }
      frozen_ = &::kernel::runner();
      GD_FINFO_DEBUG("no data, going to sleep: %x.", frozen_.get());
      frozen_->frozen_set(true);
      yield();
      if (close_)
      {
        GD_INFO_DEBUG("closed, woken up, return EOF.");
        return EOF;
      }
      aver(buffer_write_->used);
      GD_FINFO_DEBUG("%s bytes available, woken up, swapping buffers.",
                     buffer_write_->used);
    }
    else
      GD_FINFO_DEBUG("%s bytes available, swapping buffers.",
                     buffer_write_->used);
    std::swap(buffer_read_, buffer_write_);
    setg(buffer_read_->buffer,
         buffer_read_->buffer, buffer_read_->buffer + buffer_read_->used);
    buffer_write_->used = 0;
    GD_FINFO_DUMP("available data: \"%x\".",
                  libport::escape(std::string(buffer_read_->buffer,
                                              buffer_read_->used)));
    int res = static_cast<unsigned char>(buffer_read_->buffer[0]);
    GD_FINFO_DUMP("return: %x.", res);
    return res;
  }

  void
  StreamBuffer::post_data(const std::string& data)
  {
    aver(!close_);
    post_data(data.c_str(), data.size());
  }

  void
  StreamBuffer::post_data(const char* data, size_t size)
  {
    if (size == 0)
      return;

    GD_FPUSH_TRACE("posting %s bytes of data.", size);
    size_t needed = buffer_write_->used + size;
    if (needed > buffer_write_->size)
    {
      size_t size = ((needed - 1) / chunk_size + 1) * chunk_size;
      GD_FINFO_DEBUG("growing buffer from %s to %s bytes.",
                     buffer_write_->size, size);
      buffer_write_->size = size;
      buffer_write_->buffer =
        reinterpret_cast<char*>(realloc(buffer_write_->buffer, size));
    }
    memcpy(buffer_write_->buffer + buffer_write_->used, data, size);
    buffer_write_->used = needed;
    wake_up_();
  }

  int StreamBuffer::overflow(int)
  {
    GD_FABORT("%s: not implemented.", __FUNCTION__);
  }

  int
  StreamBuffer::sync()
  {
    GD_FABORT("%s: not implemented.", __FUNCTION__);
  }

  void
  StreamBuffer::close()
  {
    GD_INFO_TRACE("closing stream.");
    close_ = true;
    wake_up_();
  }

  void
  StreamBuffer::wake_up_()
  {
    if (frozen_)
    {
      GD_FINFO_DEBUG("waking the frozen job up: %x.", frozen_.get());
      frozen_->frozen_set(false);
      frozen_ = 0;
    }
  }
}
