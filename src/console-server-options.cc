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
 ** \file console-server-options.cc
 ** \brief Implementation of ConsoleServerOptions.
 */
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include "kernel/userver.hh"
#include "libport/package-info.hh"
#include "libport/tokenizer.hh"
#include "console-server-options.hh"

namespace options
{
  ConsoleServerOptions::ConsoleServerOptions (int argc,
                                              const char* argv[])  throw () :
    Options (argc, argv),
    period_ (),
    port_ (),
    fastMode_ (),
    bind_ (),
    portFilename_ (),
    input_ (),
    paths_ ()
  {
    getOptions ().add_options ()
      ("period,P",
       po::value<int> ()->default_value (50),
       "base URBI interval in milliseconds")
      ("port,p",
       po::value<int> ()->default_value (UServer::TCP_PORT),
       "specify the tcp port URBI will listen to")
      ("bind,b",
       po::value<std::string> ()->default_value ("localhost"),
       "bind to a specific ip address")
      ("fast,f", "ignore system time, go as fast as possible")
      ("write,w",
       po::value<std::string> (),
       "write port number to specified file")
      ("file,F",
       // FIXME: portability.
       po::value<std::string> ()->default_value ("/dev/stdin"),
       "evaluate this file at start-up")
      ("path,I",
       po::value<paths_t> (),
       "add directory to path")
      ;

    getPositionalOptions ().add ("file", 1);
  }

  ConsoleServerOptions::~ConsoleServerOptions () throw ()
  {
  }

  std::string
  ConsoleServerOptions::getUsage () const
  {
    return (boost::format ("usage: %1% [OPTIONS] [FILE]\n")
            % getArgv () [0]).str () +
      "  FILE    to load";
  }

  std::string
  ConsoleServerOptions::getVersion () const
  {
    std::ostringstream o;
    o << UServer::package_info().signature() << std::endl;
    return o.str ();
  }

  void
  ConsoleServerOptions::processArguments () throw ()
  {
    processSharedArguments ();
    try
      {
        period_          = getVariablesMap ()["period"].as<int> ();
        port_            = getVariablesMap ()["port"].as<int> ();
        if (getVariablesMap ().count ("bind"))
          bind_  = getVariablesMap ()["bind"].as<std::string> ();
        fastMode_	 = !!getVariablesMap ().count ("fast");
        if (getVariablesMap ().count ("write"))
          portFilename_  = getVariablesMap ()["write"].as<std::string> ();
        if (getVariablesMap ().count ("file"))
          input_         = getVariablesMap ()["file"].as<std::string> ();
        if (getVariablesMap ().count ("path"))
        {
          const paths_t& dirs = getVariablesMap ()["path"].as<paths_t> ();
          BOOST_FOREACH(const std::string& dir, dirs)
          {
            bool splitted = false;
            BOOST_FOREACH(const std::string& s, libport::make_tokenizer (dir, ":"))
            {
              paths_.push_back (s);
              splitted = true;
            }
            if (!splitted)
              paths_.push_back (dir);
          }
        }
      }
    catch (boost::bad_any_cast& e)
      {
        error (e.what ());
      }

    if (paths_.empty ())
    paths_.push_back (".");

    if (portFilename_ != "")
      std::ofstream (portFilename_.c_str (), std::ios::out) << port_ << std::endl;
  }

  int
  ConsoleServerOptions::getPeriod () const throw ()
  {
    return period_;
  }

  int
  ConsoleServerOptions::getPort () const throw ()
  {
    return port_;
  }

  int
  ConsoleServerOptions::isFastModeEnabled () const throw ()
  {
    return fastMode_;
  }

  const std::string&
  ConsoleServerOptions::getBindedAddress () const throw ()
  {
    return bind_;
  }

  const std::string&
  ConsoleServerOptions::getPortFilename () const throw ()
  {
    return portFilename_;
  }


  const std::string&
  ConsoleServerOptions::getInputFile () const throw ()
  {
    return input_;
  }

  const ConsoleServerOptions::paths_t&
  ConsoleServerOptions::getPaths () const throw ()
  {
    return paths_;
  }

} // end of namespace options
