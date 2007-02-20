/*! \file ughostconnection.hh
 *******************************************************************************

 File: ughostconnection.h\n
 Definition of the UGhostConnection class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UGHOSTCONNECTION_HH
#define UGHOSTCONNECTION_HH

#include "fwd.hh"
#include "uconnection.hh"


/// UGhostConnection is a invisible connection used to read URBI.INI
/*! This implentation of UConnection is trivial and does nothing.
 */

class UGhostConnection : public UConnection
{
public:
  UGhostConnection  ( UServer *mainserver );
  virtual ~UGhostConnection ();

  virtual UErrorValue   closeConnection    ();

  // Parameters used by the constructor.

  static const int MINSENDBUFFERSIZE = 4096;
  static const int MAXSENDBUFFERSIZE = 1048576;
  static const int PACKETSIZE        = 32768;
  static const int MINRECVBUFFERSIZE = 4096;
  static const int MAXRECVBUFFERSIZE = 1048576;

protected:
  virtual int   effectiveSend     (const ubyte *buffer, int length);
};

#endif
