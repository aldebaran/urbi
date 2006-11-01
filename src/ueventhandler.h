/*! \file ueventhandler.h
 *******************************************************************************

 File: ueventhandler.h\n
 Definition of the UEventHandler class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UEVENTHANDLER_H_DEFINED
#define UEVENTHANDLER_H_DEFINED

#include <list>
#include <string>

#include "fwd.hh"
#include "ustring.h"
#include "uasyncregister.h"

namespace kernel
{
  /** Builds a hashable name by adding the event name and its nb of arg.
   * The resulting chain is called a 'forged name' for the event
   * @param name The name of the event
   * @param nbarg The number of parameters
   * @return The concatenation of name and nbarg
   */
  std::string forgeName(UString* name, int nbarg);

  /** Locate an event handler with an event name and a nb of arg
   * @param name The name of the event (unforged)
   * @param nbarg The number of arguments
   * @return The UEventHandler pointer if found, zero otherwise
   */
  UEventHandler* findEventHandler(UString* name, int nbarg);

  /** Returns true when the specified fullname is a core function
   * of the kernel.
   * @param fullname The name of the function to test
   * @return True when @a fullname is a core function
   */
  bool isCoreFunction(UString* fullname);

  /** Returns true when an event exists with a given base name (regardless of
   * the number of parameters).
   * @param symbol Event name
   * @return True when there is an event with that name
   */
  bool eventSymbolDefined(const char* symbol);

  /// Used to store the system.alwaysTrue event
  extern UEventHandler* eh_system_alwaystrue;

  /// Empty handler to store the "always false" event
  extern UEventHandler* eh_system_alwaysfalse;

  /// The system.alwaysTrue event
  extern UEvent* system_alwaystrue;
}

/** UEvent class, storing an effective event instanciation.
 * UEventHandler is the generic container for a specific event name+nb arg
 * each instanciation of this event (effective list of args) is attached to
 * a UEvent stored in the UEventHandler::eventlist_
 */
class UEvent
{
public:

  /** UEvent constructor
   * @param eventhandler The parent eventhandler
   * @param args The event arguments
   */
  UEvent(UEventHandler* eventhandler, std::list<UValue*>& args);

  /** UEvent destructor */
  ~UEvent();

  /** Returns a list of the event arguments
   * @return A @c UValue* list of the event arguments
   */
  std::list<UValue*>& args();

  /** Turn on the zombie mode */
  void doDelete();

  /** Returns zoombie state.
   * @return The zoombie state
   */
  bool toDelete();

  /** Returns the unique id.
   * @return The unique id.
   */
  int id();

private:
  /// UEventHandle to which the event is attached
  UEventHandler* eventhandler_;

  /// List of the event's parameters
  std::list<UValue*> args_;

  /// Zombie state just before deletion
  bool toDelete_;

  /// Event unique id
  int id_;
};

/** Contains an event handler definition.
 * Just like UVariables containing values, UEventHandler
 * contain events (stored eventlist_, a list of UEVent).
 * UEventHandler is a generic event signature: name + nbarg
 * The actual instanciations of this event, coming from calls to
 * emit, are stored in the UEvent list
 */
class UEventHandler : public UASyncRegister
{
public:

  /** UEventHandler constructor.
   * @param name Name of the events (unforged)
   * @param nbarg Number of arguments of the events
   */
  UEventHandler(UString* name, int nbarg);

  /** UEventHandler destructor */
  ~UEventHandler();

  /** add an event to the handler.
   *  @param parameters The list of event parameters
   *  @param command The @c UCommand who created the event
   *  @param connection The @c UConnection where errors shall be sent
   *  @return The event created
   */
  UEvent* addEvent(UNamedParameters* parameters,
                        UCommand* command,
                        UConnection* connection);

  /** add an already existing event to the handler.
   *  @param e The event to add
   *  @return The event added
   */
  UEvent* addEvent(UEvent* e);

  /** Remove an event from the handler
   * @param event The event to remove
   */
  void removeEvent(UEvent* event);

  /** Get a valid char* usable to hash in emittab
   * @return A valid char* (that will live as long as the eventhandler
   */
  const char* getHashString();

  /** Get a reference to the list of events in the handler.
   * @return A reference to @a eventlist_
   */
  std::list<UEvent*>& eventlist();

  /** When no Event in the handler is active (empty list or everything is
   * toDelete).
   * @return True when there is no positive event
   */
  bool noPositive();

  //tmp hack
  UString* unforgedName;

private:
  /// Name of the event
  std::string name_;

  /// Number of args of the event
  int nbarg_;

  /// List of triggered events
  std::list<UEvent*> eventlist_;
};


// Inline functions

inline void
UEvent::doDelete()
{
  toDelete_ = true;
}

inline bool
UEvent::toDelete()
{
  return toDelete_;
}

inline int
UEvent::id()
{
  return id_;
}


inline std::list<UValue*>&
UEvent::args()
{
  return args_;
}

inline const char*
UEventHandler::getHashString()
{
  return name_.c_str();
}

inline std::list<UEvent*>&
UEventHandler::eventlist()
{
  return eventlist_;
}


#endif
