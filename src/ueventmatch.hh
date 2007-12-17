/*! \file ueventmatch.hh
 *******************************************************************************

 File: ueventmatch.h\n
 Definition of the UEventMatch class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UEVENTMATCH_HH
# define UEVENTMATCH_HH

# include <list>
# include <string>

# include "kernel/fwd.hh"

namespace kernel
{
  /// UEventMatch associated to the @c system.alwaysTrue eventhandler
  extern UEventMatch* eventmatch_true;

  /// UEventMatch associated to the @c system.alwaysFalse eventhandler
  extern UEventMatch* eventmatch_false;
}

// ****************************************************************************
/** UEventMatch stores a successful match between one event expression and a
 * set of possible event instanciations for that expression.
 *
 * A match with the expression event 'myevent(x,1)' can be a list of @c UEvent
 * like ['myevent(1,1)','myevent(7,1)'], obtained according to the current
 * state of events in the kernel. It is the result of the application of a
 * filter given by the event expression 'myevent(x,1)' and a corresponding @c
 * UEventHandler.
 *
 * @c UEventMatch is used as part of @c UEventCompound that stores the logical
 * (AND, OR, !) structure if an event expression.
 */
class UEventMatch
{
public:
  /** UEventMatch constructor.
   * @param eventname The name of the event to match (unforged)
   * @param filter The list of parameters for the event filter
   * @param command The @c UCommand who created the event
   * @param connection The @c UConnection where errors shall be sent
   */
  UEventMatch(UString* eventname,
	      UNamedParameters* filter,
	      UCommand* command,
	      UConnection* connection);

  /** UEventMatch constructor for system events
   * @param eh The @c UEventHandler that must be matched. All events in the
   * handler will be matched (empty filter) and the UEventMatch will not be
   * deleteable, for memory optimization reason (since it is a fixed, never
   * changing event that is used in computation of UEventCompounds only)
   */
  UEventMatch(UEventHandler *eh);

  /** UEventMatch destructor */
  virtual ~UEventMatch();

  /** Accessor to the matching list
   * @return A reference to @a matches_
   */
  std::list<UEvent*>& matches();

  /** Accessor to the filter
   * @return A reference to @a filter_
   */
  std::list<UValue*>& filter();


  /** Reduces the list of matching events to positive or negative events.
   * Positive events are currently 'existing' events and negative one are
   * those marked with a toDelete flag. Negative events are in a zoombie state
   * and will be detected on a !event matching
   * @param st The state to which the list must be reduced (true=positive,
   * false=negative)
   */
  void reduce(bool st);

  /** Reduces the list of matching events with the current state.
   * Simply calls reduce(state_)
   */
  void reduce();

  /// Returns the match state: positive or negative
  bool state();

  /// Sets the match state: positive or negative
  void setState(bool st);

  /** True when then the UEventMatch is not associated to one of the ever
   * existing @c system.alwaysTrue or @c system.alwaysFalse eventhandler
   */
  bool deleteable();

private:

  /** Find @a matches_ between the @a filter_ and the @a eventhandler_ */
  void findMatches_();

  /** The matching filter.
   * Variables are wildcards expressed as @c UValue with type @c DATA_VARIABLE
   * (created especially for this purpose) other expressions are
   * evaluated to normal @c UValue and enforced in the filter
   */
  std::list<UValue*> filter_;

  /// List of UEvent matching the filter
  std::list<UEvent*> matches_;

  /// Identified @c UEventHandler that matches the name and nbarg of the filter
  UEventHandler* eventhandler_;

  /** The match state: positive or negative.
   * A positive match corresponds to a match with existing events, a negative
   * match corresponds to zoombie events that are about to be deleted. Both
   * type of events are stored in UEventMatch and this flag is used to store
   * how the parser intended the matching to be interpreted
   */
  bool state_;

  /// is the @c UEventMatch deleteable?
  bool deleteable_;
};

// Inline functions

inline std::list<UEvent*>&
UEventMatch::matches()
{
  return matches_;
}

inline std::list<UValue*>&
UEventMatch::filter()
{
  return filter_;
}

inline bool
UEventMatch::state()
{
  return state_;
}

inline bool
UEventMatch::deleteable()
{
  return deleteable_;
}


inline void
UEventMatch::setState(bool st)
{
  state_ = st;
}


#endif
