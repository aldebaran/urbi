/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include "umachine.hh"

// Register the UMachine UObject in the Urbi world.
UStart(UMachine);

// Bouncing the name to the UObject constructor is mandatory.
UMachine::UMachine(const std::string& name)
  : urbi::UObject(name)
  , machine(0)
{
  // Register the Urbi constructor.  This is the only mandatory
  // part of the C++ constructor.
  UBindFunction(UMachine, init);
}

int
UMachine::init(ufloat d)
{
  // Failure on invalid arguments.
  if (d < 0)
    return 1;

  // Bind the functions, i.e., declare them to the Urbi world.
  UBindFunction(UMachine, assemble);
  UBindThreadedFunctionRename
    (UMachine, assemble, "threadedAssemble", urbi::LOCK_FUNCTION);
  // Bind the UVars before using them.
  UBindVar(UMachine, duration);

  // Set the duration.
  duration = d;
  // Build the machine.
  machine = new Machine(d);

  // Request that duration_set be invoked each time duration is
  // changed.  Declared after the above "duration = d" since we don't
  // want it to be triggered for this first assignment.
  UNotifyChange(duration, &UMachine::duration_set);

  // Success.
  return 0;
}

int
UMachine::duration_set(urbi::UVar& v)
{
  assert(machine);
  ufloat d = static_cast<ufloat>(v);
  if (0 <= d)
  {
    // Valid value.
    machine->duration = d;
    return 0;
  }
  else
    // Report failure.
    return 1;
}


std::string
UMachine::assemble(std::list<std::string> components)
{
  assert(machine);

  // Bounce to Machine::operator().
  return (*machine)(components);
}
