/*! \file urbi/uclient.hh
****************************************************************************
 * Definition of the URBI interface class
 *
 * Copyright (C) 2004, 2006, 2007, 2008 Jean-Christophe Baillie.
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

# include <libport/semaphore.hh>
# include <urbi/uabstractclient.hh>


namespace urbi
{
  ///Linux implementation of UAbstractClient.
  /*! This implementation creates a thread for each instance of UClient, which
    listens on the associated socket.
  */
  class URBI_SDK_API UClient: public UAbstractClient
  {
  public:
    UClient(const std::string& host, unsigned port = URBI_PORT,
	    size_t buflen = URBI_BUFLEN,
	    bool server = false,
	    int semListenInc = 1);

    virtual ~UClient();

    int closeUClient ();

    //! For compatibility with older versions of the library
    void start() {}

    virtual void printf(const char * format, ...);
    virtual unsigned int getCurrentTime() const;

    //! For internal use.
    void acceptThread();
    //! For internal use.
    void listenThread();
    UCallbackAction pong(const UMessage& msg);


    /// Active KeepAlive functionality
    virtual void setKeepAliveCheck(const unsigned pingInterval,
                                   const unsigned pongTimeout);

  protected:
    virtual int effectiveSend(const void* buffer, size_t size);
    virtual bool canSend(size_t size);

    int             sd;                  ///< Socket file descriptor.

  protected:
    /// Pipe for termination notification
    int             control_fd[2];       ///< Pipe for termination notification.
    void*           thread;

    /// Delay (in s) without activity to check if the connection is yet available
    unsigned int    pingInterval;
    /// Delay (in s) of timeout to wait 'PONG'
    unsigned int    pongTimeout;
    /// True if waiting 'PONG'
    bool            waitingPong;

  protected:
    libport::Semaphore listenSem_;
    libport::Semaphore acceptSem_;
    int semListenInc_;
  };

} // namespace urbi
#endif
