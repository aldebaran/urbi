/*! \file ucommand.hxx
 *******************************************************************************

 File: ucommand.xx\n
 Definition of the UCommand class.

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

#ifndef UCOMMAND_HXX
# define UCOMMAND_HXX

# include "ucommand.hh"

inline
std::ostream&
operator<<(std::ostream& o, const UCommand& u)
{
  // Yeah, we don't really use O here.  Too bad.
  u.print(0);
  return o;
}


inline
void
UCommand::is_channel_set (bool b)
{
  is_channel_ = b;
}

inline
bool
UCommand::is_channel_get () const
{
  return is_channel_;
}

inline
const TagInfo*
UCommand::tag_info_get () const
{
  return tagInfo;
}

#endif
