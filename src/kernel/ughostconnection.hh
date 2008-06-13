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

#ifndef KERNEL_UGHOSTCONNECTION_HH
# define KERNEL_UGHOSTCONNECTION_HH

# include <kernel/fwd.hh>
# include <kernel/uconnection.hh>

/// UGhostConnection is a invisible connection used to read URBI.INI
/*! This implentation of UConnection is trivial and does nothing.
 */

class UGhostConnection : public UConnection
{
public:
  UGhostConnection (UServer& s);
  virtual ~UGhostConnection ();
  virtual UConnection& close ();

protected:
  virtual size_t effective_send (const char* buffer, size_t length);
public:
  virtual UConnection& endline ();
};

#endif // !KERNEL_UGHOSTCONNECTION_HH
