/// \file kernel/ughostconnection.hh

#ifndef KERNEL_UGHOSTCONNECTION_HH
# define KERNEL_UGHOSTCONNECTION_HH

# include <kernel/fwd.hh>
# include <kernel/uconnection.hh>

namespace kernel
{
  /// UGhostConnection is a invisible connection used to read URBI.INI
  /*! This implentation of UConnection is trivial and does nothing.
   */

  class UGhostConnection : public UConnection
  {
  public:
    //! UGhostConnection constructor.
    UGhostConnection(UServer& s, bool interactive = false);
    //! UGhostConnection destructor.
    virtual ~UGhostConnection();

    //! Close the connection
    /*! This function does nothing. The ghost connection cannot be closed.
     */
    virtual void close();

  protected:
    // Bounce to UServer::display.
    virtual size_t effective_send(const char* buffer, size_t length);
  public:
    //! Send a "\n" through the connection
    virtual void endline();
  };

}

#endif // !KERNEL_UGHOSTCONNECTION_HH
