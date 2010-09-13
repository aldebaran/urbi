/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// A wrapper around Boost.Foreach.
#include <libport/foreach.hh>

#include "ufactory.hh"

// Register the UFactory UObject in the Urbi world.
UStart(UFactory);

// Bouncing the name to the UObject constructor is mandatory.
UFactory::UFactory(const std::string& name)
  : urbi::UObject(name)
//  , factory(0)
{
  // Register the Urbi constructor.  This is the only mandatory
  // part of the C++ constructor.
  UBindFunction(UFactory, init);
}

int
UFactory::init(ufloat d)
{
  // Failure on invalid arguments.
  if (d < 0)
    return 1;

  // Bind the functions, i.e., declare them to the Urbi world.
  UBindFunction(UFactory, assemble);
  // Bind the UVars before using them.
  UBindVar(UFactory, duration);

  // Set the duration.
  duration = d;
  // Build the factory.
  //factory = new Factory(d);

  // Request that duration_set be invoked each time duration is
  // changed.  Declared after the above "duration = d" since we don't
  // want it to be triggered for this first assignment.
  UNotifyChange(duration, &UFactory::duration_set);

  // Success.
  return 0;
}

int
UFactory::duration_set(urbi::UVar& v)
{
  //aver(factory);
  ufloat d = static_cast<ufloat>(v);
  if (0 <= d)
  {
    // Valid value.
    // factory->duration = d;
    return 0;
  }
  else
    // Report failure.
    return 1;
}


std::string
UFactory::assemble(std::list<std::string> components)
{
  //aver(factory);

  // duration is in seconds.
  useconds_t one_second = 1000 * 1000;
  long duration_us = static_cast<int>(duration) * one_second;

  // Iterate over the list of strings (using Boost.Foreach), and
  // concatenate them in res.
  std::string res;
  foreach (const std::string& s, components)
  {
    yield_for(duration_us);
    res += s;
  }
  return res;

  // Bounce to Factory::operator().
//  return (*factory)(components);
}
