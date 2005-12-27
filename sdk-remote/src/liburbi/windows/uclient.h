/*! \file uclient.h
****************************************************************************
 * $Id: uclient.h,v 1.2 2005/09/21 06:45:36 nottale Exp $
 *
 * Definition of the URBI interface class
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

#ifndef UCLIENT_H
#define UCLIENT_H
#include <Windows.h>
//#include <winsock2.h>
#include "uabstractclient.h"



///Linux implementation of UAbstractClient.
/*! This implementation creates a thread for each instance of UClient, which
  listens on the associated socket.
 */
class UClient: public UAbstractClient {

 public:

  UClient(const char *_host,int _port = URBI_PORT,int _buflen = URBI_BUFLEN);

  virtual ~UClient();
  
  //! For compatibility with older versions of the library
  void start() {}
 
  //! For internal use.
  void listenThread();

  virtual void printf(const char * format, ...);
  virtual unsigned int getCurrentTime();

  virtual void lockList();
  virtual void unlockList();
 protected:

  virtual int  effectiveSend(const void * buffer, int size);
  virtual bool canSend(int size);

  virtual void lockSend();
  virtual void unlockSend();

  
  SOCKET              sd;                  ///< Socket file descriptor.    
 private:
 
  CRITICAL_SECTION  writeLock;           ///< Send structure lock.
  CRITICAL_SECTION  listLock;            ///< Receive structure lock.
  HANDLE         listenThreadStruct;
};


/// Namespace containing wrappers for platform-dependant functions.
namespace urbi {
  ///This function must be called at the last line of your main() function.
  void execute(void);
  ///This function will terminate your URBI program.
  void exit(int code);
  /// Creates a new UClient object
  UClient & connect(const char * host);
  /// Returns the first UClient created by the program. Used by the URBI macro
  UClient * getDefaultClient();
}
#endif
