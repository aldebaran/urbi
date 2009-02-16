/****************************************************************************
 * Sample urbi client that sends commands contained in a file.
 *
 * Copyright (C) 2004, 2006, 2007, 2008, 2009 Jean-Christophe Baillie.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**********************************************************************/

/* This demonstration program sends commands to an Urbi server. */

#include <list>
#include <string>

#include <libport/cstdio>
#include <libport/sysexits.hh>

#include <libport/cli.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>

#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

using libport::program_name;

namespace
{
  static
  void
  usage()
  {
    std::cout <<
      "usage: " << program_name << " [OPTION].. [FILE]...\n"
      "\n"
      "  FILE    to upload onto the server\n"
      "\n"
      "Options:\n"
      "  -e, --expression SCRIPT  send SCRIPT to the server\n"
      "  -f, --file FILE          send the contents of FILE to the server\n"
      "  -h, --help               display this message and exit\n"
      "  -H, --host HOST          Urbi server host [localhost]\n"
      "  -p, --port PORT          Urbi server port\n"
      "  -v, --version            display version information and exit\n"
      "\n"
                << urbi::package_info().report_bugs()
                << std::endl
                << libport::exit(EX_OK);
  }

  static
  void
  version()
  {
    std::cout << "urbi-send" << std::endl
              << urbi::package_info() << std::endl
              << libport::exit(EX_OK);
  }
}

static urbi::UCallbackAction
dump(const urbi::UMessage& msg)
{
  // FIXME: This is absolutely not completely migrated.
  // To be finished -- Akim.
  switch (msg.type)
  {
    case urbi::MESSAGE_DATA:
      std::cout << msg << std::endl;
      break;

    case urbi::MESSAGE_ERROR:
    case urbi::MESSAGE_SYSTEM:
      std::cerr << msg << std::endl;
      break;
  }
  return urbi::URBI_CONTINUE;
}

static urbi::UCallbackAction
error(const urbi::UMessage& msg)
{
  dump(msg);
  exit(0);
}

/*-----------------.
| Things to send.  |
`-----------------*/

struct Data
{
  virtual ~Data() {};
  virtual void send(urbi::UClient& u) const = 0;
};

struct TextData: Data
{
  TextData(const std::string& s)
    : command_(s)
  {}

  virtual
  void
  send(urbi::UClient& u) const
  {
    u.send("%s", command_.c_str());
  }

  std::string command_;
};


struct FileData: Data
{
  FileData(const std::string& s)
    : filename_(s)
  {}

  virtual
  void
  send(urbi::UClient& u) const
  {
    u.sendFile(filename_ == "-" ? "/dev/stdin" : filename_);
  }

  std::string filename_;
};


/*-------.
| Main.  |
`-------*/

int
main(int argc, char* argv[])
{
  program_name = argv[0];
  /// Things to send to the server.
  typedef std::list<Data*> data_list;
  data_list data;
  /// Server host name.
  std::string host = "localhost";
  /// Server port.
  int port = urbi::UClient::URBI_PORT;

  // Parse the command line.
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];

    if (arg == "--expression" || arg == "-e")
      data.push_back(
        new TextData(libport::convert_argument<std::string>(arg, argv[++i])));
    else if (arg == "--file" || arg == "-f")
      data.push_back(
        new FileData(libport::convert_argument<std::string>(arg, argv[++i])));
    else if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
      host = argv[++i];
    else if (arg == "--port" || arg == "-p")
      port = libport::convert_argument<int> (arg, argv[++i]);
    else if (arg == "--version" || arg == "-v")
      version();
    else if (arg[0] == '-' && arg[1] != 0)
      libport::invalid_option (arg);
    else
      // An argument: a file.
      data.push_back(new FileData(argv[i]));
  }

  urbi::UClient client(host, port);
  client.setKeepAliveCheck(3000, 1000);
  if (client.error())
    std::cerr << libport::program_name << ": client failed to set up"
	      << std::endl
              << libport::exit(1);

  client.setWildcardCallback(callback(&dump));
  client.setClientErrorCallback(callback(&error));

  /*----------------.
  | Send contents.  |
  `----------------*/
  for (data_list::const_iterator i = data.begin(),
         i_end = data.end();
       i != i_end;
       ++i)
  {
    (*i)->send(client);
    delete *i;
  }

  std::cout << libport::program_name
	    << ": file sent, hit Ctrl-C to terminate."
            << std::endl;
  urbi::execute();
}
