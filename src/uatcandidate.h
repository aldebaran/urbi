/*! \file uatcandidate.h
 *******************************************************************************

 File: uatcandidate.h\n
 Definition of the UAtCandidate class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UATCANDIDATE_H_DEFINED
#define UATCANDIDATE_H_DEFINED

#include <list>
#include <string>

#include "fwd.hh"
#include "utypes.h"
#include "ustring.h"


// ****************************************************************************
/** UAtCandidate is a pending UMultiEventInstance that can trigger in a @c at
 * command.
 *
 * UAtCandidate is basically a UMultiEventInstance that stores the time of its
 * creation so that the softtest triggering mechanism can be handled.
 */
class UAtCandidate
{
public:
  /** UAtCandidate constructor.
   * @param startTime The time when the constructor is called
   * @param mei The @c UMultiEventInstance that is candidate
   */
  UAtCandidate(ufloat endTime, UMultiEventInstance* mei);

  /** UAtCandidate destructor */
  virtual ~UAtCandidate();

  /** Set the state to "checked" */
  void visited();

  /** Set the state to "unchecked" */
  void unVisited();

  /** Returns the state of "checked" */
  bool isVisited();

  /** Is the candidate triggering? If so, apply the matching.
   * @param currentTime The current time
   * @param cmd The assignement command when the current time is more than the @a endTime_ and the
   * event is triggering, zero otherwise
   * @return True when the candidate triggers
   */
  bool trigger(ufloat currentTime, UCommand*& cmd);

  /** Test if the candidate has already triggered (for optimization purposes)
   * @return True when the candidate has already triggered
   */
  bool hasTriggered();

  /** Compares a candidate to an actual UMultiEventInstance.
   * @param mei The @c UMultiEventInstance to be compared
   * @return True when @a mei_ and @c mei are equal
   */
  bool equal(UMultiEventInstance* mei);

private:
  ufloat endTime_; ///< Time when the candidate should trigger
  UMultiEventInstance* mei_; ///< The MultiEvent instance that is candidate
  bool checked_; ///< true when the candidate has been relocated in the last
                ///< evaluation.
  bool hasTriggered_; ///< true when the event has already triggered.
};

inline void
UAtCandidate::visited()
{
  checked_ = true;
}

inline void
UAtCandidate::unVisited()
{
  checked_ = false;
}

inline bool
UAtCandidate::isVisited()
{
  return checked_;
}

inline bool
UAtCandidate::hasTriggered()
{
  return hasTriggered_;
}


#endif
