/*! \file ueventcompound.h
 *******************************************************************************

 File: ueventcompound.h\n
 Definition of the UEventCompound class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UEVENTCOMPOUND_H_DEFINED
#define UEVENTCOMPOUND_H_DEFINED

#include <list>
#include <string>

#include "fwd.hh"
#include "utypes.h"
#include "ustring.h"


// ****************************************************************************
/** UEventCompound stores a complete multi instanciation of a given event
 * expression.
 *
 * A compound is an arithmetical construct reflecting the logical
 * relationships between several @c UEventMatch forged out of an event
 * expression. It can be used to build a set of @c UMultiEventInstance that
 * are admissible intanciations of the event expression.
 */
class UEventCompound
{
public:
  /** UEventCompound constructor for compound UEventMatch.
   * @param ectype The type of the compound (EC_AND, EC_OR or EC_BANG)
   * @param ec1 A pointeur to the first side of the compound
   * @param ec2 A pointeur to the second side of the compound or 0 in the case
   * of EC_BANG
   */
  UEventCompound(UEventCompoundType ectype,
		 UEventCompound* ec1,
		 UEventCompound* ec2 = 0);

  /** UEventCompound constructor for single UEventMatch.
   * The type of the compound will be EC_MATCH
   * @param em Pointer to the single UEventMatch
   */
  UEventCompound(UEventMatch* em);

  /** UEventCompound constructor from a UValue.
   * The type of the compound will be EC_TRUE of EC_FALSE depending on the
   * numerical value of the UValue. This constructor is just a shortcut for
   * easy integration in the @c UExpression::eval function.
   * @param v The @c UValue used to compute the compound boolean value
   */
  UEventCompound(UValue* v);

  /** UEventCompound destructor */
  virtual ~UEventCompound();

  /** Processes the content of the compound to generate a list of multievents
   * @return The list of @c UMultiEventInstance resulting from the processing
   */
  std::list<UMultiEventInstance*> mixing();

  /** Reduces the compound to its bang-terminal normal form.
   * Bangs are on pushed back on leafs and leafs of the form !event
   * are replaced by their boolean equivalent EC_TRUE or
   * EC_FALSE, depending on the content of the UEventMatch 'event'.
   * Finally, compounds like "X && EC_TRUE" are replaced by "X" and "X &&
   * EC_FALSE" by "EC_FALSE", and equivalent reductions with 'or'.
   *
   * At the end, it remains only a series of non negative events (UEventMatch)
   * linked by logicial connectors, or simply EC_TRUE/EC_FALSE in case of total
   * reduction.
   */
  void normalForm();

protected:

  /// Prevents recursive deletion in the destructor
  void keepalive();

  /// keep from recursive deletion
  bool keepalive_;

  /// Type of the UEventCompound
  UEventCompoundType ectype_;

  /// Left part of the compound, if any (0 otherwise)
  UEventCompound* ec1_;

  /// Right part of the compound, if any (0 otherwise)
  UEventCompound* ec2_;

  /// Single match if the type is EC_MATCH (0 otherwise)
  UEventMatch* em_;
};

#endif
