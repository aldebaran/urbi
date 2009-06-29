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
    , private libport::Socket
  {
  public:
    using UAbstractClient::DEFAULT_HOST;
    UClient(const std::string& host = DEFAULT_HOST,
            unsigned port = URBI_PORT,
	    size_t buflen = URBI_BUFLEN,
	    bool server = false);

    virtual ~UClient();

    //! For compatibility with older versions of the library
    void start() {}

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

    /// True if waiting 'PONG'.
    bool waitingPong;

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
