/*! \file uasyncregister.hh
 *******************************************************************************

 File: uasyncregister.h\n
 Definition of the UASyncRegister class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UASYNCREGISTER_HH
# define UASYNCREGISTER_HH

# include <list>
# include <string>

# include "kernel/fwd.hh"

/// UASyncRegister class, used to register UASyncCommand notifications
class UASyncRegister
{
public:

  /** UASyncRegister constructor
   */
  UASyncRegister();

  /** UEvent destructor */
  ~UASyncRegister();

  /** Register an asynchronous command to notify
   * @param cmd The command to add to notify list
   */
  void registerCmd(UASyncCommand* cmd);

  /** Unregister an asynchronous command
   * @param cmd The command to remove from notify list
   */
  void unregisterCmd(UASyncCommand* cmd);

  /** update the list of registered commands to ask them to be updated.
   * This is the fundamental method of this class. It is called when the
   * register receives new data and so the associated commands should
   * reevaluate their expressions containing the register.
   */
  void updateRegisteredCmd();

private:
  /// UEventHandle to which the event is attached
  std::list<UASyncCommand*> register_;
};

#endif
