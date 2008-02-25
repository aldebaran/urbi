/****************************************************************************
 * $Id: urbisend.cpp,v 1.6 2005/09/21 06:45:36 nottale Exp $
 *
 * Sample urbi client that sends commands contained in a file.
 *
 * Copyright (C) 2004, 2006, 2007 Jean-Christophe Baillie.  All rights reserved.
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


/* This is a trivial demonstration program that send commands contained in a file to an urbi server */

#include "libport/cstdio"
#include "libport/sysexits.hh"
#include "urbi/uclient.hh"

urbi::UCallbackAction
dump(const urbi::UMessage & msg)
{
  // FIXME: This is absolutely not completely migrated.
  // To be finished -- Akim.
  switch (msg.type)
  {
    case urbi::MESSAGE_DATA:
      std::cerr << *msg.value << std::endl;
      break;

    case urbi::MESSAGE_ERROR:
    case urbi::MESSAGE_SYSTEM:
      std::cerr << msg.timestamp << " "
		<< msg.tag << " "
		<< msg.message << std::endl;
      break;
  }
  return urbi::URBI_CONTINUE;
}

urbi::UCallbackAction
error(const urbi::UMessage& msg)
{
  dump(msg);
  exit(0);
}


int main(int argc, char *argv[])
{
  if (argc != 3)
    {
      std::cerr << "Missing file name\nUsage: urbisend robotname filename"
		<< std::endl;
      exit(EX_USAGE);
    }

  urbi::UClient client (argv[1]);
  client.setKeepAliveCheck(3000, 1000);
  if (client.error())
    exit(1);
  client.setWildcardCallback(callback(&dump));
  client.setClientErrorCallback(callback(&error));
  client.sendFile(argv[2]);

  std::cout << "File sent, hit Ctrl-C to terminate." << std::endl;
  urbi::execute();

  return 0;
}
