/// \file urbi/umain.hh

#ifndef URBI_UMAIN_HH
# define URBI_UMAIN_HH

# include <libport/cli.hh>
# include <urbi/uobject.hh>

# define UMAIN()				\
  int						\
  main(const cli_args_type& args)               \
  {						\
    urbi::main(args, true);                     \
  }                                             \
                                                \
  int						\
  main(int argc, const char *argv[])		\
  {						\
    urbi::main(argc, argv, true);		\
  }

extern "C"
{
  /** Bouncer to urbi::main() for easier access through dlsym(). */
  USDK_API int urbi_main(int argc, const char* argv[], bool block);
  /** Bouncer to urbi::main() for easier access through dlsym(). */
  USDK_API int urbi_main_args(const libport::cli_args_type& args, bool block);
}

namespace urbi
{

  /** Initialisation method.
   * Both plugin and remote libraries include a main function whose only
   * effect is to call urbi::main. If you need to write your own main, call
   * urbi::main(argc, argv) after your work is done.
   * This function returns if block is set to false.
   */
  USDK_API int main(const libport::cli_args_type& args, bool block = true);

  /** Initialisation method using C style arguments.
   */
  USDK_API int main(int argc, const char *argv[], bool block = true);


#ifdef URBI_ENV_REMOTE
  /** Initialisation method, for remote mode only, that returns.
   * \param host the host to connect to.
   * \param port the port number to connect to (you can use the constant
   *             UAbstractClient::URBI_PORT).
   * \param buflen receive and send buffer size (you can use the constant
   *               UAbstractClient::URBI_BUFLEN).
   * \param exitOnDisconnect call exit() if we get disconnected from server.
   * \return 0 if no error occured.
   */
  int USDK_API initialize(const std::string& host, int port, int buflen,
                          bool exitOnDisconnect, bool server = false);
#endif
}

#endif /* !URBI_UMAIN_HH */
