/*! \file usyncclient.h
****************************************************************************
 * $Id: usyncclient.h,v 1.5 2005/09/21 06:45:36 nottale Exp $
 *
 * Definition of the URBI interface class
 *
 * Copyright (C) 2004, 2006 Jean-Christophe Baillie.  All rights reserved.
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

#include "uclient.h"

class Semaphore;
class Lockable;

/*! Format in which image requested with syncGetSound are transmitted*/
enum UTransmitFormat
{
  URBI_TRANSMIT_JPEG, ///< Transmit images compressed in JPEG
  URBI_TRANSMIT_YCbCr ///< Transmit raw YCbCr images
};

/// UClient linux implementation with support for synchronous functions.
/*! These functions differs from the UClient interface in that they are
  synchronous. One must seriously ponder the fact that they are not easily
  portable before using them.
 */
class USyncClient: public UClient
{
 public:
  USyncClient(const char *_host,
	      int _port = URBI_PORT,
	      int _buflen = URBI_BUFLEN);

  /// Sends the expression and returns the result.
  UMessage * syncGet(const char * expression,...);

  /// Send given buffer without copying it.
  int syncSend(const void * buffer, int length);

  /// Get an image in a synchronous way. Returns 1 on success, 0 on failure.
  int syncGetImage(const char* cameraDevice, void* buffer, int &buffersize,
		   int format, int transmitFormat, int &width, int &height);

  /// Get the value of a device in a synchronous way. Returns 1 on success, 0 on failure.
  int syncGetDevice(const char* device, double &val);

  /// Execute an URBI command, return the resulting double value. Returns 1 on success, 0 on failure.
  int syncGetResult(const char* command, double &val);

  /// Get the normalized value of a device in a synchronous way. Returns 1 on success, 0 on failure.
  int syncGetNormalizedDevice(const char* device, double &val);

  /// Get a field of a device in a synchronous way. Returns 1 on success, 0 on failure.
  int syncGetDevice(const char* device, const char * field, double &val);

  /// Get sound for duration milliseconds in buffer.
  int syncGetSound(const char * device, int duration, urbi::USound &sound);

  /// Wait until a message with specified tag is received. Returned message must be deleted.
  UMessage * waitForTag(const char * tag);
  /// Overriding UAbstractclient implementation
  virtual void notifyCallbacks(const UMessage &msg);

  void callbackThread();

 private:
  Semaphore * sem;
  std::list<UMessage*> queue;
  Lockable * queueLock;
  UMessage * msg;
  Semaphore * syncLock;
  std::string syncTag;
};
