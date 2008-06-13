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
