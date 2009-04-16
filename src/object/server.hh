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
