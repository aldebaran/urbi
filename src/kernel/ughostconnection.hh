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
    UGhostConnection(UServer& s);
    virtual ~UGhostConnection();
    virtual void close();

  protected:
    virtual size_t effective_send(const char* buffer, size_t length);
  public:
    virtual void endline();
  };

}

#endif // !KERNEL_UGHOSTCONNECTION_HH
