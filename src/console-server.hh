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
 ** \file console-server.hh
 ** \brief Definition of ConsoleServer.
 */

#ifndef CONSOLE_SERVER_HH
# define CONSOLE_SERVER_HH
# include "kernel/userver.hh"
# include "console-server-options.hh"

namespace urbi
{
  class ConsoleServer
    : public UServer
  {
  public:
    ConsoleServer (const options::ConsoleServerOptions& opt);
    virtual ~ConsoleServer ();

    void initNetwork ();
    void initGhostConnection ();
    virtual void shutdown ();
    virtual void beforeWork ();
    virtual void reset ();
    virtual void reboot ();
    virtual ufloat getTime ();
    virtual ufloat getPower ();

    /// Called to display the header at each coonection start
    virtual void getCustomHeader (int line, char* header, int maxlength);

    virtual UErrorValue saveFile (const char* filename, const char* content);

    virtual void effectiveDisplay (const char* t);

  private:
    const options::ConsoleServerOptions& options_;
    bool fast_;
    long long ctime_;
  };
} // end of namespace urbi

#endif // !CONSOLE_SERVER_HH
