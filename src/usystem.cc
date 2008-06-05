/*! \file usystem.cc
 *******************************************************************************

 File: usystem.cc\n
 Implementation of the USystem class.

 This file is part of UObject Component Architecture\n
 (c) Gostai S.A.S., 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <cstdarg>
#include <libport/cstdio>
#include <list>

#include <urbi/usystem.hh>

#include <kernel/utypes.hh>
#include <kernel/userver.hh>
#include <kernel/uconnection.hh>

#include <ughostconnection.hh>

#define LIBURBIDEBUG

namespace urbi
{
  // **************************************************************************
  // USystemMessage

  //! USystemMessage constructor.
  USystemMessage::USystemMessage()
  {
  }

  //! USystemMessage destructor.
  USystemMessage::~USystemMessage()
  {
  }

  // **************************************************************************
  // UStringSystemMessage

  //! UStringSystemMessage constructor.
  UStringSystemMessage::UStringSystemMessage (const std::string& s)
    : msg (s)
  {
  }

  //! UStringSystemMessage destructor.
  UStringSystemMessage::~UStringSystemMessage()
  {
  }

  const std::string&
  UStringSystemMessage::getMessage () const
  {
    return msg;
  };

  // **************************************************************************
  // USystem

  //! USystemMessage constructor.
  USystem::USystem()
  {
  }

  //! USystemMessage destructor.
  USystem::~USystem()
  {
  }

  void
  USystem::register_channel (const USystemChannel&)
  {
  }

  int
  USystem::receive_message (const USystemChannel&,
			    const USystemMessage&)
  {
    // user defined.
    return 0;
  }

} // namespace urbi
