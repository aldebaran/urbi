/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#ifndef OBJECT_SERVER_HH
# define OBJECT_SERVER_HH

# include <object/socket.hh>

namespace object
{
  class URBI_SDK_API Server: public CxxObject, public libport::Socket
  {
  public:
    Server();
    Server(rServer model);
    void listen(const std::string& host, const std::string& port);
    void socket_ready(rSocket socket);
    unsigned short port() const;
    std::string host() const;
    typedef std::vector<rSocket> sockets_type;
    const sockets_type& sockets() const;

  private:
    Socket* make_socket();
    rObject connection_;
    void initialize();
    sockets_type sockets_;
    URBI_CXX_OBJECT(Server);
  };
}

#endif
