/*! \file urbi/uclient.hh
****************************************************************************
 * Definition of the URBI interface class
 *
 * Copyright (C) 2004, 2006, 2007, 2008, 2009 Jean-Christophe Baillie.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**********************************************************************/

#ifndef URBI_UCLIENT_HH
# define URBI_UCLIENT_HH

# include <libport/asio.hh>
# include <libport/pthread.h>
# include <libport/semaphore.hh>
# include <libport/utime.hh>

# include <urbi/uabstractclient.hh>


namespace urbi
{
  ///Linux implementation of UAbstractClient.
  /*! This implementation creates a thread for each instance of UClient, which
    listens on the associated socket.
  */
  class URBI_SDK_API UClient
    : public UAbstractClient
    , protected libport::Socket
  {
  public:
    using UAbstractClient::DEFAULT_HOST;

    /// Construction options.
    struct options
    {
      /// Backward compatibility with the previous UClient::UClient
      /// interface.  Don't make it "explicit" so that we can call
      /// "UClient(host, port)" and have the expected "server ==
      /// false".
      ///
      /// start defaults to true, for backward compatibility too.
      options(bool server = false);
# define UCLIENT_OPTION(Type, Name)    \
    public:                                     \
      options& Name(Type b);          \
      Type Name() const;                        \
    private:                                    \
      Type Name ## _;

      /// Whether in server mode.
      UCLIENT_OPTION(bool, server);
      /// Whether the socket autostarts.
      UCLIENT_OPTION(bool, start);
    };

# define UCLIENT_OPTION_IMPL(Class, Type, Name) \
  Class::options&                             \
  Class::options::Name(Type v)                \
  {                                             \
    Name ## _ = v;                              \
    return *this;                               \
  }                                             \
                                                \
  Type                                          \
  Class::options::Name() const                \
  {                                             \
    return Name ## _;                           \
  }

    /// \param opt  options: whether server, whether autostart.
    UClient(const std::string& host = DEFAULT_HOST,
            unsigned port = URBI_PORT,
	    size_t buflen = URBI_BUFLEN,
	    const options& opt = options());

    virtual ~UClient();

    /// Bounce to listen or connect, depending whether server mode.
    error_type start();
  protected:
    virtual error_type onClose();

  public:
    int closeUClient ();

    virtual void printf(const char * format, ...);
    virtual unsigned int getCurrentTime() const;

    UCallbackAction pong(const UMessage& msg);

    /// Activate KeepAlive functionality.
    /// \param pingInterval  is in milliseconds.
    /// \param pongTimeout   is in milliseconds.
    virtual void setKeepAliveCheck(unsigned pingInterval,
                                   unsigned pongTimeout);

    using UAbstractClient::send;

  protected:
    virtual int effectiveSend(const void* buffer, size_t size);

    libport::Socket* mySocketFactory();

    virtual void onConnect();

    virtual void onError(boost::system::error_code erc);

    virtual int onRead(const void*, size_t length);

  protected:
    /// Delay (in microseconds) without activity to check if the
    /// connection is yet available.
    libport::utime_t ping_interval_;

    /// Delay (in microseconds) of timeout to wait 'PONG'.
    libport::utime_t pong_timeout_;

    /// Send timeout error.
    void pongTimeout();

    void sendPing();

    libport::utime_t ping_sent_;

    libport::AsyncCallHandler pong_timeout_handler_;

    libport::Semaphore ping_sem_;

  private:
    /// Wrapper around Socket::connect.
    /// Client mode.
    error_type connect_();
    /// Wrapper around Socket::listen.
    /// Server mode.
    error_type listen_();
  };

} // namespace urbi
#endif
