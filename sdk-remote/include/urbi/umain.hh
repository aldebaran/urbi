/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/// \file urbi/umain.hh

#ifndef URBI_UMAIN_HH
# define URBI_UMAIN_HH

# include <libport/cli.hh>
# include <urbi/exit.hh>
# include <urbi/uobject.hh>

# define UMAIN()				\
  int						\
  main_args(const libport::cli_args_type& args) \
  {						\
    return urbi::main(args, true, true);        \
  }                                             \
                                                \
  int						\
  main(int argc, const char* argv[])		\
  {						\
    return urbi::main(argc, argv, true, true);  \
  }

extern "C"
{
  /** Bouncer to urbi::main() for easier access through dlsym(). */
  URBI_SDK_API int urbi_main(int argc, const char* argv[],
                             bool block, bool errors);
  /** Bouncer to urbi::main() for easier access through dlsym(). */
  URBI_SDK_API int urbi_main_args(const libport::cli_args_type& args,
                                  bool block, bool errors);
}

namespace urbi
{

  /** Initialisation method.
   * Both plugin and remote libraries include a main function whose only
   * effect is to call urbi::main. If you need to write your own main, call
   * urbi::main(argc, argv) after your work is done.
   * This function returns if block is set to false.
   */
  URBI_SDK_API int main(const libport::cli_args_type& args,
                        bool block = true, bool errors = false);

  /** Initialisation method using C style arguments.
   */
  URBI_SDK_API int main(int argc, const char *argv[],
                        bool block = true, bool errors = false);


  /** Initialisation method, for remote mode only, that returns.
   * \param host the host to connect to.
   * \param port the port number to connect to (you can use the constant
   *             UAbstractClient::URBI_PORT).
   * \param buflen receive and send buffer size (you can use the constant
   *               UAbstractClient::URBI_BUFLEN).
   * \param exitOnDisconnect call exit() if we get disconnected from server.
   * \return 0 if no error occured.
   */
  int URBI_SDK_API initialize(const std::string& host, int port, size_t buflen,
                              bool exitOnDisconnect, bool server = false);
}

#endif /* !URBI_UMAIN_HH */
