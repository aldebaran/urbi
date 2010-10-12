/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_SOCKET_HH
# define URBI_SOCKET_HH

#include <libport/asio.hh>
#include <urbi/uobject.hh>

namespace urbi
{
  /** Use this class instead of libport::Socket when you need an UObject which
   * is also a Socket.
   */
  class UObjectSocket: public libport::Socket
  {
  public:
    UObjectSocket(boost::asio::io_service& io =
                  getCurrentContext()->getIoService())
    : libport::Socket(io)
    {
    }
    ~UObjectSocket()
    {
      close();
      wasDestroyed();
      while (!checkDestructionPermission())
        getCurrentContext()->yield_for(1000);
    }
  };
}
#endif
