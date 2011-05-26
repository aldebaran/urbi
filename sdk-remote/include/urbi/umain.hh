/*
 * Copyright (C) 2009, 2010, 2011, Gostai S.A.S.
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
# include <urbi/urbi-root.hh>

# ifdef STATIC_BUILD
#  define UMAIN_URBIROOT_STATIC true
# else
#  define UMAIN_URBIROOT_STATIC false
# endif

# define UMAIN()                                                \
                                                                \
  int                                                           \
  main(int argc, const char** argv)                             \
  {                                                             \
    UrbiRoot urbi_root(argv[0], UMAIN_URBIROOT_STATIC);         \
    std::vector<std::string> args(argv, argv + argc);           \
    return urbi_root.urbi_main(args, true, true);               \
  }                                                             \
                                                                \
  int main_args(const libport::cli_args_type& args);            \
                                                                \
  int                                                           \
  main_args(const libport::cli_args_type& args)                 \
  {                                                             \
    size_t argc = args.size();                                  \
    const char** argv = new const char*[argc];                  \
    for (unsigned i = 0; i < argc; ++i)                         \
      argv[i] = args[i].c_str();                                \
    int res = main(argc, argv);                                 \
    delete [] argv;                                             \
    return res;                                                 \
  }                                                             \

extern "C"
{
  /** Bouncer to urbi::main() for easier access through dlsym(). */
  URBI_SDK_API
  int urbi_main(int argc, const char* argv[], UrbiRoot& root,
                bool block, bool errors);
  /** Bouncer to urbi::main() for easier access through dlsym(). */
  URBI_SDK_API
  int urbi_main_args(const libport::cli_args_type& args, UrbiRoot& root,
                     bool block, bool errors);
}

namespace urbi
{

  /** Initialization method.
   * Both plugin and remote libraries include a main function whose only
   * effect is to call urbi::main. If you need to write your own main, call
   * urbi::main(argc, argv) after your work is done.
   * This function returns if block is set to false.
   */
  URBI_SDK_API
  int
  main(const libport::cli_args_type& args, UrbiRoot& root,
       bool block = true, bool errors = false);

  /** Initialisation method using C style arguments.
   */
  URBI_SDK_API
  int
  main(int argc, const char *argv[], UrbiRoot& root,
       bool block = true, bool errors = false);


  /** Initialisation method, for remote mode only, that returns.
   * \param host the host to connect to.
   * \param port the port number to connect to (you can use the constant
   *             UAbstractClient::URBI_PORT).
   * \param buflen receive and send buffer size (you can use the constant
   *               UAbstractClient::URBI_BUFLEN).
   * \param exitOnDisconnect call exit() if we get disconnected from server.
   * \param server  whether listens instead of connecting.
   * \param files Files to send when connected.
   * \param useSyncClient use a UClient instead of USyncClient if false.
   * \return 0 if no error occured.
   */
  URBI_SDK_API
  int
  initialize(const std::string& host, int port, size_t buflen,
             bool exitOnDisconnect, bool server = false,
             const std::vector<std::string>& files = std::vector<std::string>(),
             bool useSyncClient = true);
}

#endif /* !URBI_UMAIN_HH */
