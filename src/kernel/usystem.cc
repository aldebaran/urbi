/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file kernel/usystem.cc
/// \brief Implementation of the USystem class.

#include <urbi/usystem.hh>

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
