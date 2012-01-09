/*
 * Copyright (C) 2005-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file kernel/uconnection.cc
/// \brief Implementation of UConnection.

#include <libport/config.h>
#include <libport/cstring>
#include <libport/cstdio>
#include <libport/cassert>
#include <sstream>
#include <iomanip>

#include <libport/foreach.hh>
#include <libport/format.hh>
#include <libport/ref-pt.hh>

#include <ast/nary.hh>
#include <ast/print.hh>

#include <urbi/kernel/userver.hh>
#include <urbi/kernel/uconnection.hh>

#include <urbi/uvalue.hh> // SYNCLINE_WRAP

#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/tag.hh>
#include <urbi/object/urbi-exception.hh>

#include <parser/uparser.hh>
#include <parser/transform.hh>

#include <binder/binder.hh>

#include <runner/sneaker.hh>
#include <runner/shell.hh>

namespace kernel
{

  UConnection::UConnection(UServer& server, size_t packetSize)
    : uerror_(USUCCESS)
    , closing_(false)
    , receiving_(false)
    , server_(server)
    , lobby_(new object::Lobby(this))
    , send_queue_(new queue_type(1024))
    , packet_size_(packetSize)
    , blocked_(false)
      // Initial state of the connection: unblocked, not receiving binary.
    , active_(true)
    , interactive_p_(true)
    , bytes_sent_(0)
    , bytes_received_(0)
    , stream_buffer_()
    , stream_(&stream_buffer_)
  {
    stream_.exceptions(std::ios::badbit);

    // Create the shell.
    shell_ = new runner::Shell(lobby_, server_.scheduler_get(), SYMBOL(shell), stream_);
    shell_->start_job();

    // Create the sneaker if it needs to be.
    dbg::create_sneaker_if_needed(lobby_, server_.scheduler_get());
  }

  UConnection::~UConnection()
  {
    close(); // Just in case we reached here without closing.
    shell_->terminate_now();
  }

  void
  UConnection::initialize()
  {
    received(SYNCLINE_WRAP("initialize(%s, %s)|;",
                             kernel::urbiserver->opt_banner_get(),
                             &kernel::urbiserver->ghost_connection_get()==this));
  }

  void
  UConnection::send(const char* buf, size_t len, const char* tag, bool flush_p)
  {
    if (tag)
    {
      if (*tag)
        send_queue(libport::format("[%08d:%s] ",
                                   server_.lastTime() / 1000L, tag));
      else
        send_queue(libport::format("[%08d] ",
                                   server_.lastTime() / 1000L));
    }
    if (buf)
    {
      send_queue(buf, len);
      UErrorValue res = error_;
      if (flush_p && res != UFAIL)
        flush();
      error_ = res;
    }
  }

  void
  UConnection::send_queue(const char* buf, size_t len)
  {
    if (!closing_)
      send_queue_->push(buf, len);
    error_ = USUCCESS;
  }

  void
  UConnection::continue_send()
  {
    if (closing_)
    {
      error_ = UFAIL;
      return;
    }
    blocked_ = false;	    // continue_send unblocks the connection.

    // nb of bytes to send.
    size_t toSend = std::min(packet_size_, send_queue_->size());

    if (!toSend)
    {
      error_ = USUCCESS;
      return;
    }

    if (const char* popData = send_queue_->peek(toSend))
    {
      int wasSent = effective_send(popData, toSend);
      bytes_sent_ += wasSent;
      // FIXME: This can never happen, as effective_send
      // returns a size_t which cannot be negative.
      if (wasSent < 0)
      {
        error_ = UFAIL;
        return;
      }
      else if (wasSent == 0 || send_queue_->pop(wasSent))
      {
        error_ = USUCCESS;
        return;
      }
    }

    error_ = UFAIL;
  }

  void
  UConnection::received(const char* buffer, size_t length)
  {
    bytes_received_ += length;
    stream_buffer_.post_data(buffer, length);
  }

  bool
  UConnection::has_pending_command() const
  {
    return shell_->pending_command_get();
  }

  void
  UConnection::drop_pending_commands()
  {
    shell_->pending_commands_clear();
  }

  bool
  UConnection::send_queue_empty() const
  {
    return send_queue_->empty();
  }

  bool
  UConnection::interactive_p() const
  {
    return interactive_p_;
  }

  void
  UConnection::interactive_p(bool b)
  {
    interactive_p_ = b;
  }

  static void shell_stop(runner::rShell s)
  {
    s->stop();
  }

  void UConnection::close()
  {
    if (closing_)
      return;
    closing_ = true;
    close_();
    stream_buffer_.close();
    kernel::server().schedule_fast(boost::bind(&shell_stop,
                                               shell_get()));
  }
}
