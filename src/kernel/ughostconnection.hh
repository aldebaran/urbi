/*
 * Copyright (C) 2005-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file kernel/ughostconnection.hh

#ifndef KERNEL_UGHOSTCONNECTION_HH
# define KERNEL_UGHOSTCONNECTION_HH

# include <urbi/kernel/fwd.hh>
# include <urbi/kernel/uconnection.hh>

namespace kernel
{
  /// UGhostConnection is a invisible connection used to boot
  /// urbiscript (reading urbi/urbi.u, global.u).

  class UGhostConnection : public UConnection
  {
  public:
    typedef UConnection super_type;

    /// UGhostConnection constructor.
    UGhostConnection(UServer& s, bool interactive = false);

    //! UGhostConnection destructor.
    virtual ~UGhostConnection();

  protected:
    //! Close the connection
    /// Does nothing. The ghost connection cannot be closed.
    virtual void close_();

    /// Bounce to UServer::display.
    virtual size_t effective_send(const char* buffer, size_t length);

  public:
    /// Send a "\n" through the connection.
    virtual void endline();
  };

}

#endif // !KERNEL_UGHOSTCONNECTION_HH
