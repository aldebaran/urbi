/*
 * Copyright (C) Gostai S.A.S., 2006.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 * For comments, bug reports and feedback: http://www.urbiforge.com
 */

/**
 ** \file console-server-options.hh
 ** \brief Definition of ConsoleServerOptions.
 */

#ifndef CONSOLE_SERVER_OPTIONS_HH
# define CONSOLE_SERVER_OPTIONS_HH
# include <string>
# include "options/options.hh"

namespace options
{
  /// Parse options for ConsoleServer.
  class ConsoleServerOptions : public Options
  {
  public:
    // List of paths types.
    typedef std::vector<std::string> paths_t;

    ConsoleServerOptions (int argc,
                          const char* argv[]) throw ();

    virtual ~ConsoleServerOptions () throw ();

    virtual std::string getUsage () const;
    virtual std::string getVersion () const;

    void processArguments () throw ();

    int getPeriod () const throw ();
    int getPort () const throw ();
    int isFastModeEnabled () const throw ();
    const std::string& getBindedAddress () const throw ();
    const std::string& getPortFilename () const throw ();
    const std::string& getInputFile () const throw ();
    const paths_t& getPaths () const throw ();

  private:
    int period_;
    int port_;
    bool fastMode_;
    std::string bind_;
    std::string portFilename_;
    std::string input_;
    std::vector<std::string> paths_;
  };

} // end of namespace options

#endif //! CONSOLE_SERVER_OPTIONS_HH
