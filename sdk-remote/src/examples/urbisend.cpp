/****************************************************************************
 * $Id: urbisend.cpp,v 1.6 2005/09/21 06:45:36 nottale Exp $
 *
 * Sample urbi client that sends commands contained in a file.
 *
 * Copyright (C) 2004 Jean-Christophe Baillie.  All rights reserved.
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

#include <stdio.h>
#include "uclient.h"

UCallbackAction dump(const UMessage & msg) {
  switch (msg.type) {
  case MESSAGE_DOUBLE:
    printf("%d %s %lf\n", msg.timestamp, msg.tag, msg.doubleValue);
    break;
  case MESSAGE_STRING:
  case MESSAGE_ERROR:
  case MESSAGE_SYSTEM:
    printf("%d %s %s\n", msg.timestamp, msg.tag, msg.stringValue);
    break;
  case MESSAGE_LIST:
    printf("%d %s %d [", msg.timestamp, msg.tag, msg.listSize);
    for (int i=0;i<msg.listSize;i++)
      if (msg.listValue[i].type == MESSAGE_DOUBLE)
	printf("%lf, ", msg.listValue[i].doubleValue);
      else
	printf("'%s', ", msg.listValue[i].stringValue);
    printf("]\n");
    break;
  }
  return URBI_CONTINUE;
}


int main(int argc, char *argv[])
{
  UClient *client;

  if (argc < 3) {
    fprintf(stderr, "Missing file name\nUsage: urbisend robotname filename\n");
    exit(1);
  }

 
    client = new UClient(argv[1]);
  if (client->error() != 0)
    exit(0);
  client->setWildcardCallback(callback(&dump));
  client->sendFile(argv[2]);
  fprintf(stdout, "File sent, hit Ctrl-C to terminate.\n");
  urbi::execute();
  return 0;
}
