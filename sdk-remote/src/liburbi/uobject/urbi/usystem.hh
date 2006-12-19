/*! \file urbi/usystem.hh
 *******************************************************************************

 File: usystem.hh\n
 Definition of the USystem class and necessary related classes.

 This file is part of UObject Component Architecture\n
 (c) 2006 Gostai S.A.S.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#ifndef URBI_USYSTEM_HH
# define URBI_USYSTEM_HH

# include <string>
# include <list>
# include <algorithm>

# include "libport/singleton-ptr.hh"
# include "libport/ufloat.h"
# include "libport/hash.hh"

# include "urbi/utypes-common.hh"

namespace urbi
{
  /// Possible value types a UValue can contain.
  enum USystemChannel {
    NEW_CHANNEL
  };

  /** USystemMessage class definition.
      USystemMessage provides a generic container for system messages.
   */
  class USystemMessage
  {
  public:

    USystemMessage();
    virtual ~USystemMessage();
  };

  /** UStringSystemMessage class definition.
      USystemMessage provides a std::string container for system messages.
   */
  class UStringSystemMessage : public USystemMessage
  {
  public:

    UStringSystemMessage(const std::string &s);
    virtual ~UStringSystemMessage();

    /// get the system message as a string
    const std::string& getMessage () const;

  private:
    std::string msg;
  };


  /** Main USystem class definition.
      USystem provides an interface to system information and low level
      kernel messages. It can only be used in plugged kernel mode.

      Current available channels for system messages are:

      NEW_CHANNEL: send a message when a new command fail, with the name of 
                   the requested object in a UStringSystemMessage
  */
  class USystem
  {
  public:

    USystem();
    virtual ~USystem();

    /// Register to be notified of kernel messages on a specific channel
    void register_channel(const USystemChannel &channel);

    /** Kernel message callback.
     * receive_message must return immediately, returning a timeout limit in
     * milliseconds. If necessary, it should spawn a thread to handle
     * asynchronous responses and set an appropriate timeout.
     * 
     * Possible channels are:
     *
     *  NEW_CHANNEL: 
     *
     *    @c message contains the name of an object on which a 'new'
     *    command failed. It is stored as a UStringSystemMessage and a cast is
     *    needed + call to @c UStringSystemMessage::getMessage.
     *
     *  How to cast?
     *  
     *  const UStringSystemMessage& msg = 
     *    dynamic_cast<const UStringSystemMessage&> (message);
     *
     *  @return the number of milliseconds before timeout.
     */
    virtual int receive_message (const USystemChannel &channel,
                                 const USystemMessage &message);
  };

} // end namespace urbi

#endif // ! URBI_UOBJECT_HH

/// Local Variables:
/// mode: c++
/// End:
