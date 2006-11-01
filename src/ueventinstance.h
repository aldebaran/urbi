/*! \file ueventinstance.h
 *******************************************************************************

 File: ueventinstance.h\n
 Definition of the UEventInstance and UMultiEventInstance class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UEVENTINSTANCE_H_DEFINED
#define UEVENTINSTANCE_H_DEFINED

#include <list>
#include <string>

#include "fwd.hh"
#include "ustring.h"


// ****************************************************************************
/** UEventInstance stores an event signature and an event match to this
 * signature.
 *
 * An event signature can be something like 'myevent(x,3)' which will be
 * stored as a list of parameters, some of them are variables (seen as
 * wildcard), some of them are values (all stored in UValues)
 * An event match is something like 'myevent(77,3)' which correspond to the
 * signature and is also stored as a list of UValues.
 * The signature is kept in @c UEventInstance because it will be used to
 * instanciate the variables it contains with the propriate corresponding
 * value when the instance is "activated".
 */
class UEventInstance
{
  friend class UAtCandidate;

public:
  /** UEventInstance constructor.
   * @param match The match that will be used to create a list of variable
   * instanciation
   * @param e The event that matches (part of @a match event list)
   */
  UEventInstance(UEventMatch *match, UEvent* e);

  /** UEventInstance copy constructor.*/
  UEventInstance(UEventInstance* uei);

  /** UEventInstance destructor */
  virtual ~UEventInstance();

  /** comparison operator */
  bool operator== (const UEventInstance& ei);

private:

  /** The matching filter. 
   * Variables name are stored in full name and wildcards are stored as "*".
   * This filter can be paired with the content of the UEvent::args to execute
   * the matching.
   */
  std::list<std::string> filter_;

  /// The event that matches the match
  UEvent* e_;

  /** Event unique id.
   * This id is copied from @c UEvent::id_ and guarantees that, even if the @a
   * e_ pointeur is identical due to coincidence in memory allocation, the
   * @c UEventInstance is uniquely identified, in particular in the ==
   * operator.
   */
  int id_;
};

// ****************************************************************************
/** UMultiEventInstance stores a list of UEventInstance that forms a complete
 * instanciation of a test expression containing events.
 *
 * A test expression can be 'myevent1(x) && myevent2(y,3)' and a @c
 * UMultiEventInstance of this test expression can be a list of two @c
 * UEventInstance like ['myevent1(x/7)','myevent2(y/77,3)'], keeping the
 * association between x and 7, and y and 77 in each @c UEventInstance.)
 */
class UMultiEventInstance
{
  friend class UAtCandidate;

public:
  /** UMultiEventInstance constructor */
  UMultiEventInstance();

  /** UMultiEventInstance constructor.
   * This constructor will mix the two mei given as parameters in a
   * cartesien product.
   */
  UMultiEventInstance (UMultiEventInstance *mei1,
                       UMultiEventInstance *mei2);

  /** UMultiEventInstance destructor */
  virtual ~UMultiEventInstance();

  /** Adds a new instance to the list.
   * @param instance A @c UEventInstance to add to the multieventinstance
   */
  void addInstance(UEventInstance *instance);

  /** comparison operator */
  bool operator== (UMultiEventInstance& mei);

protected:
  /// List of UEventInstances
  std::list<UEventInstance*> instances_;
};

#endif
