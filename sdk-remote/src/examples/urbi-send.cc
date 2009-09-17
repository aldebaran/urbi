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

#include <boost/bind.hpp>

#include <libport/cli.hh>
#include <libport/foreach.hh>
#include <libport/option-parser.hh>
#include <libport/package-info.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>

#include <urbi/package-info.hh>
#include <urbi/uclient.hh>

using libport::program_name;

namespace
{
  static
  void
  usage(libport::OptionParser& parser)
  {
    std::cout <<
      "usage: " << program_name() << " [OPTION].. [FILE]...\n"
      "\n"
      "  FILE    to upload onto the server\n"
                << parser
                << "\n"
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
dump(void* banner, const urbi::UMessage& msg)
{
  if ((msg.tag == "start" || msg.tag == "ident")
      && !*static_cast<bool*>(banner))
    return urbi::URBI_CONTINUE;

  switch (msg.type)
  {
    case urbi::MESSAGE_DATA:
    case urbi::MESSAGE_SYSTEM:
      std::cout << msg << std::endl;
      break;

    case urbi::MESSAGE_ERROR:
      std::cerr << msg << std::endl;
      break;
  }
  return urbi::URBI_CONTINUE;
}

static urbi::UCallbackAction
error(void* banner, const urbi::UMessage& msg)
{
  dump(banner, msg);
  exit(0);
}


/*-----------------.
| Things to send.  |
`-----------------*/

struct Data
{
  virtual ~Data() {};
  typedef urbi::UAbstractClient::error_type error_type;

  /// Send and return the error code, but don't issue error
  /// messages.
  virtual error_type send(urbi::UClient& u) const = 0;

  /// Try to send, and die verbosely on errors.
  void xsend(urbi::UClient& u) const;

  /// Print.
  virtual std::ostream& dump(std::ostream& o) const = 0;
};

inline
std::ostream&
operator<< (std::ostream& o, const Data& d)
{
  return d.dump(o);
}

void Data::xsend(urbi::UClient& u) const
{
  if (send(u))
  {
    std::cerr << libport::program_name() << ": failed to send " << *this;
    if (errno)
      std::cerr << ": " << strerror(errno);
    std::cerr << std::endl << libport::exit(1);
  }
}

struct TextData: Data
{
  TextData(const std::string& s)
    : command_(s)
  {}

  virtual
  error_type
  send(urbi::UClient& u) const
  {
    return u.send("%s", command_.c_str());
  }

  virtual
  std::ostream&
  dump(std::ostream& o) const
  {
    return o << "expression {{{" << command_ << "}}}";
  }

  std::string command_;
};


struct FileData: Data
{
  FileData(const std::string& s)
    : filename_(s)
  {}

  virtual
  error_type
  send(urbi::UClient& u) const
  {
    return u.sendFile(filename_ == "-" ? "/dev/stdin" : filename_);
  }

  virtual
  std::ostream&
  dump(std::ostream& o) const
  {
    return o << "file `" << filename_ << "'";
  }

  std::string filename_;
};


/*-------------.
| Callbacks.   |
`-------------*/

typedef std::list<Data*> data_list;

void add_exp(data_list* data, const std::string& arg)
{
  data->push_back(new TextData(arg));
}

void add_file(data_list* data, const std::string& arg)
{
  data->push_back(new FileData(arg));
}

/*-------.
| Main.  |
`-------*/

int
main(int argc, char* argv[])
{
  libport::program_initialize(argc, argv);
  /// Things to send to the server.
  data_list data;
  /// Display the server's banner.
  bool banner = false;

  // Parse the command line.
  libport::OptionValues
    arg_exp("send SCRIPT to the server", "expression", 'e', "SCRIPT"),
    arg_file("send the contents of FILE to the server", "file", 'f', "FILE");
  libport::OptionValue
    arg_pfile("file containing the port to listen to", "port-file", 0, "FILE");
  libport::OptionFlag
    arg_banner("do not hide the server-sent banner", "banner", 'b'),
    arg_quit("send `quit' at the end to disconnect", "quit", 'q');

  typedef boost::function1<void, const std::string&> cb_type;

  cb_type cb_exp(boost::bind(&add_exp, &data, _1));
  cb_type cb_file(boost::bind(&add_file, &data, _1));
  arg_exp.set_callback(&cb_exp);
  arg_file.set_callback(&cb_file);

  libport::OptionParser opt_parser;
  opt_parser << "Options:"
	     << arg_banner
	     << arg_exp
	     << arg_file
	     << libport::opts::help
	     << libport::opts::host_l
	     << libport::opts::port_l
	     << arg_pfile
	     << libport::opts::version
             << arg_quit;

  libport::cli_args_type
    remainings_args = opt_parser(libport::program_arguments());

  if (libport::opts::help.get())
    usage(opt_parser);
  banner = arg_banner.get();
  /// Server host name.
  std::string host = libport::opts::host_l.value(urbi::UClient::default_host());
  /// Server port.
  int port = libport::opts::port_l.get<int>(urbi::UClient::URBI_PORT);
  if (arg_pfile.filled())
    port = libport::file_contents_get<int>(arg_pfile.value());
  if (libport::opts::version.get())
    version();

  foreach(std::string arg, remainings_args)
    if (arg[0] == '-' && arg[1] != 0)
      libport::invalid_option (arg);
    else
      data.push_back(new FileData(arg));

  urbi::UClient client(host, port);
  client.setKeepAliveCheck(3000, 1000);
  if (client.error())
    std::cerr << libport::program_name() << ": client failed to set up"
	      << std::endl
              << libport::exit(1);

  client.setWildcardCallback(callback(&dump, &banner));
  client.setClientErrorCallback(callback(&error, &banner));

  /*----------------.
  | Send contents.  |
  `----------------*/
  foreach (Data* d, data)
  {
    d->xsend(client);
    delete d;
  }

  if (arg_quit.get())
    client.send("quit;");
  else
    std::cout << libport::program_name()
              << ": file sent, hit Ctrl-C to terminate."
              << std::endl;
  urbi::execute();
}
