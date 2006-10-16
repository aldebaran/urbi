/*! \file aiboconnection.h
 *******************************************************************************

 File: aiboconnection.h\n
 Definition of the AiboConnection class.

 This file is part of 
 %URBI Server Aibo, version __rsaiboversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://urbi.sourceforge.net

 **************************************************************************** */

#ifndef AIBOCONNECTION_H_DEFINED
#define AIBOCONNECTION_H_DEFINED

#include <ant.h>
#include <IPAddress.h>
#include <OPENR/ODataFormats.h>
#include <antTypes.h>
 
#include "uconnection.h"

/// AiboConnection implements an TCP/IP client connection using OPENR.
/*! AiboConnection uses the asynchronous message sending method of OPENR
    Once a buffer has been sent to the IPStack object, one should wait
    for the callback stating that the system is reading to send a new
    string. This asynchronous mechanism is handled by UConnection using
    its blocking feature (see UConnection::block());
*/
class AiboConnection : public UConnection 
{
public:

  AiboConnection();
  virtual ~AiboConnection();

  UErrorValue           oListen            ();
  UErrorValue           oReceive           ();
  UErrorValue           oClose             ();
  byte*                 antRecvData        ();

  virtual UErrorValue   closeConnection    ();


  bool             isListening;       ///< true when AiboConnection is waiting
                                      ///< for the TCPListenCont callback.
  bool             isClosing;         ///< true when AiboConnection is waiting
                                      ///< for the TCPCloseCont callback.
 
  int              cameraFormat;      ///< Camera image format: 
                                      ///< - 0 = YCrCb
                                      ///< - 1 = JPEG

  int              cameraJpegfactor;  ///< jpeg compression factor in 0..100

  OFbkImageLayer   cameraResolution;  ///< Camera resolution
                                      ///<  - 0 = ofbkimageLAYER_H
                                      ///<  - 1 = ofbkimageLAYER_M
                                      ///<  - 2 = ofbkimageLAYER_L
                                      ///<  - 3 = ofbkimageLAYER_C

  bool             cameraReconstruct; ///< reconstruction of the image.

  bool             recoverFromIsolation; ///< true when the oReceive function 
                                         ///< has not been recalled after a 
                                         ///< URBI::TCPReceiveCont
  
  // Parameters used by the constructor.
  
  static const int MINSENDBUFFERSIZE = 4096;
  static const int MAXSENDBUFFERSIZE = 1048576;
  static const int PACKETSIZE        = 32768;
  static const int MINRECVBUFFERSIZE = 4096;
  static const int MAXRECVBUFFERSIZE = 32768;

protected:

  virtual int      effectiveSend      (const ubyte *buffer, int length);

private:

  antModuleRef     endpoint_;         ///< Internal OPENR stuff.

  // OPENR Send buffer data
  antSharedBuffer  antSendBuffer_;
  byte*            antSendData_;///< byte* is for OPENR (no kernel's ubyte*)

  // OPENR Receive buffer data
  antSharedBuffer  antRecvBuffer_;
  byte*            antRecvData_;///< byte* is for OPENR (no kernel's ubyte*)
};

//! Accessor for antReceiveData_ used by URBI::ReceiveCont()
/*! The returned type is not ubyte* (URBI kernel) but byte* (OPENR lib).
 */
inline byte*
AiboConnection::antRecvData()
{
  return antRecvData_;
}

#endif
