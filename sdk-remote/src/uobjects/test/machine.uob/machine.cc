/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// A wrapper around Boost.Foreach.
#include <libport/foreach.hh>

#include "machine.hh"

Machine::Machine(float d)
  : duration(d)
{
  assert(0 <= d);
}

std::string
Machine::operator()(const strings& components) const
{
  // Waiting for duration seconds.
  useconds_t one_second = 1000 * 1000;
  usleep(useconds_t(duration * one_second));

  // Iterate over the list of strings (using Boost.Foreach), and
  // concatenate them in res.
  std::string res;
  foreach (const std::string& s, components)
    res += s;
  return res;
}
